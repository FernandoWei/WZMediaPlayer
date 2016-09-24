//
//  MediaSource.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaSource.hpp"

MediaSource::MediaSource(const std::string& url):
mUrl(url),
mStopped(false),
mVideoStreamIndex(-1),
mAudioStreamIndex(-1)
{
    ;
}

MediaSource::~MediaSource(){
    close();
}

PlayerState MediaSource::read(AVPacket* pkt){
    PlayerState result = PlayerState::ERROR;
    return result;
}

void MediaSource::init(){
    ;
}

void MediaSource::open(){
    ;
}

void MediaSource::prepare(){
    ;
}

void MediaSource::close(){
    ;
}

int MediaSource::getAudioStreamIndex(){
    return mAudioStreamIndex;
}

int MediaSource::getVideoStreamIndex(){
    return mVideoStreamIndex;
}

void MediaSource::stop(){
    mStopped = true;
}