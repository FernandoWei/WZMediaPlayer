//
//  VideoDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "VideoDecoder.hpp"

VideoDecoder::VideoDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData)
{
    MediaDecoder::MediaDecoder(std::move(name), firstBufferedPktCount, nonFirstBufferSecondsOfData);
}

VideoDecoder::~VideoDecoder(){
    ;
}

void VideoDecoder::prepare(){
    ;
}

PlayerState VideoDecoder::decode(AVPacket* pkt){
    return PlayerState::OK;
}

void VideoDecoder::flush(){
    MediaDecoder::flush();
}