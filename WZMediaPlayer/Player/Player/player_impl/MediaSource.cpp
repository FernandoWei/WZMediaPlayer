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
mAudioStreamIndex(-1),
mFormatCtx(nullptr)
{
}

MediaSource::~MediaSource(){
    close();
}

PlayerState MediaSource::read(AVPacket* pkt){
    PlayerState result = PlayerState::ERROR;
    int status = av_read_frame(mFormatCtx, pkt);
    if (status < 0){
        if (AVERROR_EOF == status || AVERROR(EIO) == status){
            if (mStopped){
                std::cout << "reading packet were stopped.\n";
                result = PlayerState::TRY_AGAIN_LATER;
            } else {
                std::cout << "end of stream with result " << "EOF: " << (status == AVERROR_EOF) << ", EIO: " << (status == AVERROR(EIO)) << ".\n";
                result = PlayerState::END_OF_FILE;
            }
        } else {
            std::array<char, 1024> error{0};
            av_strerror(status, error.data(), error.size());
            std::cout << error.data();
            result = PlayerState::TRY_AGAIN_LATER;
        }
    }
    return result;
}

void MediaSource::init(){
    av_register_all();
    avformat_network_init();
}

int MediaSource::didStopped(void* thiz){
    MediaSource* source = static_cast<MediaSource*>(thiz);
    return source->mStopped ? 1 : 0;
}


//打开文件
PlayerState MediaSource::open(){
    PlayerState result = PlayerState::ERROR;
    mFormatCtx = avformat_alloc_context();
    mFormatCtx->interrupt_callback.opaque = this;
    mFormatCtx->interrupt_callback.callback = didStopped;
    if (0 == avformat_open_input(&mFormatCtx, mUrl.c_str(), nullptr, nullptr)){
        if (0 <= avformat_find_stream_info(mFormatCtx, nullptr)){
            for (int i = 0; i < mFormatCtx->nb_streams; i++){
                if (mFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
                    mVideoStreamIndex = mFormatCtx->streams[i]->index;
                    mVideoStream = mFormatCtx->streams[i];
                } else if (mFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
                    mAudioStreamIndex = mFormatCtx->streams[i]->index;
                    mAudioStream = mFormatCtx->streams[i];
                }
                
                result = PlayerState::OK;
            }
        } else {
            close();
            std::cout << "Failed to find stream info.\n";
        }
    } else {
        std::cout << "Failed to open input.\n";
    }
    return result;
}

PlayerState MediaSource::prepare(){
    init();
    return open();
}

void MediaSource::close(){
    if (mFormatCtx){
        avformat_close_input(&mFormatCtx);
        mFormatCtx = nullptr;
    }
}

AVStream* MediaSource::getAudioStream(){
    return mAudioStream;
}

AVStream* MediaSource::getVideoStream(){
    return mVideoStream;
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