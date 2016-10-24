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
mLastClock(),
mCurrentClock(),
mLastVideoPTS(-1),
mDiscardFrameType(DiscardFrameType::DISCARD_NONE),
mFirst(true)
{
    ;
}

MediaState::~MediaState(){
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

int64_t MediaState::updateReferenceClock(int64_t pts) {
    if (mAudioReady) {
        updateAudioClock();
        return pts - mAudioClock;
    } else {
        mCurrentClock = std::chrono::steady_clock::now();
        if (mFirst){
            mLastClock = mCurrentClock;
            mLastVideoPTS = pts;
            mFirst = false;
        }
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(mCurrentClock - mLastClock);
        int64_t ptsIncrement = pts - mLastVideoPTS;
        return ptsIncrement - elapsedTime.count();
    }
}

void MediaState::synchronize(int64_t pts, std::atomic_bool& stopped){
    int64_t delay = updateReferenceClock(pts);
    if (delay > kMaxSyncDelay){
        if (mDiscardFrameType != DiscardFrameType::DISCARD_NONE){
            mDiscardFrameType = DiscardFrameType::DISCARD_NONE;
        }
        
        while (delay > kMaxSyncDelay && !stopped){
            delay -= kMaxSyncDelay;
            delay = std::max<int64_t>(delay, kMinSyncSleep);
            delay = std::min<int64_t>(delay, kMaxSyncSleep);
            std::chrono::duration<int64_t, std::milli> duration(delay);
            std::this_thread::sleep_for(duration);
            delay = updateReferenceClock(pts);
        }
    } else if (delay < kMinSyncDelay){
        if (mDiscardFrameType != DiscardFrameType::DISCARD_DEFAULT){
            mDiscardFrameType = DiscardFrameType::DISCARD_DEFAULT;
        }
    }
    
    if (!mAudioReady){
        mLastClock = mCurrentClock;
        mLastVideoPTS = pts;
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

std::string MediaState::toString() const {
    return std::string("MediaState");
}

void MediaState::setAudioClockPtr(int64_t* ptr){
    if (ptr){
        mAudioClockPtr = ptr;
    } else {
        log("null audio clock.");
    }
}

