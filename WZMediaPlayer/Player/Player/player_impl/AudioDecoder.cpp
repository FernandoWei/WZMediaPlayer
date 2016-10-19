//
//  AudioDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "AudioDecoder.hpp"

AudioDecoder::AudioDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream)
{
    MediaDecoder::MediaDecoder(std::move(name), firstBufferedPktCount, nonFirstBufferSecondsOfData, stream);
}

AudioDecoder::~AudioDecoder(){
    ;
}

bool AudioDecoder::prepare(){
    prepareAudioCodecParameters();
    if (setupAudioUnit()){
        return prepareAudioBuffer();
    }
    return false;
}

void AudioDecoder::prepareAudioCodecParameters(){
    AVCodecParameters* codec = mMediaStream->codecpar;
    mSampleRate = codec->sample_rate;
    mChannelsPerSample = codec->channels;
    mSampleSize = codec->frame_size;
    mSampleFormat = (AVSampleFormat)(codec->format);
}

bool AudioDecoder::prepareAudioBuffer(){
    try {
        mAudioBuffer = std::unique_ptr<uint8_t[]>(new uint8_t[AAC_FRAMES_PER_PACKET * 10]);
    } catch (const std::bad_alloc& e){
        std::cout << "Failed to allocate audio buffer.\n";
        return false;
    }
    return true;
}

PlayerState AudioDecoder::decode(AVPacket* pkt){
    PlayerState result = PlayerState::ERROR;
    enqueueAudioPacket(pkt);
    return result;
}

void AudioDecoder::enqueueAudioPacket(AVPacket* pkt){
}

void AudioDecoder::flush(){
    MediaDecoder::flush();
}



bool AudioDecoder::setupAudioUnit(){
    AudioComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_VoiceProcessingIO;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    
    AudioComponent inputComponent = AudioComponentFindNext(nullptr, &desc);
    OSStatus status = AudioComponentInstanceNew(inputComponent, &mAudioUnit);
    checkErrorWithReturn(status, "AudioComponentInstanceNew", false);
    UInt32 flag = 1;
    status = AudioUnitSetProperty(mAudioUnit,
                                  kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output,
                                  kOutputBus,
                                  &flag,
                                  sizeof(flag));
    checkErrorWithReturn(status, "AudioUnitSetProperty:kAudioOutputUnitProperty_EnableIO", false);
    
    AudioStreamBasicDescription audioFormat;
    audioFormat.mSampleRate = mSampleRate;
    audioFormat.mFormatID = kAudioFormatMPEG4AAC;
    audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger|kAudioFormatFlagIsPacked;
    audioFormat.mFramesPerPacket = AAC_FRAMES_PER_PACKET;
    audioFormat.mChannelsPerFrame = mChannelsPerSample;
    audioFormat.mBytesPerFrame = mSampleSize;
    audioFormat.mBitsPerChannel = (mSampleSize / mChannelsPerSample) * 8;
    audioFormat.mBytesPerPacket = mSampleSize * AAC_FRAMES_PER_PACKET;
    
    status = AudioUnitSetProperty(mAudioUnit, kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input, kOutputBus, &audioFormat, sizeof(audioFormat));
    checkErrorWithReturn(status, "AudioUnitSetProperty:kAudioUnitProperty_StreamFormat", false);
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = playbackCallback;
    callbackStruct.inputProcRefCon = this;
    status = AudioUnitSetProperty(mAudioUnit, kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Global, kOutputBus, &callbackStruct, sizeof(callbackStruct));
    checkErrorWithReturn(status, "AudioUnitSetProperty:kAudioUnitProperty_SetRenderCallback", false);
    status = AudioUnitInitialize(mAudioUnit);
    checkErrorWithReturn(status, "AudioUnitInitialize", false);
    return true;
}
