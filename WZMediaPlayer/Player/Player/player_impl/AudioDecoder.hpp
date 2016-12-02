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


namespace WZ {
    namespace MediaPlayer {

const unsigned int kNumAQBufs = 3;
const unsigned int kAACSamplesPerPacket = 1024;
const size_t kAQBufSize = 128 * kAACSamplesPerPacket;

class AudioDecoder : public MediaDecoder {
public:
    AudioDecoder(uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream, std::shared_ptr<MediaState> state);
    ~AudioDecoder();
    
public:
    void setVolume(float volume);
    void getVolume(float* volume);
    int64_t* getAudioClockPtr();
    
protected:
    bool virtual prepare();
    Utility::PlayerState virtual decode(AVPacket* pkt);
    void virtual flush();
    
private:
    std::string virtual getClassName() const {return std::string("AudioDecoder");}
    
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
    Utility::PlayerState enqueueAudioPacket(const AVPacket* pkt);
    
private:
    static void MyAudioQueueOutputCallback(void* inClientData,
                                           AudioQueueRef inAQ,
                                           AudioQueueBufferRef		inBuffer);
    static void MyAudioQueueIsRunningCallback(void*				inClientData,
                                              AudioQueueRef		inAQ,
                                              AudioQueuePropertyID	inID);
    
private:
    AudioQueueBufferRef mAudioQueueBuffer[kNumAQBufs];
    int64_t audioClock[kNumAQBufs] = {0};
    bool inUse[kNumAQBufs] = {false, false, false};
    
    int64_t mCurrentAudioClock;
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
    }}

#endif /* AudioDecoder_hpp */
