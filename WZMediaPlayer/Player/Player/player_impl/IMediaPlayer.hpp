//
//  IMediaPlayer.hpp
//  WZMediaPlayer
//
//  Created by fernando on 16/9/22.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef IMediaPlayer_hpp
#define IMediaPlayer_hpp

#include <string>

class IMediaPlayer {
    
public:
    IMediaPlayer(const IMediaPlayer& player) = delete;
    IMediaPlayer(IMediaPlayer&& player) noexcept = delete;
    IMediaPlayer& operator=(const IMediaPlayer& player) = delete;
    IMediaPlayer& operator=(const IMediaPlayer&& player) = delete;
    virtual ~IMediaPlayer(){}
    
protected:
    IMediaPlayer(){}
    
public:
    void virtual start() = 0;
    void virtual pause() = 0;
    void virtual stop() = 0;
    void virtual resume() = 0;
    uint64_t virtual getCurrentPosition() = 0;
    
public:
    static std::shared_ptr<IMediaPlayer> createPlayer(std::string&& url);
};

#endif /* MediaPlayer_hpp */
