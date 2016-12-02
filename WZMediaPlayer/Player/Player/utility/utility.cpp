//
//  utility.cpp
//  WZMediaPlayer
//
//  Created by fernando on 16/12/2.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#include "utility.hpp"

namespace WZ {
    namespace Utility {
        
        NALType getNALType(uint8_t* data, int index){
            NALType type;
            switch(data[index] & 0x1F){
                case 0x01:{
                    type = NALType::NAL_Slice;
                    break;
                }case 0x02:{
                    type = NALType::NAL_PA;
                    break;
                }case 0x03:{
                    type = NALType::NAL_PB;
                    break;
                }case 0x04:{
                    type = NALType::NAL_PC;
                    break;
                }case 0x05:{
                    type = NALType::NAL_IDR_Slice;
                    break;
                }case 0x06:{
                    type = NALType::NAL_SEI;
                    break;
                }case 0x07:{
                    type = NALType::NAL_SPS;
                    break;
                }case 0x08:{
                    type = NALType::NAL_PPS;
                    break;
                }case 0x09:{
                    type = NALType::NAL_AUD;
                    break;
                }default:{
                    type = NALType::NAL_ERR;
                    break;
                }
            }
            return type;
        }
        
        uint32_t getNALSize(uint8_t* data, int index){
            return data[index - 1]
            + (data[index - 2] << 8)
            + (data[index - 3] << 16)
            + (data[index - 4] << 24);
        }
        
    }
}
