//
//  VideoDecoder.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "VideoDecoder.hpp"

namespace WZ {
    namespace MediaPlayer {
        
        VideoDecoder::VideoDecoder(uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream, std::shared_ptr<MediaState> state)
        :MediaDecoder(firstBufferedPktCount, nonFirstBufferSecondsOfData, stream, state)
        {
        }
        
        VideoDecoder::~VideoDecoder(){
            if (mPixelBuffer){
                CFRelease(mPixelBuffer);
                mPixelBuffer = nullptr;
            }
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
            
            if (!mSPS.size() || !mPPS.size()){
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
                    log("invalid codec data.");
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
                                         CMTime presentationDuration)
        {
            if (noErr != status) {
                return;
            }
            CVPixelBufferRef *outputPixelBuffer = (CVPixelBufferRef *)sourceFrameRefCon;
            *outputPixelBuffer = CVPixelBufferRetain(pixelBuffer);
        }
        
        Utility::PlayerState VideoDecoder::decode(AVPacket* pkt){
            Utility::PlayerState result = Utility::PlayerState::OK;
            mMediaState->setVideoReady(true);
            if (!pkt || !(pkt->data) || pkt->size < 4){
                return Utility::PlayerState::ERROR;
            }
            if (!duplicatePacket(pkt)){
                return Utility::PlayerState::ERROR;
            }
            if (mNeedRedecodeDuplicatedPkt){
                return redecodeDuplicatedVideoPkt();
            }
            
            if (mPktInfoArrayEnd > 0){
                mPktInfo = &mPktInfoArray[mPktInfoArrayEnd - 1];
                result = createImage();
                if (result == Utility::PlayerState::OK){
                    result = enqueueImage();
                    if (result == Utility::PlayerState::OK){
                        result = renderImage();
                    }
                }
            }
            return result;
        }
        
        Utility::PlayerState VideoDecoder::renderImage(){
            if (!mImageMap.empty()){
                mMediaState->synchronize(mImageMap.begin()->first, mStopped);
                auto pixelBuffer = mImageMap.begin()->second->pixelBuff;
                if (pixelBuffer){
                    auto picture = (CVPixelBufferRef)CFRetain(pixelBuffer);
                    //            iOSNativeWindow_dispatchPixelBuffer((void*)picture);
                }
                mImageMap.erase(mImageMap.begin());
            }
            return Utility::PlayerState::OK;
        }
        
        bool VideoDecoder::isKeyVideoPkt(AVPacket* pkt){
            bool result = false;
            uint32_t keyIndex = 4;
            uint32_t currentSegmentLength = 0;
            uint32_t size = pkt->size;
            while (keyIndex < size){
                auto type = Utility::getNALType(pkt->data, keyIndex);
                if (Utility::NALType::NAL_Slice == type){
                    break;
                }else if (Utility::NALType::NAL_IDR_Slice == type){
                    result = true;
                    break;
                }else{
                    currentSegmentLength = Utility::getNALSize(pkt->data, keyIndex);
                    keyIndex += currentSegmentLength + 4;
                    continue;
                }
            }
            return result;
        }
        
        bool VideoDecoder::duplicatePacket(AVPacket* pkt){
            if (isKeyVideoPkt(pkt)){
                cleanupBufferedPktInfo();
                mPktInfoArray[mPktInfoArrayEnd].isKeyPkt = true;
            }
            
            if (mPktInfoArrayEnd + 1 >= mPktInfoArrayCapacity){
                if (!resizePktInfoArray()){
                    return false;
                }
            }
            mPktInfoArray[mPktInfoArrayEnd].begin = mPktDataPoolEnd;
            mPktInfoArray[mPktInfoArrayEnd].pts   = pkt->pts;
            mPktInfoArray[mPktInfoArrayEnd].dts   = pkt->dts;
            mPktInfoArray[mPktInfoArrayEnd].size  = pkt->size;
            mPktInfoArrayEnd++;
            
            if (mPktDataPoolEnd + pkt->size >= mPktDataPoolCapacity){
                if (!resizePktDataPool()){
                    return false;
                }
            }
            memcpy(mPktDataPool.get() + mPktDataPoolEnd, pkt->data, pkt->size);
            mPktDataPoolEnd += pkt->size;
            return true;
        }
        
        bool VideoDecoder::resizePktDataPool(){
            uint8_t* dataPtr = nullptr;
            try {
                dataPtr = new uint8_t[mPktDataPoolCapacity * 2];
            } catch (const std::bad_alloc& e){
                //        log("failed to resize pktDataPool.");
                return false;
            }
            memmove(dataPtr, mPktDataPool.get(), mPktDataPoolEnd);
            mPktDataPool.reset(dataPtr);
            mPktDataPoolCapacity *= 2;
            return true;
        }
        
        bool VideoDecoder::resizePktInfoArray(){
            PktInfo* dataPtr = nullptr;
            try {
                dataPtr = new PktInfo[mPktInfoArrayCapacity * 2];
            } catch (const std::bad_alloc& e) {
                //        log("failed to resize pktInfoArray.");
                return false;
            }
            memmove(dataPtr, mPktInfoArray.get(), mPktInfoArrayEnd * sizeof(PktInfo));
            mPktInfoArray.reset(dataPtr);
            mPktInfoArrayCapacity *= 2;
            return true;
        }
        
        Utility::PlayerState VideoDecoder::redecodeDuplicatedVideoPkt(){
            Utility::PlayerState result = Utility::PlayerState::OK;
            if (mPktInfoArrayEnd > 0 && !(mPktInfoArray[mPktInfoArrayEnd - 1].isKeyPkt)){
                uint32_t index = 0;
                while (index < mPktInfoArrayEnd){
                    mPktInfo = &mPktInfoArray[index];
                    if (Utility::PlayerState::INVALID_DECODER_SESSION == createImage()) {
                        index = 0;
                        continue;
                    }
                    ++index;
                }
            }
            mNeedRedecodeDuplicatedPkt = false;
            return result;
        }
        
        bool VideoDecoder::resetDecoderSession(){
            bool result = false;
            if (mDecoderSession){
                VTDecompressionSessionInvalidate(mDecoderSession);
                CFRelease(mDecoderSession);
                mDecoderSession = nullptr;
                if (mDecoderFormatDescription){
                    CFRelease(mDecoderFormatDescription);
                    mDecoderFormatDescription = nullptr;
                }
                result = initDecoder();
                if (result){
                    mNeedRedecodeDuplicatedPkt = true;
                } else {
                    //            log("Failed to init decoder.");
                }
            }
            return result;
        }
        
        Utility::PlayerState VideoDecoder::createImage(){
            Utility::PlayerState result = Utility::PlayerState::OK;
            if (!mPktInfo || mPktInfo->size < 4){
                return Utility::PlayerState::ERROR;
            }
            if (mPktInfo->isKeyPkt){
                //        extractIDRIfNecessary(*mPktInfo);
            }
            
            if (mPixelBuffer){
                CFRelease(mPixelBuffer);
                mPixelBuffer = nullptr;
            }
            CMBlockBufferRef blockBuffer = nullptr;
            OSStatus status  = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                                                  (void*)(mPktDataPool.get() + mPktInfo->begin),
                                                                  (size_t)mPktInfo->size,
                                                                  kCFAllocatorNull,
                                                                  nullptr, 0, (size_t)(mPktInfo->size),
                                                                  0, &blockBuffer);
            if(status == kCMBlockBufferNoErr){
                CMSampleBufferRef sampleBuffer = nullptr;
                const size_t sampleSizeArray[] = {(size_t)(mPktInfo->size)};
                status = CMSampleBufferCreateReady(kCFAllocatorDefault,
                                                   blockBuffer,
                                                   mDecoderFormatDescription,
                                                   1, 0, nullptr, 1, sampleSizeArray,
                                                   &sampleBuffer);
                if (status == kCMBlockBufferNoErr && sampleBuffer){
                    if (mMediaState->getCurrentDiscardState() == Utility::DiscardFrameType::DISCARD_DEFAULT){
                        mDecodeFrameFlags = kVTDecodeFrame_DoNotOutputFrame;
                    }else {
                        mDecodeFrameFlags = 0;
                    }
                    VTDecodeInfoFlags flagOut = 0;
                    auto decodeStatus = VTDecompressionSessionDecodeFrame(mDecoderSession,
                                                                          sampleBuffer,
                                                                          mDecodeFrameFlags,
                                                                          &mPixelBuffer,
                                                                          &flagOut);
                    if(decodeStatus == kVTInvalidSessionErr){
                        //                 log("invalid session, reset decoder session.");
                        if (resetDecoderSession()){
                            if (mLastFramePTS < mPktInfo->pts){
                                mLastFramePTS = mPktInfo->pts;
                            }
                            result = Utility::PlayerState::INVALID_DECODER_SESSION;
                        } else {
                            result = Utility::PlayerState::ERROR;
                        }
                    }else if(decodeStatus != noErr){
                        log("decode failed status= ", decodeStatus);
                        result = Utility::PlayerState::IGNORE;
                    }
                    CFRelease(sampleBuffer);
                }
                CFRelease(blockBuffer);
            }
            return result;
        }
        
