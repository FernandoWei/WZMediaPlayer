//
//  AudioDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "AudioDecoder.hpp"

AudioDecoder::AudioDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream, std::shared_ptr<MediaState> state):
mAudioQueue(nullptr),
mSampleRate(0),
mChannelsPerSample(0),
mCurrentAudioClock(0),
mAudioQueueStarted(false),
mVolume(1.0)
{
    MediaDecoder::MediaDecoder(std::move(name), firstBufferedPktCount, nonFirstBufferSecondsOfData, stream, state);
}

AudioDecoder::~AudioDecoder(){
    audioQueueDispose();
}

bool AudioDecoder::prepare(){
    if (prepareCodecParameters()){
        return setupAudioQueue();
    }
    return false;
}

bool AudioDecoder::prepareCodecParameters(){
    if (mMediaStream){
        AVCodecParameters* codec = mMediaStream->codecpar;
        mSampleRate = codec->sample_rate;
        mChannelsPerSample = codec->channels;
        return true;
    }
    return false;
}


PlayerState AudioDecoder::decode(AVPacket* pkt){
    return enqueueAudioPacket(pkt);
}


void AudioDecoder::flush(){
    MediaDecoder::flush();
    audioQueueFlush();
}

PlayerState AudioDecoder::enqueueAudioPacket(const AVPacket* pkt){
    if (!pkt || !pkt->data || pkt->size > kAQBufSize){
        std::cout << "Invalid audio packet for input.\n";
        return PlayerState::IGNORE;
    }
    
    if (waitForFreeBuffer()){
        inUse[mCurrentAvailableBuffIndex] = true;
        audioClock[mCurrentAvailableBuffIndex] = pkt->pts;
        AudioQueueBufferRef aqBuffer = mAudioQueueBuffer[mCurrentAvailableBuffIndex];
        aqBuffer->mAudioDataByteSize = pkt->size;
        memcpy(aqBuffer->mAudioData, pkt->data, pkt->size);
        OSStatus err = AudioQueueEnqueueBuffer(mAudioQueue, aqBuffer, 0, nullptr);
        checkErrorWithReturn(err, "AudioQueueEnqueueBuffer", PlayerState::ERROR);
        startAudioQueueIfNeeded();
    }
    return PlayerState::OK;
}

bool AudioDecoder::waitForFreeBuffer(){
    bool result = true;
    if (++mCurrentAvailableBuffIndex >= kNumAQBufs){
        mCurrentAvailableBuffIndex = 0;
    }
    
    std::lock_guard<std::mutex> guard(mMutexForSynchronize);
    while (inUse[mCurrentAvailableBuffIndex]){
        std::unique_lock<std::mutex> unique_lock(mMutexForSynchronize);
        auto now = std::chrono::system_clock::now();
        if (mCondition.wait_until(unique_lock, now + std::chrono::milliseconds(500)) == std::cv_status::timeout){
            result = false;
            break;
        }
    }
    return result;
}

void AudioDecoder::MyAudioQueueOutputCallback(void*	inClientData, AudioQueueRef	inAQ, AudioQueueBufferRef inBuffer){
    AudioDecoder* thiz = (AudioDecoder*)inClientData;
    int index = thiz->findCurrentAvailableBuffIndex(inBuffer);
    if (index < 0){
        return;
    }
    
    std::lock_guard<std::mutex> guard(thiz->mMutexForSynchronize);
    thiz->inUse[index] = false;
    thiz->mCurrentAudioClock = thiz->audioClock[index];
    thiz->mMediaState->setAudioReady(true);
    thiz->mCondition.notify_one();
    if (!thiz->inUse[0] && !thiz->inUse[1] && !thiz->inUse[2]){
        thiz->audioQueueStop(true);
    }
}

