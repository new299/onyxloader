//
//  AppDelegate.h
//  OnyxLoader
//
//  Created by new on 05/11/2012.
//  Copyright (c) 2012 SGenomics Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "FTDIKextLoader.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *window;
@property (nonatomic, retain) IBOutlet NSProgressIndicator *loadingSpinner;
@property (nonatomic, retain) IBOutlet NSButton *saveDataButton;
@property (nonatomic, retain) IBOutlet NSButton *setTimeButton;
@property (nonatomic, retain) IBOutlet NSButton *updateLatestButton;
@property (nonatomic, retain) IBOutlet NSButton *updateBetaButton;
@property (nonatomic, retain) IBOutlet NSTextField *statusText;

@property (nonatomic, retain) FTDIKextLoader *kextLoader;

@end
