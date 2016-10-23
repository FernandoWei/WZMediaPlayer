//
//  MediaDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaDecoder.hpp"


MediaDecoder::MediaDecoder(uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream, std::shared_ptr<MediaState> state):
mPrepared(false),
mPaused(false),
mStopped(false),
mFirstBuffered(true),
mIsFlushing(false),
mFirstBufferedPktCount(firstBufferedPktCount),
mNonFirstBufferSecondsOfData(nonFirstBufferSecondsOfData),
mMediaStream(stream),
mMediaState(state)
{
    log("construct.");
    av_init_packet(&mDequeuePacket);
}

MediaDecoder::~MediaDecoder(){
    log("desconstruct.");
    clearPktQueue();
}

void MediaDecoder::clearPktQueue(){
    mPacketQueue.clear();
}

std::shared_ptr<AVPacket> MediaDecoder::allocatePacket(){
    try {
        return std::shared_ptr<AVPacket>(new AVPacket, [](AVPacket* pkt){
            if (pkt){
                av_packet_unref(pkt);
                delete pkt;
                pkt = nullptr;
            }
        });
    } catch (const std::bad_alloc& e) {
        std::cout << "Failed to allocate AVPacket.\n";
        return nullptr;
    }
}

void MediaDecoder::enqueuePacket(const AVPacket* pkt){
    if (!mIsFlushing){
        auto packetPtr = allocatePacket();
        if (packetPtr){
            av_copy_packet(packetPtr.get(), pkt);
            mPacketQueue.push_back(packetPtr);
            log("buffered", mPacketQueue.size(), "packets.");
        }
    }
}

void MediaDecoder::dequeuePacket(AVPacket* pkt){
    av_copy_packet(pkt, mPacketQueue.front().get());
    av_packet_unref(mPacketQueue.front().get());
    mPacketQueue.erase(mPacketQueue.begin());
}

bool MediaDecoder::isFullBuffered(){
    bool result = false;
    if (mFirstBuffered){
        if (mPacketQueue.size() >= mFirstBufferedPktCount){
            result = true;
        }
    }else {
        if (mPacketQueue.size() >= mFirstBufferedPktCount * mNonFirstBufferSecondsOfData){
            result = true;
        }
    }
    return result;
}

void MediaDecoder::start(){
    if (!mPrepared){
        if (prepare()){
            mPrepared = true;
        } else {
            return;
        }
    }
    
    std::chrono::duration<int, std::milli> duration(NUM_TWENTY);
    while (!mStopped){
        if (mPaused || !isFullBuffered() || mIsFlushing){
            std::this_thread::sleep_for(duration);
            continue;
        }
        
        dequeuePacket(&mDequeuePacket);
        switch(decode(&mDequeuePacket)){
            case PlayerState::TRY_AGAIN_LATER:{
                break;
            }
            case PlayerState::ERROR:{
                break;
            }
            default:{
                break;
            }
        }
        av_packet_unref(&mDequeuePacket);
    }
    
    log("ends.");
}

void MediaDecoder::stop(){
    if (!mStopped){
        mStopped = true;
        log("stopped.");
    }
}

void MediaDecoder::pause(){
    if (!mPaused){
        mPaused = true;
        log("paused.");
    }
}

void MediaDecoder::resume(){
    if (mPaused){
        mPaused = false;
        log("resumed.");
    }
}

void MediaDecoder::flush(){
    if (!mIsFlushing){
        log("flushed.");
        mIsFlushing = true;
        clearPktQueue();
        mIsFlushing = false;
    }
}

PlayerState MediaDecoder::decode(AVPacket* pkt){
    return PlayerState::OK;
}

bool MediaDecoder::prepare(){
    return true;
}

uint32_t* MediaDecoder::getAudioClockPtr(){
    return nullptr;
}

std::string MediaDecoder::toString() const {
    return std::string("MediaDecoder");
}




