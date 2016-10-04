//
//  VideoDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "VideoDecoder.hpp"

VideoDecoder::VideoDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream)
{
    MediaDecoder::MediaDecoder(std::move(name), firstBufferedPktCount, nonFirstBufferSecondsOfData, stream);
}

VideoDecoder::~VideoDecoder(){
    ;
}

void VideoDecoder::prepare(){
    prepareCodecData();
    prepareHWDecoder();
}

void VideoDecoder::prepareHWDecoder(){
    
}

void VideoDecoder::prepareCodecData(){
    if (mMediaStream){
        AVCodecParameters* codecPar = mMediaStream->codecpar;
        if (codecPar && codecPar->extradata && codecPar->extradata[0] == 0x01 && codecPar->extradata_size > 8){
            uint8_t spsOffsetIndex = 8;
            uint8_t spsLength = codecPar->extradata[spsOffsetIndex - 1];
            mSPS.resize(spsLength);
            memcpy(mSPS.data(), &codecPar->extradata[spsOffsetIndex], spsLength);
            uint8_t ppsOffsetIndex = spsOffsetIndex + spsLength + 3;
            uint8_t ppsLength = codecPar->extradata[ppsOffsetIndex - 1];
            mPPS.resize(ppsLength);
            memcpy(mPPS.data(), &codecPar->extradata[ppsOffsetIndex], ppsLength);
        } else {
            std::cout << "invalid codec data.\n";
        }
    }
}

PlayerState VideoDecoder::decode(AVPacket* pkt){
    return PlayerState::OK;
}

void VideoDecoder::flush(){
    MediaDecoder::flush();
}