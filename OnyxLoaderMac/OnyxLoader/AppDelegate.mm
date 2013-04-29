//
//  AppDelegate.m
//  OnyxLoader
//
//  Created by new on 05/11/2012.
//  Copyright (c) 2012 SGenomics Ltd. All rights reserved.
//

#import "AppDelegate.h"
#include "../doflash.h"

#import <IOKit/kext/KextManager.h>

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  NSString *ftdiKextKey = @"com.FTDI.driver.FTDIUSBSerialDriver";
    // Insert code here to initialize your application
  NSDictionary *kexts = (__bridge NSDictionary *)KextManagerCopyLoadedKextInfo((__bridge CFArrayRef)[NSArray arrayWithObject: ftdiKextKey], NULL);
  if ([kexts objectForKey: ftdiKextKey] != nil) {
    // unload the kext
    
#if 0
    AuthorizationRef myAuthorizationRef;
    OSStatus myStatus;
    myStatus = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
                                   kAuthorizationFlagDefaults, &myAuthorizationRef);
    AuthorizationItem myItems[1];
    
    myItems[0].name = "com.myOrganization.myProduct.myRight1";
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
    // do stuff
    // need to do SMJobBless
    
    myStatus = AuthorizationFree (myAuthorizationRef,
                                  kAuthorizationFlagDestroyRights);
#else
    NSRunAlertPanel(@"Help: FTDI Virtual Serial Driver is loaded",
                    @"The FTDI virtual serial port kext seems to be loaded so things will likely fail.  You can temporarily fix this by doing 'sudo kextunload -b com.FTDI.driver.FTDIUSBSerialDriver' and then 'sudo kextutil -b com.FTDI.driver.FTDIUSBSerialDriver' when you want to use the virtual serial driver again.", @"OK", nil, nil);
#endif
    
    
  }
}


- (void)applicationWillTerminate:(NSNotification *)notification
{
  
}



- (void)startSpinnerDisableControls {
  [self.loadingSpinner startAnimation: self];
  [self.saveDataButton setEnabled: NO];
  [self.setTimeButton setEnabled: NO];
  [self.updateLatestButton setEnabled: NO];
  [self.updateBetaButton setEnabled: NO];
}

- (void)stopSpinnerEnableControls {
  [self.loadingSpinner stopAnimation: self];
  [self.saveDataButton setEnabled: YES];
  [self.setTimeButton setEnabled: YES];
  [self.updateLatestButton setEnabled: YES];
  [self.updateBetaButton setEnabled: YES];
  self.statusText.stringValue = @"";
}

- (void)runErrorAlertWithMessage: (NSString *)message {
  NSRunAlertPanel(@"OnyxLoader: Something went wrong", message, @"OK", nil, nil);
}

- (void)backgroundSaveCSV: (NSURL*)url {
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  char *data = do_get_log_csv();
  
  if (data == NULL) {
    [self performSelectorOnMainThread: @selector(runErrorAlertWithMessage:) withObject: @"Couldn't talk to the Onyx" waitUntilDone: YES];
  } else {
    NSData *d = [NSData dataWithBytesNoCopy: data length: strlen(data) freeWhenDone: YES];
    [d writeToURL: url atomically: YES];
  }
  
  [self performSelectorOnMainThread: @selector(stopSpinnerEnableControls) withObject: nil waitUntilDone: NO];
}

- (IBAction)SaveCSV:(id)sender {
    NSLog(@"SaveCSV");
    self.statusText.stringValue = @"Saving Log";
  
    NSSavePanel * savePanel = [NSSavePanel savePanel];

    [savePanel setAllowedFileTypes:[NSArray arrayWithObject:@"csv"]];

    [savePanel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSLog(@"Got URL: %@", [savePanel URL]);
          
          [self performSelectorInBackground: @selector(backgroundSaveCSV:) withObject: [savePanel URL]];
        } else
          self.statusText.stringValue = @"";
    }];

}

- (void)backgroundSetTime {
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  if (do_set_time() == 0) {
    [self performSelectorOnMainThread: @selector(runErrorAlertWithMessage:) withObject: @"Couldn't talk to the Onyx" waitUntilDone: YES];
  }
  
  [self performSelectorOnMainThread: @selector(stopSpinnerEnableControls) withObject: nil waitUntilDone: NO];  
}

- (IBAction)SetTime:(id)sender {
  NSLog(@"Set Time");
  self.statusText.stringValue = @"Setting Time";
  [self performSelectorInBackground: @selector(backgroundSetTime) withObject: nil];
}

- (void)runFirmwareAlertWithResult: (NSString *)result {
  if(result == nil) {
    NSRunAlertPanel(@"OnyxLoader: Programming complete",
                    @"Please disconnect the device.",
                    @"OK", nil, nil);
  } else {
    NSRunAlertPanel(@"OnyxLoader: Programming failed",
                    result,
                    @"OK", nil, nil);
  }
}

- (NSString *)downloadFromUrl: (NSURL *)url toBaseName: (NSString *)base {
  // Determine cache file path
  // This could use NSTemporaryDirectory() instead.
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
  NSString *filePath = [NSString stringWithFormat:@"%@/%@", [paths objectAtIndex:0], base];
  
  // Download and write to file
  // This shouldn't be reading into one NSData object like this (unbounded size).  Should use NSURLConnection instead.
  NSData *urlData = [NSData dataWithContentsOfURL:url];
  if (urlData == nil)
    return nil;
  
  [urlData writeToFile:filePath atomically:YES];
  
  return filePath;
}

