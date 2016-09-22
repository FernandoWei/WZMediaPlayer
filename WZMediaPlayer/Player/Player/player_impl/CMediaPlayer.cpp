//
//  CMediaPlayer.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "CMediaPlayer.hpp"
#include <chrono>

std::shared_ptr<IMediaPlayer> IMediaPlayer::createPlayer(const std::string&& url){
    return std::shared_ptr<IMediaPlayer>(new CMediaPlayer(std::move(url)));
}

CMediaPlayer::CMediaPlayer(const std::string&& url) noexcept
:mUrl(url),
mPrepared(false),
mStarted(false),
mPaused(false),
mFirstBuffering(true),
mStreamEnd(false)
{
    ;
}

CMediaPlayer::~CMediaPlayer(){
    for (auto& thread : mThreadPool){
        thread.join();
    }
}

void CMediaPlayer::start(){
    if (!mStarted){
        prepare();
        startDecoding();
        mStarted = true;
    }
}

void CMediaPlayer::prepare(){
    if (!mPrepared){
        init();
        prepareMediaObjects();
        prepareTasks();
        mMediaSource->prepare();
        mPrepared = true;
    }
}

void CMediaPlayer::stop(){
    if (mStarted){
        mStarted = false;
    }
}

void CMediaPlayer::pause(){
    if (!mPaused){
        mAudioDecoder->pause();
        mVideoDecoder->pause();
        mPaused = true;
    }
}

uint64_t CMediaPlayer::getCurrentPosition(){
    return 0;
}

void CMediaPlayer::prepareTasks(){
    mTasks.insert(std::move(std::make_pair(std::string("PreparePacketQueueThread"), [this](){
        this->preparePacketQueue();
    })));
    mTasks.insert(std::move(std::make_pair(std::string("VideoDecoderThread"), [this](){
        this->mVideoDecoder->start();
    })));
    mTasks.insert(std::move(std::make_pair(std::string("AudioDecoderThread"), [this](){
        this->mAudioDecoder->start();
    })));
}

void CMediaPlayer::prepareMediaObjects(){
    mMediaSource = std::make_shared<MediaSource>(new MediaSource(mUrl));
    mAudioDecoder = std::make_shared<MediaDecoder>(new AudioDecoder);
    mVideoDecoder = std::make_shared<MediaDecoder>(new VideoDecoder);
}

void CMediaPlayer::preparePacketQueue(){
    AVPacket packet;
    std::chrono::duration<int, std::milli> duration(20);
    while (mStarted){
        switch(mMediaSource->read(&packet)){
            case PlayerState::TRY_AGAIN_LATER:{
                std::this_thread::sleep_for(duration);
                continue;
            }
            case PlayerState::END_OF_FILE:{
                mStreamEnd = true;
                std::cout << "End of file.\n";
                break;
            }
            default:{
                enqueuePacket(&packet);
                break;
            }
        }
        
        if (mStreamEnd){
            break;
        }
    }
    
    std::cout << "PreparePacketQueueThread end.\n";
}

void CMediaPlayer::enqueuePacket(AVPacket* packet){
    if (packet->stream_index == mMediaSource->getVideoStreamIndex()){
        mVideoDecoder->enqueuePacket(packet);
    } else if (packet->stream_index == mMediaSource->getAudioStreamIndex()){
        mAudioDecoder->enqueuePacket(packet);
    } else {
        av_packet_unref(packet);
    }
}

void CMediaPlayer::startDecoding(){
    for (auto& task : mTasks){
        mThreadPool.push_back(std::move(std::thread(task.second)));
        mThreadIDs.insert(std::make_pair(task.first, mThreadPool.back().get_id()));
    }
    
}







