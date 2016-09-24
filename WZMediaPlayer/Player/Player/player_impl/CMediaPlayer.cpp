//
//  CMediaPlayer.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "CMediaPlayer.hpp"


std::shared_ptr<IMediaPlayer> IMediaPlayer::createPlayer(const std::string&& url){
    return std::shared_ptr<IMediaPlayer>(new CMediaPlayer(std::move(url)));
}

CMediaPlayer::CMediaPlayer(const std::string&& url) noexcept
:mUrl(url),
mPrepared(false),
mStarted(false),
mPaused(false),
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
        prepareMediaObjects();
        prepareTasks();
        mMediaSource->prepare();
        mPrepared = true;
    }
}

void CMediaPlayer::stop(){
    if (mStarted){
        mMediaSource->stop();
        mAudioDecoder->stop();
        mVideoDecoder->stop();
        mStarted = false;
    }
}

void CMediaPlayer::pause(){
    if (mStarted && !mPaused){
        mAudioDecoder->pause();
        mVideoDecoder->pause();
        mPaused = true;
    }
}

void CMediaPlayer::resume(){
    if (mStarted && mPaused){
        mAudioDecoder->resume();
        mVideoDecoder->resume();
        mPaused = false;
    }
}

void CMediaPlayer::flush(){
    mAudioDecoder->flush();
    mVideoDecoder->flush();
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
    mMediaSource = std::shared_ptr<MediaSource>(new MediaSource(mUrl));
    mAudioDecoder = std::shared_ptr<MediaDecoder>(new AudioDecoder(std::move(std::string("AudioDecoder")), 50, 3));
    mVideoDecoder = std::shared_ptr<MediaDecoder>(new VideoDecoder(std::move("VideoDecoder"), 25, 3));
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
            case PlayerState::STOPPED:{
                break;
            }
            default:{
                enqueuePacket(&packet);
                av_packet_unref(&packet);
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
    }
}

void CMediaPlayer::startDecoding(){
    for (auto& task : mTasks){
        mThreadPool.push_back(std::move(std::thread(task.second)));
        mThreadIDs.insert(std::make_pair(task.first, mThreadPool.back().get_id()));
    }
    
}







