//
//  VideoDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "VideoDecoder.hpp"

VideoDecoder::VideoDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream)
{
    MediaDecoder::MediaDecoder(std::move(name), firstBufferedPktCount, nonFirstBufferSecondsOfData, stream);
}

VideoDecoder::~VideoDecoder(){
    ;
}

bool VideoDecoder::prepare(){
    return prepareDecoder();
}

void VideoDecoder::reset(){
    cleanupDecoder();
    cleanupBufferedPktInfo();
    cleanupPixelBuffList();
    releaseDuplicatedPktBuffer();
}

void VideoDecoder::cleanupDecoder(){
    if(mDecoderSession){
        VTDecompressionSessionInvalidate(mDecoderSession);
        CFRelease(mDecoderSession);
        mDecoderSession = nullptr;
    }
    
    if(mDecoderFormatDescription){
        CFRelease(mDecoderFormatDescription);
        mDecoderFormatDescription = nullptr;
    }
}

void VideoDecoder::cleanupBufferedPktInfo(){
    mPktInfoArrayEnd = 0;
    mPktDataPoolEnd = 0;
}

void VideoDecoder::releaseDuplicatedPktBuffer(){
    mPktDataPool.reset();
    mPktInfoArray.reset();
}

void VideoDecoder::cleanupPixelBuffList(void){
    mImageMap.clear();
}

bool VideoDecoder::prepareDecoder(){
    bool result = false;
    reset();
    if (extractSPSAndPPS()){
        if (prepareDuplicatedPktBuffer() && initDecoder()){
            result = true;
        }
    }
    return result;
}

bool VideoDecoder::prepareDuplicatedPktBuffer(){
    try {
        mPktDataPool.reset(new uint8_t[MAX_DATA_SIZE]);
    } catch (const std::bad_alloc& e) {
        return false;
    }
    
    try {
        mPktInfoArray.reset(new PktInfo[MAX_PKT_COUNT]);
    } catch (const std::bad_alloc& e) {
        return false;
    }
    return true;
}

bool VideoDecoder::initDecoder()
{
    if (mDecoderSession && mDecoderFormatDescription){
        return true;
    }
    
    if (mSPS.size() == 0 || mPPS.size() == 0){
        return false;
    }
    
    const uint8_t* const parameterSetPointers[2] = { mSPS.data(), mPPS.data() };
    const size_t parameterSetSizes[2] = { mSPS.size(), mPPS.size() };
    auto status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                      2,
                                                                      parameterSetPointers,
                                                                      parameterSetSizes,
                                                                      4,
                                                                      &mDecoderFormatDescription);
    
    if(status == noErr){
        CFDictionaryRef attrs = nullptr;
        const void *keys[] = { kCVPixelBufferPixelFormatTypeKey };
        uint32_t v = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
        const void *values[] = { CFNumberCreate(nullptr, kCFNumberSInt32Type, &v) };
        attrs = CFDictionaryCreate(nullptr, keys, values, 1, nullptr, nullptr);
        VTDecompressionOutputCallbackRecord callBackRecord;
        callBackRecord.decompressionOutputCallback = didDecompress;
        callBackRecord.decompressionOutputRefCon = nullptr;
        status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                              mDecoderFormatDescription,
                                              nullptr, attrs,
                                              &callBackRecord,
                                              &mDecoderSession);
        if (attrs){
            CFRelease(attrs);
            attrs = nullptr;
        }
        if (noErr != status){
            return false;
        }
    }else{
    }
    
    return true;
}

bool VideoDecoder::extractSPSAndPPS(){
    bool result = false;
    if (mMediaStream){
        AVCodecParameters* codecPar = mMediaStream->codecpar;
        if (codecPar && codecPar->extradata && codecPar->extradata[0] == 0x01 && codecPar->extradata_size > 8){
            uint8_t spsOffsetIndex = 8;
            uint8_t spsLength = codecPar->extradata[spsOffsetIndex - 1];
            mSPS.resize(spsLength);
            memcpy(mSPS.data(), &codecPar->extradata[spsOffsetIndex], spsLength);
            uint8_t ppsOffsetIndex = spsOffsetIndex + spsLength + 3;
            uint8_t ppsLength = codecPar->extradata[ppsOffsetIndex - 1];
            mPPS.resize(ppsLength);
            memcpy(mPPS.data(), &codecPar->extradata[ppsOffsetIndex], ppsLength);
            result = true;
        } else {
            std::cout << "invalid codec data.\n";
        }
    }
    return result;
}

void VideoDecoder::didDecompress(
                                 void *decompressionOutputRefCon,
                                 void *sourceFrameRefCon,
                                 OSStatus status,
                                 VTDecodeInfoFlags infoFlags,
                                 CVImageBufferRef pixelBuffer,
                                 CMTime presentationTimeStamp,
                                 CMTime presentationDuration )
{
    if (noErr != status) {
        return;
    }
    CVPixelBufferRef *outputPixelBuffer = (CVPixelBufferRef *)sourceFrameRefCon;
    *outputPixelBuffer = CVPixelBufferRetain(pixelBuffer);
}

PlayerState VideoDecoder::decode(AVPacket* pkt){
    PlayerState result = PlayerState::ERROR;
    if (pkt){
        result = decodeVideoPkt(pkt);
    } else {
        ;
    }
    return result;
}

PlayerState VideoDecoder:: decodeVideoPkt(AVPacket* pkt){
    if (!pkt || !(pkt->data) || pkt->size < 4){
        return PlayerState::ERROR;
    }
    if (!duplicatePacket()){
        return PlayerState::ERROR;
    }
    if (mRedecodeFromPreviousIDRFrame){
        return redecodeDuplicatedVideoPkt();
    }
    PlayerState result = createImage();
    if (result != PlayerState::ERROR){
        result = renderImage();
    }
    return result;
}

PlayerState VideoDecoder::createImage(){
    PlayerState result = PlayerState::ERROR;
    return result;
}

PlayerState VideoDecoder::renderImage(){
    PlayerState result = PlayerState::ERROR;
    return result;
}

void VideoDecoder::flush(){
    MediaDecoder::flush();
}
