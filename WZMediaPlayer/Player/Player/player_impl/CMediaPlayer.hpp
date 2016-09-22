//
//  CMediaPlayer.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef CMediaPlayer_hpp
#define CMediaPlayer_hpp



#include "IMediaPlayer.hpp"
#include "MediaSource.hpp"
#include "MediaDecoder.hpp"
#include "AudioDecoder.hpp"
#include "VideoDecoder.hpp"
#include "CommonInclude.h"
#include <thread>
#include <vector>
#include <map>
#include <iostream>

class AudioDecoder;
class VideoDecoder;



class CMediaPlayer : public IMediaPlayer {
    
public:
    virtual ~CMediaPlayer();
    CMediaPlayer(const std::string&& url) noexcept;
    
private:
    CMediaPlayer(CMediaPlayer& player) = delete;
    CMediaPlayer(CMediaPlayer&& player) = delete;
    CMediaPlayer& operator=(const CMediaPlayer& player) = delete;
    CMediaPlayer& operator=(const CMediaPlayer&& player) = delete;
    
private:
    void virtual start();
    void virtual pause();
    void virtual stop();
    uint64_t virtual getCurrentPosition();
    
private:
    void prepare();
    void startDecoding();
    void init();
    void prepareTasks();
    void prepareMediaObjects();
    void preparePacketQueue();
    void demuxerPacket();
    void enqueuePacket(AVPacket* packet);
    
private:
    std::map<std::string, std::thread::id> mThreadIDs;
    std::vector<std::thread> mThreadPool;
    std::map<std::string, std::function<void(void)>> mTasks;
    
private:
    std::string mUrl;
    bool mPrepared;
    bool mStarted;
    bool mPaused;
    bool mFirstBuffering;
    bool mStreamEnd;
    
    std::shared_ptr<MediaSource> mMediaSource;
    std::shared_ptr<MediaDecoder> mAudioDecoder;
    std::shared_ptr<MediaDecoder> mVideoDecoder;
    
    
    
};

#endif /* CMediaPlayer_hpp */
