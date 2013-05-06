//
//  FTDIKextLoader.m
//  OnyxLoader
//
//  Created by Kristen A. McIntyre on 5/5/13.
//  Copyright (c) 2013 SGenomics Ltd. All rights reserved.
//

#import "FTDIKextLoader.h"

#import <IOKit/kext/KextManager.h>
#import <ServiceManagement/ServiceManagement.h>

@implementation FTDIKextLoader

- (id) init {
  if ((self = [super init]) != nil) {
    self.helperLabel = @"uk.co.sgenomics.OnyxLoaderHelper";
    self.ftdiKextLabel = @"com.FTDI.driver.FTDIUSBSerialDriver";
  }
  return self;
}

- (AuthorizationRef) authenticateForInstallingHelper {
  AuthorizationRef myAuthorizationRef;
  OSStatus myStatus;
  myStatus = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
                                 kAuthorizationFlagDefaults, &myAuthorizationRef);
  AuthorizationItem myItems[1];
  
  myItems[0].name = "uk.co.sgenomics.OnyxLoader.fixFTDIKext";
  myItems[0].valueLength = 0;
  myItems[0].value = NULL;
  myItems[0].flags = 0;
  
  AuthorizationRights myRights;
  myRights.count = sizeof (myItems) / sizeof (myItems[0]);
  myRights.items = myItems;
  
  AuthorizationFlags myFlags;
  myFlags = kAuthorizationFlagDefaults |
  kAuthorizationFlagInteractionAllowed |
  kAuthorizationFlagExtendRights;
  
  myStatus = AuthorizationCopyRights (myAuthorizationRef, &myRights,
                                      kAuthorizationEmptyEnvironment, myFlags, NULL);
  return myAuthorizationRef;
}

- (void) destroyAuthRef: (AuthorizationRef)authRef {
  AuthorizationFree (authRef,
                     kAuthorizationFlagDestroyRights);

}

- (BOOL) aHelperExists {
  NSDictionary *currentHelper = CFBridgingRelease(SMJobCopyDictionary(kSMDomainSystemLaunchd, (__bridge CFStringRef)self.helperLabel));
  
  return currentHelper != nil;
}

- (BOOL) installHelperApp {
  AuthorizationRef authRef = [self authenticateForInstallingHelper];
  if (authRef == NULL)
    return NO;
  
  CFErrorRef error = NULL;

  if ([self aHelperExists]) {
    SMJobRemove(kSMDomainSystemLaunchd, (__bridge CFStringRef)self.helperLabel, authRef, YES, &error);
    if (error != NULL)
      NSLog(@"SMJobRemove failed with %@", [CFBridgingRelease(error) description]);
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 5.0, YES);
    //[self destroyAuthRef: authRef];
    //authRef = [self authenticateForInstallingHelper];
  }
  
  if (SMJobBless(kSMDomainSystemLaunchd, (__bridge CFStringRef)self.helperLabel, authRef, &error)) {
    [self destroyAuthRef: authRef];
    return YES;
  }
  [self destroyAuthRef: authRef];
  if (error != NULL) {
    NSLog(@"SMJobBless failed with %@", [CFBridgingRelease(error) description]);
    return NO;
  }
  NSLog(@"SMJobBless didn't do anything");
  return NO;
}

- (BOOL) doesHelperAppNeedToBeInstalled {
  NSDictionary *currentHelper = CFBridgingRelease(SMJobCopyDictionary(kSMDomainSystemLaunchd, (__bridge CFStringRef)self.helperLabel));
  
  if (currentHelper == nil)
    return YES;
  
  NSString *path = [[currentHelper objectForKey: @"ProgramArguments"] objectAtIndex: 0];
  NSDictionary *infoDictForLS = CFBridgingRelease(CFBundleCopyInfoDictionaryForURL((__bridge CFURLRef)[NSURL fileURLWithPath: path]));
  
  NSString *bundleComponent = [NSString stringWithFormat: @"Contents/Library/LaunchServices/%@", self.helperLabel];
  NSURL *localHelperURL = [[[NSBundle mainBundle] bundleURL] URLByAppendingPathComponent: bundleComponent];
  NSDictionary *infoDictLocal = CFBridgingRelease(CFBundleCopyInfoDictionaryForURL((__bridge CFURLRef)localHelperURL));
  return ![[infoDictLocal objectForKey: @"CFBundleVersion"] isEqual: [infoDictForLS objectForKey: @"CFBundleVersion"]];
}

