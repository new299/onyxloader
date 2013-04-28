//
//  AppDelegate.m
//  OnyxLoader
//
//  Created by new on 05/11/2012.
//  Copyright (c) 2012 SGenomics Ltd. All rights reserved.
//

#import "AppDelegate.h"
#include "../doflash.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
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

- (void)backgroundSaveCSV: (NSURL*)url {
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  char *data = do_get_log_csv();
  
  NSString * str = [NSString stringWithFormat:@"%s", data];
  free(data);
  
  [str writeToURL: url atomically:YES encoding:NSUTF8StringEncoding error:NULL];
  
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
        }
    }];

}

- (void)backgroundSetTime {
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  do_set_time();
  
  [self performSelectorOnMainThread: @selector(stopSpinnerEnableControls) withObject: nil waitUntilDone: NO];  
}

- (IBAction)SetTime:(id)sender {
  NSLog(@"Set Time");
  self.statusText.stringValue = @"Setting Time";
  [self performSelectorInBackground: @selector(backgroundSetTime) withObject: nil];
}

- (void)runFirmwareAlertWithResult: (NSNumber*)result {
  if([result integerValue] == 0) {
    NSRunAlertPanel(@"Programming complete",
                    @"Please disconnect the device.",
                    @"OK", NULL, NULL);
  } else {
    NSRunAlertPanel(@"Programming failed",
                    [NSString stringWithFormat: @"Failure Code: %ld", (long)[result integerValue]],
                    @"OK", NULL, NULL);
  }
}


-(void)backgroundUpdateExperimental {
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  // Download flash image from http://41j.com/safecast_exp.bin
  // Determile cache file path
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
  NSString *filePath = [NSString stringWithFormat:@"%@/%@", [paths objectAtIndex:0],@"firmwareB"];
  
  // Download and write to file
  NSURL *url = [NSURL URLWithString:@"http://41j.com/safecast_exp.bin"];
  // This shouldn't be reading into one NSData object like this (unbounded size).  Should use NSURLConnection instead.
  NSData *urlData = [NSData dataWithContentsOfURL:url];
  [urlData writeToFile:filePath atomically:YES];
  
  int argc=3;
  char *argv[3];
  char *argv0 = "flash";
  char *argv1 = "-f";
  char *argv2 = (char *) filePath.UTF8String;
  argv[0] = argv0;
  argv[1] = argv1;
  argv[2] = argv2;
  
  int result = do_flash_main(argc,argv);
  
  [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];
  [self performSelectorOnMainThread: @selector(runFirmwareAlertWithResult:) withObject: [NSNumber numberWithInt: result] waitUntilDone: YES];
  
  [self performSelectorOnMainThread: @selector(stopSpinnerEnableControls) withObject: nil waitUntilDone: NO];
}


- (IBAction)UpdateExperimental:(id)sender {
    NSLog(@"Update Firmware - experimental");
    
    NSInteger res = NSRunAlertPanel(@"Programming Beta Firmware",
    @"Warning: This firmware is for testing only, it is unsupported by Medcom International",
    @"Continue", @"Cancel", NULL);
    
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
  [self performSelectorOnMainThread: @selector(startSpinnerDisableControls) withObject: nil waitUntilDone: YES];
  
  // Download flash image from http://41j.com/safecast_latest.bin
  // Determine cache file path
  NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
  NSString *filePath = [NSString stringWithFormat:@"%@/%@", [paths objectAtIndex:0],@"firmwareL"];
  
  // Download and write to file
  NSURL *url = [NSURL URLWithString:@"http://41j.com/safecast_latest.bin"];
  // This shouldn't be reading into one NSData object like this (unbounded size).  Should use NSURLConnection instead.
  NSData *urlData = [NSData dataWithContentsOfURL:url];
  [urlData writeToFile:filePath atomically:YES];
  int argc=3;
  char *argv[3];
  char *argv0 = "flash";
  char *argv1 = "-f";
  char *argv2 = (char *) filePath.UTF8String;
  argv[0] = argv0;
  argv[1] = argv1;
  argv[2] = argv2;
  
  int result = do_flash_main(argc,argv);

  [[NSFileManager defaultManager] removeItemAtPath:filePath error:nil];

  [self performSelectorOnMainThread: @selector(runFirmwareAlertWithResult:) withObject: [NSNumber numberWithInt: result] waitUntilDone: YES];
  
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
    
    NSString *boundary = [NSString stringWithString:@"0xKhTmLbOuNdArY"];
    NSString *contentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@",boundary];
    [post addValue:contentType forHTTPHeaderField: @"Content-Type"];
    
    NSMutableData *body = [NSMutableData data];
    
    [body appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
    [body appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"uploadedfile\"; filename=\"sc1\"\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
    [body appendData:[[NSString stringWithString:@"Content-Type: application/octet-stream\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
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
