//
//  WZMediaPlayer.h
//  WZMediaPlayer
//
//  Created by fernando on 16/12/2.
//  Copyright © 2016年 Fernando. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface WZMediaPlayer :

+ (WZMediaPlayer*)setupPlayer:(NSString*)url ContainerView:(UIView *)container;
- (void)start;
- (void)pause;
- (void)resume;
- (void)stop;

@end
