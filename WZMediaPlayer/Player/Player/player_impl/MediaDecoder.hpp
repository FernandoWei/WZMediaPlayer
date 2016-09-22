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
    MediaDecoder();
    ~MediaDecoder(){}
    
public:
    void enqueuePacket(AVPacket* packet);
    PlayerState dequeuePacket(AVPacket* packet);
    void start();
    void pause();
    void resume();
    
public:
    void virtual prepare() = 0;
    void virtual decode(AVPacket* pkt) = 0;
    
private:
    bool mPrepared;
    bool mPaused;
    std::list<AVPacket> mPacketQueue;
};

#endif /* MediaDecoder_hpp */