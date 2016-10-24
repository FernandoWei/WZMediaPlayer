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
#include "MediaLog.hpp"

const uint32_t kMaxSyncDelay = 125;
const uint32_t kMinSyncDelay = -45;
const uint32_t kMaxSyncSleep = 300;
const uint32_t kMinSyncSleep = 10;

class MediaState : public MediaLog {
public:
    MediaState();
    MediaState(const MediaState& state) = delete;
    MediaState(MediaState&& state) noexcept = delete;
    MediaState& operator=(const MediaState& state) = delete;
    MediaState& operator=(MediaState&& state) = delete;
    ~MediaState();
    
public:
    void synchronize(int64_t pts, std::atomic_bool& stopped);
    void setAudioClockPtr(int64_t* clock);
    DiscardFrameType getCurrentDiscardState();
    void setAudioReady(bool yes);
    void setVideoReady(bool yes);
    
private:
    void updateAudioClock();
    void updateVideoClock(int64_t pts);
    int64_t updateReferenceClock(int64_t pts);
    std::string toString() const;
    
public:
    
private:
    int64_t* mAudioClockPtr;
    int64_t mAudioClock;
    int64_t mVideoClock;
    DiscardFrameType mDiscardFrameType;
    
    bool mVideoReady;
    bool mAudioReady;
    bool mFirst;
    
    int64_t mLastVideoPTS;
    std::chrono::steady_clock::time_point mLastClock;
    std::chrono::steady_clock::time_point mCurrentClock;
};

#endif /* MediaState_hpp */
