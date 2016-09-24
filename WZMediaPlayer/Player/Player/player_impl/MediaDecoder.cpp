//
//  MediaDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaDecoder.hpp"


MediaDecoder::MediaDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData):
mPrepared(false),
mPaused(false),
mStopped(false),
mFirstBuffered(true),
mName(name),
mFirstBufferedPktCount(firstBufferedPktCount),
mNonFirstBufferSecondsOfData(nonFirstBufferSecondsOfData)
{
    av_init_packet(&mDequeuePacket);
}

MediaDecoder::~MediaDecoder(){
    clearPktQueue();
}

void MediaDecoder::clearPktQueue(){
    for (auto& pktPtr : mPacketQueue){
        av_packet_unref(pktPtr.get());
    }
    mPacketQueue.clear();
}

void MediaDecoder::enqueuePacket(const AVPacket* pkt){
    auto packetPtr = std::make_shared<AVPacket>();
    av_copy_packet(packetPtr.get(), pkt);
    mPacketQueue.push_back(packetPtr);
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
        prepare();
    }
    
    std::chrono::duration<int, std::milli> duration(20);
    while (!mStopped){
        if (mPaused || !isFullBuffered()){
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
    
    std::cout << mName << " ends.\n";
}

void MediaDecoder::stop(){
    if (!mStopped){
        mStopped = true;
        std::cout << mName << " stopped.\n";
    }
}

void MediaDecoder::pause(){
    if (!mPaused){
        mPaused = true;
        std::cout << mName << " paused.\n";
    }
}

void MediaDecoder::resume(){
    if (mPaused){
        mPaused = false;
        std::cout << mName << " resumed.\n";
    }
}