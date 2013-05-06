//
//  FTDIKextLoader.h
//  OnyxLoader
//
//  Created by Kristen A. McIntyre on 5/5/13.
//  Copyright (c) 2013 SGenomics Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface FTDIKextLoader : NSObject {
  xpc_object_t helper_connection;
}

@property (nonatomic, retain) NSString *helperLabel;
@property (nonatomic, retain) NSString *ftdiKextLabel;

- (BOOL) installHelperIfNeeded;
- (BOOL) isFtdiKextLoaded;
- (BOOL) sendKextLoadToHelper;
- (BOOL) sendKextUnloadToHelper;

@end