        Utility::PlayerState VideoDecoder::enqueueImage(){
            Utility::PlayerState result = Utility::PlayerState::OK;
            if (!mPixelBuffer){
                if (mDecodeFrameFlags == kVTDecodeFrame_DoNotOutputFrame){
                    //            log("Decoder discarded current frame.");
                }else{
                    //            log("Empty pixelBuffer after decoded! Just ignore it!");
                    return Utility::PlayerState::IGNORE;
                }
            }
            
            auto imagePtr = std::shared_ptr<image>(new image, [](image* ptr){
                if (ptr->pixelBuff){
                    CFRelease(ptr->pixelBuff);
                    ptr->pixelBuff = nullptr;
                }
                delete ptr;
            });
            
            imagePtr->pts = mPktInfo->pts;
            imagePtr->dts = mPktInfo->dts;
            if (mPixelBuffer){
                imagePtr->pixelBuff = (CVPixelBufferRef)CFRetain(mPixelBuffer);
            }
            mImageMap.insert(std::make_pair(mPktInfo->pts, std::move(imagePtr)));
            if (mImageMap.size() < 4){
                result = Utility::PlayerState::NEED_MORE_DATA;
            }
            return result;
        }
        
        void VideoDecoder::flush(){
            MediaDecoder::flush();
        }
        

        
    }
}
