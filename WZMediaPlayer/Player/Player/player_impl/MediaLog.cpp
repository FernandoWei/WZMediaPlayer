//
//  MediaLog.cpp
//  WZMediaPlayer
//
//  Created by Hongqiang.Wei on 16/10/23.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "MediaLog.hpp"
#include <iomanip>

namespace WZ {
    namespace Utility {
        
        MediaLog::MediaLog():mMutex(){}
        
        void MediaLog::log(std::shared_ptr<WZ::Utility::WZObject> mediaObject, const char* format, va_list va){
            std::lock_guard<std::mutex> guard(mMutex);
            printCurrentTimeAndClassname(mediaObject);
            std::printf(format, va);
        }
        
        void MediaLog::printCurrentTimeAndClassname(std::shared_ptr<WZ::Utility::WZObject> mediaObject) const {
            auto now = std::chrono::system_clock::now();
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
            auto now_ms_part = now_ms - std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
            
            auto now_time_t = std::chrono::system_clock::to_time_t(now);
            struct std::tm* tm = std::localtime(&now_time_t);
            std::printf("%4d-%2d-%2d %2d:%2d:%2d.%3lld", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, now_ms_part.count());
            std::cout << " [" << mediaObject->getClassName() << "]: ";
        }
    }
}
