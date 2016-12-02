//
//  MediaLog.hpp
//  WZMediaPlayer
//
//  Created by Hongqiang.Wei on 16/10/23.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#ifndef MediaLog_hpp
#define MediaLog_hpp

#include "WZObject.hpp"

#include <string>
#include <thread>
#include <iostream>



namespace WZ {
    namespace Utility {
        
        class WZObject;
        
        class MediaLog {
            MediaLog(const MediaLog& log) = delete;
            MediaLog(MediaLog&& log) = delete;
            MediaLog& operator=(const MediaLog& log) = delete;
            MediaLog& operator=(MediaLog&& log) = delete;
            
        public:
            MediaLog();
            ~MediaLog(){}
            void log(std::shared_ptr<WZ::Utility::WZObject> mediaObject, const char* format, va_list va);
            
        private:
            void printCurrentTimeAndClassname(std::shared_ptr<WZ::Utility::WZObject> mediaObject) const;
            
        private:
            std::mutex mMutex;
        };
    }
}

#endif /* MediaLog_hpp */
