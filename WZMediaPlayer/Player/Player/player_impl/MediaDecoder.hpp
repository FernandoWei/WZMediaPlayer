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

enum class PlayerState;

class MediaDecoder {
    
public:
    MediaDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream);
    MediaDecoder() = default;
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
    bool virtual prepare();
    PlayerState virtual decode(AVPacket* pkt);
    void virtual flush();
    
private:
    void dequeuePacket(AVPacket* packet);
    std::shared_ptr<AVPacket> allocatePacket();
    bool isFullBuffered();
    void clearPktQueue();
    
protected:
    AVStream* mMediaStream;
    
private:
    bool mPrepared;
    std::atomic_bool mPaused;
    std::atomic_bool mStopped;
    std::atomic_bool mIsFlushing;
    bool mFirstBuffered;
    
    std::list<std::shared_ptr<AVPacket>> mPacketQueue;
    std::string mName;
    AVPacket mDequeuePacket;
    
    uint8_t mFirstBufferedPktCount;
    uint8_t mNonFirstBufferSecondsOfData;
};

#endif /* MediaDecoder_hpp */
