//
//  MediaDecoder.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef MediaDecoder_hpp
#define MediaDecoder_hpp

extern "C" {
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
    //#include <libavutil/time.h>
    //#include <libswresample/swresample.h>
}

#include <stdio.h>

class MediaDecoder {
    
public:
    MediaDecoder();
    ~MediaDecoder(){}
    
public:
    void enqueuePacket(AVPacket* packet);
    void dequeuePacket(AVPacket* packet);
    void start();
    void pause();
    void resume();
    
public:
    void virtual prepare() = 0;
    void virtual decode(AVPacket* pkt) = 0;
    
private:
    bool mPrepared;
    bool mPaused;
//    std::list<AVPacket> mPacketQueue;
};

#endif /* MediaDecoder_hpp */
