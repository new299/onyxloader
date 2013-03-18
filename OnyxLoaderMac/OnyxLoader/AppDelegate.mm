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

- (IBAction)SaveCSV:(id)sender {

    NSLog(@"SaveCSV");
    NSSavePanel * savePanel = [NSSavePanel savePanel];

    [savePanel setAllowedFileTypes:[NSArray arrayWithObject:@"csv"]];

    [savePanel beginWithCompletionHandler:^(NSInteger result){
        if (result == NSFileHandlingPanelOKButton) {
            NSLog(@"Got URL: %@", [savePanel URL]);
            
            char *data = do_get_log_csv();
    
            NSString * str = [NSString stringWithFormat:@"%s", data];
            free(data);
            
            [str writeToURL:[savePanel URL] atomically:YES encoding:NSUTF8StringEncoding error:NULL];
        }
    }];

}

- (IBAction)SetTime:(id)sender {
  NSLog(@"Set Time");
}


- (IBAction)UpdateExperimental:(id)sender {
    NSLog(@"Update Firmware - experimental");
    
    // Download flash image from http://41j.com/safecast_exp.bin
    // Determile cache file path
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *filePath = [NSString stringWithFormat:@"%@/%@", [paths objectAtIndex:0],@"index.html"];
    
    // Download and write to file
    NSURL *url = [NSURL URLWithString:@"http://41j.com/safecast_exp.bin"];
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
}

- (IBAction)UpdateFirmware:(NSButton *)sender {
    NSLog(@"Update Firmware - stable");
    
    // Download flash image from http://41j.com/safecast_latest.bin
    // Determile cache file path
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *filePath = [NSString stringWithFormat:@"%@/%@", [paths objectAtIndex:0],@"index.html"];
    
    // Download and write to file
    NSURL *url = [NSURL URLWithString:@"http://41j.com/safecast_latest.bin"];
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
}

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

@end
