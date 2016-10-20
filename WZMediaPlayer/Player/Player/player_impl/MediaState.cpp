//
//  MediaState.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/10/20.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaState.hpp"

MediaState::MediaState()
:mAudioClockPtr(nullptr),
mDiscardFrameType(DiscardFrameType::DISCARD_NONE)
{
    ;
}

void MediaState::updateAudioClock(){
    if (mAudioReady && mAudioClockPtr){
        mAudioClock = *mAudioClockPtr;
    }
}

void MediaState::updateVideoClock(int64_t pts){
    if (pts > 0){
        mVideoClock = pts;
    }
}

void MediaState::synchronize(int64_t pts, std::atomic_bool& stopped){
    updateAudioClock();
//    updateVideoClock(pts);
    
    if (mAudioReady){
        int32_t delay = pts - mAudioClock;
        if (delay > kMaxSyncDelay){
            if (mDiscardFrameType != DiscardFrameType::DISCARD_NONE){
                mDiscardFrameType = DiscardFrameType::DISCARD_NONE;
            }
            
            while (delay > kMaxSyncDelay && !stopped){
                delay -= kMaxSyncSleep;
                delay = std::max<int32_t>(delay, kMinSyncSleep);
                delay = std::min<int32_t>(delay, kMaxSyncSleep);
                std::chrono::duration<int32_t, std::milli> duration(delay);
                std::this_thread::sleep_for(duration);
                updateAudioClock();
                delay = pts - mAudioClock;
            }
        } else if (delay < kMinSyncDelay){
            if (mDiscardFrameType != DiscardFrameType::DISCARD_DEFAULT){
                mDiscardFrameType = DiscardFrameType::DISCARD_DEFAULT;
            }
        }
    }else {
        auto currentClock = std::chrono::system_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentClock - mLastClock);
        int64_t ptsDuration = pts - mLastVideoPTS;
        int64_t delay = ptsDuration - elapsedTime.count();
        if (delay)
        mLastClock = currentClock;
    }
}

DiscardFrameType MediaState::getCurrentDiscardState(){
    return mDiscardFrameType;
}

void MediaState::setAudioReady(bool yes){
    if (mAudioReady != yes){
        mAudioReady = yes;
    }
}

void MediaState::setVideoReady(bool yes){
    if (mVideoReady != yes){
        mVideoReady = yes;
    }
}