- (int)writeFileToFlash: (NSString *)filePath {
  int argc = 3;
  char av0[20], av1[20], av2[MAXPATHLEN];
  
  char *argv[3];
  strlcpy(av0, "flash", sizeof(av0));
  char *argv0 = av0;
  strlcpy(av1, "-f", sizeof(av1));
  char *argv1 = av1;
  strlcpy(av2, filePath.UTF8String, sizeof(av2));
  char *argv2 = av2;
  argv[0] = argv0;
  argv[1] = argv1;
  argv[2] = argv2;
  
  return do_flash_main(argc,argv);
}


- (void)backgroundUpdateExperimental {
  NSString *resultString = nil;
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  // Download flash image from http://41j.com/safecast_exp.bin
  NSString *filePath = [self downloadFromUrl: [NSURL URLWithString:@"http://41j.com/safecast_exp.bin"]
                                  toBaseName: @"firmwareB"];
  
  if (filePath != nil)
    NSLog(@"Downloaded experimental firmware successfully");
  else
    NSLog(@"Experimental firmware download failed");
  
    
  if (filePath != nil) {
    int result = [self writeFileToFlash: filePath];
    if (result != 0)
      resultString = [NSString stringWithFormat: @"Failure Code: %d", result];
  } else {
    resultString = @"Failed to download firmware";
  }
  
  [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];
  [self performSelectorOnMainThread: @selector(runFirmwareAlertWithResult:) withObject: resultString waitUntilDone: YES];
  
  [self performSelectorOnMainThread: @selector(stopSpinnerEnableControls) withObject: nil waitUntilDone: NO];
}


- (IBAction)UpdateExperimental:(id)sender {
    NSLog(@"Update Firmware - experimental");
    
    NSInteger res = NSRunAlertPanel(@"Programming Beta Firmware",
    @"Warning: This firmware is for testing only, it is unsupported by Medcom International",
    @"Continue", @"Cancel", nil);
    
    switch(res) {
      case NSAlertDefaultReturn:
      break;
      case NSAlertAlternateReturn:
      return;
      break;
      case NSAlertOtherReturn:
      return;
      break;
    }
  self.statusText.stringValue = @"Beta Firmware";
  [self performSelectorInBackground: @selector(backgroundUpdateExperimental) withObject: nil];
}

- (void)backgroundUpdateFirmware {
  NSString *resultString = nil;
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  // Download flash image from http://41j.com/safecast_latest.bin
  NSString *filePath = [self downloadFromUrl: [NSURL URLWithString:@"http://41j.com/safecast_latest.bin"]
                                  toBaseName: @"firmwareL"];
  
  if (filePath != nil)
    NSLog(@"Downloaded latest firmware successfully");
  else
    NSLog(@"Latest firmware download failed");

  if (filePath != nil) {
    int result = [self writeFileToFlash: filePath];
    if (result != 0)
      resultString = [NSString stringWithFormat: @"Failure Code: %d", result];
  } else {
    resultString = @"Failed to download firmware";
  }

  [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];

  [self performSelectorOnMainThread: @selector(runFirmwareAlertWithResult:) withObject: resultString waitUntilDone: YES];
  
  [self performSelectorOnMainThread: @selector(stopSpinnerEnableControls) withObject: nil waitUntilDone: NO];
}

- (IBAction)UpdateFirmware:(NSButton *)sender {
  NSLog(@"Update Firmware - stable");
  self.statusText.stringValue = @"Latest Firmware";
  [self performSelectorInBackground: @selector(backgroundUpdateFirmware) withObject: nil];
}


// The button for this is currently disabled.
// This should be similarly backgrounded if this button is enabled in
// a future release.
- (IBAction)SendLog:(NSButton *)sender {
    NSLog(@"Sending Log");
    
    char *data = do_get_log();
    
    NSString * str = [NSString stringWithFormat:@"%s", data];
    free(data);
    
    NSData* myFileNSData = [str dataUsingEncoding:NSUTF8StringEncoding];
    
    NSMutableURLRequest* post = [NSMutableURLRequest requestWithURL: [NSURL URLWithString:@"http://41j.com/sc/sc.php"]];
    
    [post setHTTPMethod: @"POST"];
    
    NSString *boundary = @"0xKhTmLbOuNdArY";
    NSString *contentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@",boundary];
    [post addValue:contentType forHTTPHeaderField: @"Content-Type"];
    
    NSMutableData *body = [NSMutableData data];
    
    [body appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
    [body appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"uploadedfile\"; filename=\"sc1\"\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
    [body appendData:[@"Content-Type: application/octet-stream\r\n\r\n" dataUsingEncoding:NSUTF8StringEncoding]];
    [body appendData:myFileNSData] ;
    [body appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
    
    [post setHTTPBody:body];
    
    NSURLResponse* response;
    
    NSError* error;
    
    NSData* result = [NSURLConnection sendSynchronousRequest:post returningResponse:&response error:&error];
    
    NSLog(@"%@", [[NSString alloc] initWithData:result encoding:NSASCIIStringEncoding ]);
}

- (void) dealloc {
  self.loadingSpinner = nil;
  self.saveDataButton = nil;
  self.setTimeButton = nil;
  self.updateLatestButton = nil;
  self.updateBetaButton = nil;
  self.statusText = nil;
}

@end
