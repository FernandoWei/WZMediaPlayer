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

#define kOutputBus 0
#define AAC_FRAMES_PER_PACKET 1024

class AudioDecoder : public MediaDecoder {
public:
    AudioDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream);
    ~AudioDecoder();
    
protected:
    bool virtual prepare();
    PlayerState virtual decode(AVPacket* pkt);
    void virtual flush();
    
private:
    static OSStatus playbackCallback(void *inRefCon,
                                     AudioUnitRenderActionFlags *ioActionFlags,
                                     const AudioTimeStamp *inTimeStamp,
                                     UInt32 inBusNumber,
                                     UInt32 inNumberFrames,
                                     AudioBufferList *ioData);
    bool setupAudioUnit();
    void prepareAudioCodecParameters();
    bool prepareAudioBuffer();
    void enqueueAudioPacket(AVPacket* pkt);
    
private:
    AudioComponentInstance mAudioUnit;
    
    int mSampleRate;
    int mChannelsPerSample;
    int mBytesPerPacket;
    int mSampleSize;
    AVSampleFormat mSampleFormat;
    
    std::unique_ptr<uint8_t[]> mAudioBuffer;
    
};

#endif /* AudioDecoder_hpp */
