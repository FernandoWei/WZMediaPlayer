//
//  AudioDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "AudioDecoder.hpp"

AudioDecoder::AudioDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData)
{
    MediaDecoder::MediaDecoder(std::move(name), firstBufferedPktCount, nonFirstBufferSecondsOfData);
}

AudioDecoder::~AudioDecoder(){
    ;
}

void AudioDecoder::prepare(){
    ;
}

PlayerState AudioDecoder::decode(AVPacket* pkt){
    PlayerState result = PlayerState::ERROR;
    return result;
}

void AudioDecoder::flush(){
    MediaDecoder::flush();
}