- (BOOL) isFtdiKextLoaded {
  NSDictionary *kexts = (__bridge NSDictionary *)KextManagerCopyLoadedKextInfo((__bridge CFArrayRef)[NSArray arrayWithObject: self.ftdiKextLabel], NULL);
  return [kexts objectForKey: self.ftdiKextLabel] != nil;

}

- (BOOL) establishXPCConnectionToHelper {
  if (helper_connection != NULL)
    xpc_release(helper_connection);
  helper_connection = xpc_connection_create_mach_service([self.helperLabel UTF8String],
                                                         NULL,
                                                         XPC_CONNECTION_MACH_SERVICE_PRIVILEGED);
  
  if (helper_connection == NULL) {
    NSLog(@"XPC connection to helper failed.");
    return false;
  }
  
  xpc_connection_set_event_handler(helper_connection, ^(xpc_object_t event) {
    xpc_type_t type = xpc_get_type(event);
    
    if (type == XPC_TYPE_ERROR) {
      
      if (event == XPC_ERROR_CONNECTION_INTERRUPTED) {
        NSLog(@"XPC helper connection interupted.");
      } else if (event == XPC_ERROR_CONNECTION_INVALID) {
        NSLog(@"XPC helper connection invalid.");
      } else {
        NSLog(@"Unexpected XPC connection error with helper.");
      }
      
    } else {
      NSLog(@"Unexpected XPC connection event with helper.");
    }
  });
  
  xpc_connection_resume(helper_connection);
  return true;
}

- (void) tearDownXPCConnectionToHelper {
  if (helper_connection != NULL) {
    xpc_connection_cancel(helper_connection);
    xpc_release(helper_connection);
    helper_connection = NULL;
  }
}

- (BOOL) isMainThread {
  return CFRunLoopGetCurrent() == CFRunLoopGetMain();
}

- (NSString *) sendMessageToHelper: (NSString *)messageString errorCode: (int *)ecode {
  xpc_object_t message = xpc_dictionary_create(NULL, NULL, 0);
  const char* request = [messageString UTF8String];
  xpc_dictionary_set_string(message, "request", request);
  
  __block NSString *replyString = nil;
  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
  dispatch_retain(semaphore);
  xpc_connection_send_message_with_reply(helper_connection, message, dispatch_get_main_queue(), ^(xpc_object_t event) {
    const char* response = xpc_dictionary_get_string(event, "reply");
    printf("helper responded with: %s\n", response);
    *ecode = (int)xpc_dictionary_get_int64(event, "errorCode");
    if (response != NULL)
      replyString = [NSString stringWithUTF8String: response];
    dispatch_semaphore_signal(semaphore);
    dispatch_release(semaphore);
  });
  // give things roughly 5 seconds to complete
  if ([self isMainThread]) {
    int maxLoops = 5;
    while (dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 100 * NSEC_PER_MSEC)) != 0 && maxLoops--) {
      CFRunLoopRunInMode(kCFRunLoopDefaultMode, 1.0, YES);
    }
  }
  else
  {
    dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
  }
  dispatch_release(semaphore);
  return replyString;
}

- (BOOL) installHelperIfNeeded {
  if ([self doesHelperAppNeedToBeInstalled])
    return [self installHelperApp];
  return YES;
}

- (BOOL) setUpXPCOnDemand {
  if (helper_connection == NULL) {
    return [self establishXPCConnectionToHelper];
  }
  return YES;
}

- (BOOL) sendKextUnloadToHelper {
  if (![self setUpXPCOnDemand])
    return NO;
  int code;
  [self sendMessageToHelper: @"unloadkext" errorCode: &code];
  return YES;
}

- (BOOL) sendKextLoadToHelper {
  if (![self setUpXPCOnDemand])
    return NO;
  int code;
  [self sendMessageToHelper: @"loadkext" errorCode: &code];  
  return YES;
}

@end