bool AudioDecoder::setupAudioQueue(){
    OSStatus err = noErr;
    if (!mAudioQueue){
        AudioStreamBasicDescription asbd = {0};
        asbd.mFormatID = kAudioFormatMPEG4AAC;
        asbd.mSampleRate = mSampleRate;
        asbd.mFramesPerPacket = kAACSamplesPerPacket;
        asbd.mChannelsPerFrame = mChannelsPerSample;
        asbd.mBytesPerFrame = mChannelsPerSample * 2;
        asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
        asbd.mBytesPerPacket = asbd.mFramesPerPacket * asbd.mBytesPerFrame;
        asbd.mBitsPerChannel = 16;
        asbd.mReserved = 0;
        
        err = AudioQueueNewOutput(&asbd, MyAudioQueueOutputCallback, this, nullptr, nullptr, 0, &mAudioQueue);
        checkErrorWithReturn(err, "AudioQueueNewOutput", false);
        
        for (unsigned int i = 0; i < kNumAQBufs; ++i){
            err = AudioQueueAllocateBuffer(mAudioQueue, kAQBufSize, &mAudioQueueBuffer[i]);
            inUse[i] = false;
            checkErrorWithReturn(err, "AudioQueueAllocateBuffer", false);
        }
        
        err = AudioQueueAddPropertyListener(mAudioQueue, kAudioQueueProperty_IsRunning, MyAudioQueueIsRunningCallback, this);
        checkErrorWithReturn(err, "set kAudioQueueProperty_IsRunning", false);
        
        mCurrentAudioClock = 0;
        mCurrentAvailableBuffIndex = 0;
    }
    return true;
}

int AudioDecoder::findCurrentAvailableBuffIndex(AudioQueueBufferRef buffer){
    for (int i = 0; i < kNumAQBufs; i++){
        if (buffer == this->mAudioQueueBuffer[i]){
            return i;
        }
    }
    return -1;
}



void AudioDecoder::MyAudioQueueIsRunningCallback(void* inClientData, AudioQueueRef inAQ, AudioQueuePropertyID inID){
    UInt32 running;
    UInt32 size = sizeof(UInt32);
    OSStatus err = AudioQueueGetProperty(inAQ, kAudioQueueProperty_IsRunning, &running, &size);
    checkError(err, "get kAudioQueueProperty_IsRunning");
    if (!running){
    }
}

void AudioDecoder::audioQueueFlush(){
    OSStatus err = noErr;
    if (mAudioQueue && mAudioQueueStarted){
        err = AudioQueueFlush(mAudioQueue);
        checkError(err, "AudioQueueFlush");
    }
}

void AudioDecoder::startAudioQueueIfNeeded(){
    std::lock_guard<std::mutex> guard(mMutexForStartAQ);
    if (!mAudioQueueStarted && mAudioQueue){
        if (mPaused || mStopped){
            std::cout << "AQ starting was rejected when Paused or Stopped.\n";
            return;
        }
        OSStatus err = AudioQueueStart(mAudioQueue, nullptr);
        checkError(err, "AudioQueueStart");
        mAudioQueueStarted = true;
        
        err = AudioQueueSetParameter(mAudioQueue, kAudioQueueParam_Volume, mVolume);
        checkError(err, "set kAudioQueueParam_Volume");
    }
}

void AudioDecoder::getVolume(float* volume){
    *volume = mVolume;
}

uint32_t* AudioDecoder::getAudioClockPtr(){
    return &mCurrentAudioClock;
}

void AudioDecoder::audioQueueStop(bool didStopImmediately){
    std::lock_guard<std::mutex> guard(mMutexForStartAQ);
    if (mAudioQueue && mAudioQueueStarted){
        OSStatus err = AudioQueueStop(mAudioQueue, didStopImmediately);
        checkError(err, "AudioQueueStop");
        mAudioQueueStarted = false;
    }
}

void AudioDecoder::audioQueueDispose(){
    audioQueueFlush();
    audioQueueStop(false);
    if (mAudioQueue){
        AudioQueueDispose(mAudioQueue, false);
        mAudioQueue = nullptr;
        
    }
}
