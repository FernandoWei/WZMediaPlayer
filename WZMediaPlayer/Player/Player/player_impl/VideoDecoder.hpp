//
//  VideoDecoder.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef VideoDecoder_hpp
#define VideoDecoder_hpp

#include "MediaDecoder.hpp"
#include <videotoolbox/VideoToolbox.h>

#define MAX_DATA_SIZE 10485760  //10 MB
#define MAX_PKT_COUNT 200  //默认最大gop长度

typedef struct _pktInfo {
    uint32_t begin;
    uint32_t size;
    int64_t pts;
    int64_t dts;
    bool isKeyPkt;
    _pktInfo(){
        begin = 0;
        size = 0;
        pts = 0;
        dts = 0;
        isKeyPkt = false;
    }
} PktInfo;

typedef struct _image
{
    int64_t pts;
    int64_t dts;
    CVPixelBufferRef pixelBuff;
    
    _image(){
        pts = 0;
        dts = 0;
        pixelBuff = NULL;
    }
    
} image;

class VideoDecoder : public MediaDecoder {
public:
    VideoDecoder(std::string&& name, uint8_t firstBufferedPktCount, uint8_t nonFirstBufferSecondsOfData, AVStream* stream, std::shared_ptr<MediaState> state);
    ~VideoDecoder();
    
public:
    bool virtual prepare();
    PlayerState virtual decode(AVPacket* pkt);
    void flush();
    virtual std::string toString() const;
    
private:
    PlayerState redecodeDuplicatedVideoPkt();
    PlayerState createImage();
    PlayerState enqueueImage();
    PlayerState renderImage();
    bool prepareDecoder();
    bool extractSPSAndPPS();
    void resetHWDecoder();
    bool initDecoder();
    bool duplicatePacket(AVPacket* pkt);
    bool isKeyVideoPkt(AVPacket* pkt);
    bool resizePktDataPool();
    bool resizePktInfoArray();
    void synchronize();
    void reset();
    void cleanupDecoder();
    inline void cleanupPixelBuffList();
    void cleanupBufferedPktInfo();
    void releaseDuplicatedPktBuffer();
    bool prepareDuplicatedPktBuffer();
    bool resetDecoderSession();
    static void didDecompress( void *decompressionOutputRefCon, void *sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags infoFlags, CVImageBufferRef pixelBuffer, CMTime presentationTimeStamp, CMTime presentationDuration );
    
private:
    std::vector<uint8_t> mSPS;
    std::vector<uint8_t> mPPS;
    
    VTDecompressionSessionRef mDecoderSession;
    CMVideoFormatDescriptionRef mDecoderFormatDescription;
    VTDecodeFrameFlags mDecodeFrameFlags;
    CVPixelBufferRef mPixelBuffer;
    std::multimap<int64_t, std::shared_ptr<image>> mImageMap;
    
    std::unique_ptr<uint8_t[]> mPktDataPool;
    uint32_t mPktDataPoolEnd;
    uint32_t mPktDataPoolCapacity;
    std::unique_ptr<PktInfo[]> mPktInfoArray;
    uint32_t mPktInfoArrayEnd;
    uint32_t mPktInfoArrayCapacity;
    
    bool mNeedRedecodeDuplicatedPkt;
    int64_t mTrackStartTime;
    int mOffset;
    int64_t mLastFramePTS;
    AVDiscard* mVideoSkipFrame;
    PktInfo* mPktInfo;
    bool mIsEmptyImageCausedOfInvalidDecodeSession;
};

#endif /* VideoDecoder_hpp */
