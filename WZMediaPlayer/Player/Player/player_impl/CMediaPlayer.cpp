//
//  CMediaPlayer.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "CMediaPlayer.hpp"


std::shared_ptr<IMediaPlayer> IMediaPlayer::createPlayer(std::string&& url){
    return std::shared_ptr<IMediaPlayer>(new WZ::MediaPlayer::CMediaPlayer(std::move(url)));
}

namespace WZ {
    namespace MediaPlayer {
        
        CMediaPlayer::CMediaPlayer(std::string&& url) noexcept
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
                log("Player starts.");
            if (!mStarted){
                if (Utility::PlayerState::OK != prepare()){
                                log("Failed to start playing because of unsuccessful player preparing.");
                    return;
                }
                startDecoding();
                mStarted = true;
            }
        }
        
        Utility::PlayerState CMediaPlayer::prepare(){
                log("Player prepared.");
            auto result = Utility::PlayerState::ERROR;
            if (!mPrepared){
                result = prepareMediaSource();
                if (result == Utility::PlayerState::OK){
                    result = prepareMediaDecoder();
                    if (result == Utility::PlayerState::OK){
                        result = prepareTasks();
                    }
                }
                if (result == Utility::PlayerState::OK){
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
                        log("Player stopped.");
            }
        }
        
        void CMediaPlayer::pause(){
            if (mStarted && !mPaused){
                mAudioDecoder->pause();
                mVideoDecoder->pause();
                mPaused = true;
                        log("Player paused.");
            }
        }
        
        void CMediaPlayer::resume(){
            if (mStarted && mPaused){
                mAudioDecoder->resume();
                mVideoDecoder->resume();
                mPaused = false;
                //        log("Player resumed.");
            }
        }
        
        void CMediaPlayer::flush(){
            mAudioDecoder->flush();
            mVideoDecoder->flush();
        }
        
        uint64_t CMediaPlayer::getCurrentPosition(){
            return 0;
        }
        
        Utility::PlayerState CMediaPlayer::prepareTasks(){
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
                return Utility::PlayerState::OK;
            } else {
                        log("Failed to prepare media tasks because of stopped state.");
                return Utility::PlayerState::STOPPED;
            }
        }
        
        Utility::PlayerState CMediaPlayer::prepareMediaSource(){
            if (mStarted){
                mMediaSource = std::shared_ptr<MediaSource>(new MediaSource(mUrl));
                return mMediaSource->prepare();
            } else {
                log("Failed to prepare media source because of stopped state.");
                return Utility::PlayerState::STOPPED;
            }
        }
        
        Utility::PlayerState CMediaPlayer::prepareMediaDecoder(){
            auto result = Utility::PlayerState::ERROR;
            if (mStarted && mMediaSource){
                mMediaState = std::make_shared<MediaState>();
                mAudioDecoder = std::shared_ptr<MediaDecoder>(new AudioDecoder(AUDIO_PKT_COUNT_PER_SECOND, NON_FIRST_BUFFER_SECONDS, mMediaSource->getAudioStream(), mMediaState));
                mVideoDecoder = std::shared_ptr<MediaDecoder>(new VideoDecoder(VIDEO_PKT_COUNT_PER_SECOND, NON_FIRST_BUFFER_SECONDS, mMediaSource->getVideoStream(), mMediaState));
                result = Utility::PlayerState::OK;
            } else {
                if (!mStarted){
                    log("Failed to prepare media decoder because of stopped state.");
                    result = Utility::PlayerState::STOPPED;
                } else if (!mMediaSource){
                    log("Failed to prepare media decoder because of unprepared media source.");
                    result = Utility::PlayerState::ERROR;
                }
            }
            return result;
        }
        
        void CMediaPlayer::readAndEnqueuePacket(){
            AVPacket packet;
            std::chrono::duration<int, std::milli> duration(NUM_TWENTY);
            while (mStarted){
                switch(mMediaSource->read(&packet)){
                    case Utility::PlayerState::TRY_AGAIN_LATER:{
                        std::this_thread::sleep_for(duration);
                        continue;
                    }
                    case Utility::PlayerState::END_OF_FILE:{
                        mStreamEnd = true;
                        //                log("End of file.");
                        break;
                    }
                    case Utility::PlayerState::STOPPED:{
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
            
                log("PreparePacketQueueThread end.");
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
        
    }
    
}
