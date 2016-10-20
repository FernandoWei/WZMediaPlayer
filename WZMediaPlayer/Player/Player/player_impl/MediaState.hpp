//
//  MediaState.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/10/20.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef MediaState_hpp
#define MediaState_hpp

#include "CommonInclude.h"

const uint32_t kMaxSyncDelay = 125;
const uint32_t kMinSyncDelay = -45;
const uint32_t kMaxSyncSleep = 300;
const uint32_t kMinSyncSleep = 10;

class MediaState {
public:
    MediaState();
    MediaState(const MediaState& state) = delete;
    MediaState(MediaState&& state) noexcept = delete;
    MediaState& operator=(const MediaState& state) = delete;
    MediaState& operator=(MediaState&& state) = delete;
    ~MediaState();
    
public:
    void synchronize(int64_t pts, std::atomic_bool& stopped);
    void setAudioClockPtr(uint32_t* clock);
    DiscardFrameType getCurrentDiscardState();
    void setAudioReady(bool yes);
    void setVideoReady(bool yes);
    
private:
    void updateAudioClock();
    void updateVideoClock(int64_t pts);
    
public:
    
private:
    uint32_t* mAudioClockPtr;
    uint32_t mAudioClock;
    uint32_t mVideoClock;
    DiscardFrameType mDiscardFrameType;
    
    bool mVideoReady;
    bool mAudioReady;
    
    int64_t mLastVideoPTS;
    std::chrono::system_clock mLastClock;
    std::chrono::system_clock mCurrentClock;
};

#endif /* MediaState_hpp */
