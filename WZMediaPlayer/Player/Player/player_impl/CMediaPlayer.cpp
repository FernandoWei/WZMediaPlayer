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
    std::cout << "Player starts.\n";
    if (!mStarted){
        if (PlayerState::OK != prepare()){
            std::cout << "Failed to start playing because of unsuccessful player preparing.\n";
            return;
        }
        startDecoding();
        mStarted = true;
    }
}

PlayerState CMediaPlayer::prepare(){
    std::cout << "Player prepared.\n";
    PlayerState result = PlayerState::ERROR;
    if (!mPrepared){
        result = prepareMediaSource();
        if (result == PlayerState::OK){
            result = prepareMediaDecoder();
            if (result == PlayerState::OK){
                result = prepareTasks();
            }
        }
        if (result == PlayerState::OK){
            mPrepared = true;
        }
    }
    return result;
}

void CMediaPlayer::stop(){
    if (mStarted){
        mMediaSource->stop();
        mAudioDecoder->stop();
        mVideoDecoder->stop();
        mStarted = false;
        std::cout << "Player stopped.\n";
    }
}

void CMediaPlayer::pause(){
    if (mStarted && !mPaused){
        mAudioDecoder->pause();
        mVideoDecoder->pause();
        mPaused = true;
        std::cout << "Player paused.\n";
    }
}

void CMediaPlayer::resume(){
    if (mStarted && mPaused){
        mAudioDecoder->resume();
        mVideoDecoder->resume();
        mPaused = false;
        std::cout << "Player resumed.\n";
    }
}

void CMediaPlayer::flush(){
    mAudioDecoder->flush();
    mVideoDecoder->flush();
}

uint64_t CMediaPlayer::getCurrentPosition(){
    return 0;
}

PlayerState CMediaPlayer::prepareTasks(){
    if (mStarted){
        mTasks.insert(std::move(std::make_pair(std::string("PreparePacketQueueThread"), [this](){
            this->readAndEnqueuePacket();
        })));
        mTasks.insert(std::move(std::make_pair(std::string("VideoDecoderThread"), [this](){
            this->mVideoDecoder->start();
        })));
        mTasks.insert(std::move(std::make_pair(std::string("AudioDecoderThread"), [this](){
            this->mAudioDecoder->start();
        })));
        return PlayerState::OK;
    } else {
        std::cout << "Failed to prepare media tasks because of stopped state.\n";
        return PlayerState::STOPPED;
    }
}

PlayerState CMediaPlayer::prepareMediaSource(){
    if (mStarted){
        mMediaSource = std::shared_ptr<MediaSource>(new MediaSource(mUrl));
        return mMediaSource->prepare();
    } else {
        std::cout << "Failed to prepare media source because of stopped state.\n";
        return PlayerState::STOPPED;
    }
}

PlayerState CMediaPlayer::prepareMediaDecoder(){
    PlayerState result = PlayerState::ERROR;
    if (mStarted && mMediaSource){
        mAudioDecoder = std::shared_ptr<MediaDecoder>(new AudioDecoder(std::move(std::string("AudioDecoder")), AUDIO_PKT_COUNT_PER_SECOND, NON_FIRST_BUFFER_SECONDS, mMediaSource->getAudioStream()));
        mVideoDecoder = std::shared_ptr<MediaDecoder>(new VideoDecoder(std::move("VideoDecoder"), VIDEO_PKT_COUNT_PER_SECOND, NON_FIRST_BUFFER_SECONDS, mMediaSource->getVideoStream()));
        result = PlayerState::OK;
    } else {
        if (!mStarted){
            std::cout << "Failed to prepare media decoder because of stopped state.\n";
            result = PlayerState::STOPPED;
        } else if (!mMediaSource){
            std::cout << "Failed to prepare media decoder because of unprepared media source.\n";
            result = PlayerState::ERROR;
        }
    }
    return result;
}

void CMediaPlayer::readAndEnqueuePacket(){
    AVPacket packet;
    std::chrono::duration<int, std::milli> duration(NUM_TWENTY);
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







