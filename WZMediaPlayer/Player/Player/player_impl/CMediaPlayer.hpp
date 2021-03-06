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
#include "AudioDecoder.hpp"
#include "VideoDecoder.hpp"


namespace WZ {
    namespace MediaPlayer {

class CMediaPlayer : public IMediaPlayer, public Utility::WZObject {
    
public:
    virtual ~CMediaPlayer();
    CMediaPlayer(std::string&& url) noexcept;
    
private:
    CMediaPlayer(CMediaPlayer& player) = delete;
    CMediaPlayer(CMediaPlayer&& player) = delete;
    CMediaPlayer& operator=(const CMediaPlayer& player) = delete;
    CMediaPlayer& operator=(const CMediaPlayer&& player) = delete;
    
private:
    void virtual start();
    void virtual stop();
    void virtual pause();
    void virtual resume();
    uint64_t virtual getCurrentPosition();
    std::string virtual getClassName() const {return std::string("CMediaPlayer");}
    
private:
    Utility::PlayerState prepare();
    void startDecoding();
    Utility::PlayerState prepareTasks();
    Utility::PlayerState prepareMediaSource();
    Utility::PlayerState prepareMediaDecoder();
    void readAndEnqueuePacket();
    void demuxerPacket();
    void enqueuePacket(AVPacket* packet);
    void flush();
    
private:
    std::map<std::string, std::thread::id> mThreadIDs;
    std::vector<std::thread> mThreadPool;
    std::map<std::string, std::function<void(void)>> mTasks;
    
private:
    const std::string mUrl;
    bool mPrepared;
    bool mStarted;
    bool mPaused;
    bool mStreamEnd;
    
    std::shared_ptr<MediaSource> mMediaSource;
    std::shared_ptr<MediaDecoder> mAudioDecoder;
    std::shared_ptr<MediaDecoder> mVideoDecoder;
    std::shared_ptr<MediaState> mMediaState;
};
        
}
}

#endif /* CMediaPlayer_hpp */
