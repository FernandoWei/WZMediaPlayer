//
//  AudioDecoder.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef AudioDecoder_hpp
#define AudioDecoder_hpp

#include "MediaDecoder.hpp"
#include <AudioToolbox/AudioToolbox.h>

const unsigned int kNumAQBufs = 3;
const unsigned int kAACSamplesPerPacket = 1024;
const size_t kAQBufSize = 128 * kAACSamplesPerPacket;

class AudioDecoder : public MediaDecoder {
public:
    AudioDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream, std::shared_ptr<MediaState> state);
    ~AudioDecoder();
    
public:
    void setVolume(float volume);
    void getVolume(float* volume);
    uint32_t* getAudioClockPtr();
    
protected:
    bool virtual prepare();
    PlayerState virtual decode(AVPacket* pkt);
    void virtual flush();
    virtual std::string toString() const;
    
private:
    bool prepareCodecParameters();
    bool setupAudioQueue();
    void startAudioQueueIfNeeded();
    int findCurrentAvailableBuffIndex(AudioQueueBufferRef buffer);
    bool waitForFreeBuffer();
    
    void audioQueueResume();
    void audioQueuePause();
    void audioQueueFlush();
    void audioQueueStop(bool didStopImmediately);
    void audioQueueDispose();
    PlayerState enqueueAudioPacket(const AVPacket* pkt);
    
private:
    static void MyAudioQueueOutputCallback(void* inClientData,
                                           AudioQueueRef inAQ,
                                           AudioQueueBufferRef		inBuffer);
    static void MyAudioQueueIsRunningCallback(void*				inClientData,
                                              AudioQueueRef		inAQ,
                                              AudioQueuePropertyID	inID);
    
private:
    AudioQueueBufferRef mAudioQueueBuffer[kNumAQBufs];
    uint32_t audioClock[kNumAQBufs] = {0};
    bool inUse[kNumAQBufs] = {false, false, false};
    
    uint32_t mCurrentAudioClock;
    AudioQueueRef mAudioQueue;
    bool mAudioQueueStarted;
    int mCurrentAvailableBuffIndex;
    float mVolume;
    
    std::mutex mMutexForSynchronize;
    std::mutex mMutexForStartAQ;
    std::condition_variable mCondition;
    
    int mSampleRate;
    int mChannelsPerSample;
    int mBytesPerPacket;
    int mSampleSize;
    AVSampleFormat mSampleFormat;
    
};

#endif /* AudioDecoder_hpp */
