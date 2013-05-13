//
//  main.c
//  uk.co.sgenomics.OnyxLoaderHelper
//
//  Created by Kristen A. McIntyre on 5/4/13.
//  Copyright (c) 2013 SGenomics Ltd. All rights reserved.
//

#include <stdio.h>
#include <syslog.h>
#include <xpc/xpc.h>
#import <IOKit/kext/KextManager.h>

static void __XPC_Peer_Event_Handler(xpc_connection_t connection, xpc_object_t event) {
  //syslog(LOG_NOTICE, "Received event in helper.");
  
	xpc_type_t type = xpc_get_type(event);
  
	if (type == XPC_TYPE_ERROR) {
		if (event == XPC_ERROR_CONNECTION_INVALID) {
			// The client process on the other end of the connection has either
			// crashed or cancelled the connection. After receiving this error,
			// the connection is in an invalid state, and you do not need to
			// call xpc_connection_cancel(). Just tear down any associated state
			// here.
      
		} else if (event == XPC_ERROR_TERMINATION_IMMINENT) {
			// Handle per-connection termination cleanup.
		}
    
	} else {
    xpc_connection_t remote = xpc_dictionary_get_remote_connection(event);
    
    const char *request = xpc_dictionary_get_string(event, "request");
    const char *replyString = "OK";
    int64_t errorCode = 0;
    int shutdownRequest = 0;
   
    if (request != NULL) {
      if (strcmp(request, "loadkext") == 0) {
        syslog(LOG_NOTICE, "got the loadkext request");
        CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, NULL, 0, NULL);
        OSReturn ret = KextManagerLoadKextWithIdentifier(CFSTR("com.FTDI.driver.FTDIUSBSerialDriver"), array);
        CFRelease(array);
        if (ret != kOSReturnSuccess) {
          replyString = "Load Error";
          errorCode = ret;
        }
      } else if (strcmp(request, "unloadkext") == 0) {
        syslog(LOG_NOTICE, "got the unloadkext request");
        OSReturn ret = KextManagerUnloadKextWithIdentifier(CFSTR("com.FTDI.driver.FTDIUSBSerialDriver"));
        if (ret != kOSReturnSuccess) {
          replyString = "Load Error";
          errorCode = ret;
        }
      } else if (strcmp(request, "shutdown") == 0) {
        syslog(LOG_NOTICE, "got the shutdown request");
        shutdownRequest = 1;
      } else
        syslog(LOG_NOTICE, "got an unknown request");
    }

    xpc_object_t reply = xpc_dictionary_create_reply(event);
    xpc_dictionary_set_string(reply, "reply", "OK");
    xpc_dictionary_set_int64(reply, "errorCode", errorCode);
    xpc_connection_send_message(remote, reply);
    xpc_release(reply);
    if (shutdownRequest) {
      // shut down after a short delay (5 seconds)
      dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        exit(EXIT_SUCCESS);
      });
    }
	}
}

static void __XPC_Connection_Handler(xpc_connection_t connection)  {
  syslog(LOG_NOTICE, "Configuring message event handler for helper");
  
	xpc_connection_set_event_handler(connection, ^(xpc_object_t event) {
		__XPC_Peer_Event_Handler(connection, event);
	});
	
	xpc_connection_resume(connection);
}

int main(int argc, const char *argv[]) {
  xpc_connection_t service = xpc_connection_create_mach_service("uk.co.sgenomics.OnyxLoaderHelper",
                                                                dispatch_get_main_queue(),
                                                                XPC_CONNECTION_MACH_SERVICE_LISTENER);
  
  if (!service) {
    syslog(LOG_NOTICE, "Failed to create service.");
    exit(EXIT_FAILURE);
  }
  
  CFBundleRef bundle = CFBundleGetMainBundle();
  
  if (bundle != NULL) {
    CFStringRef vers = CFBundleGetValueForInfoDictionaryKey(bundle, kCFBundleVersionKey);
    if (vers != NULL) {
      char b[256];
      
      b[0] = '\0';
      CFStringGetCString(vers, b, sizeof(b), kCFStringEncodingUTF8);
      syslog(LOG_NOTICE, "Version %s", b);
    } else
      syslog(LOG_NOTICE, "Couldn't determine the version - no version value");
  } else
    syslog(LOG_NOTICE, "Couldn't determine the version - no bundle");
    
  
  syslog(LOG_NOTICE, "Configuring connection event handler for helper");
  xpc_connection_set_event_handler(service, ^(xpc_object_t connection) {
    __XPC_Connection_Handler(connection);
  });
  
  xpc_connection_resume(service);
  
  dispatch_main();
  
  xpc_release(service);
  
  return EXIT_SUCCESS;
}


