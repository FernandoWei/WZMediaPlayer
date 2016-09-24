//
//  MediaDecoder.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef MediaDecoder_hpp
#define MediaDecoder_hpp

#include "CommonInclude.h"

#include <list>

enum class PlayerState;

class MediaDecoder {
    
public:
    MediaDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData);
    MediaDecoder(const MediaDecoder& decoder) = delete;
    MediaDecoder(MediaDecoder&& decoder) noexcept = delete;
    MediaDecoder& operator=(const MediaDecoder& decoder) = delete;
    MediaDecoder& operator=(MediaDecoder&& decoder) noexcept = delete;
    ~MediaDecoder();
    
public:
    void enqueuePacket(const AVPacket* packet);
    void start();
    void stop();
    void pause();
    void resume();
    
public:
    void virtual prepare() = 0;
    PlayerState virtual decode(AVPacket* pkt) = 0;
    
private:
    void dequeuePacket(AVPacket* packet);
    bool isFullBuffered();
    void clearPktQueue();
    void flush();
    
private:
    bool mPrepared;
    std::atomic_bool mPaused;
    std::atomic_bool mStopped;
    bool mFirstBuffered;
    
    std::list<std::shared_ptr<AVPacket>> mPacketQueue;
    std::string mName;
    AVPacket mDequeuePacket;
    
    uint8_t mFirstBufferedPktCount;
    uint8_t mNonFirstBufferSecondsOfData;
};

#endif /* MediaDecoder_hpp */
