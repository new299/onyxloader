/*
	Simple example to open a maximum of 4 devices - write some data then read it back.
	Shows one method of using list devices also.
	Assumes the devices have a loopback connector on them and they also have a serial number

	To build use the following gcc statement 
	(assuming you have the d2xx library in the /usr/local/lib directory).
	gcc -o simple main.c -L. -lftd2xx -Wl,-rpath /usr/local/lib
*/
#pragma once

//#include <WinBase.h>
#include "stm32ld/stm32ld.h"
#include <ctype.h>
#include <stdlib.h>
//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32ld/serial.h"

#include <errno.h>
#include <limits.h>

#include "ftd2xx.h"

#define BL_VERSION_MAJOR  2
#define BL_VERSION_MINOR  1
#define BL_MKVER( major, minor )    ( ( major ) * 256 + ( minor ) ) 
#define BL_MINVERSION               BL_MKVER( BL_VERSION_MAJOR, BL_VERSION_MINOR )

#define CHIP_ID           0x0414

#define BUF_SIZE 0x10

FT_HANDLE	ftHandle[MAX_DEVICES];
char 	        cBufLD[MAX_DEVICES][64];
int             foundDevices = 0;
//extern int32_t stm32_ser_id;
int ser_dbg = 0;

static  FILE *fp_page1;
static  FILE *fp_page4;
static  u32 fpsize;

int fw_abort();

#define CBUS_RESET_MASK   0x1
#define CBUS_BOOT_MASK    0x4

int getandcheckCBUS( FT_HANDLE ftHandle0 ) {
  FT_PROGRAM_DATA Data;
  int need_write = 0;
  FT_STATUS	ftStatus;

  if( ser_dbg )
    printf("ftHandle0 = %p\n", ftHandle0);

  Data.Signature1 = 0x00000000;
  Data.Signature2 = 0xffffffff;
  Data.Manufacturer = (char *)malloc(256);		// "FTDI"
  Data.ManufacturerId = (char *)malloc(256);	// "FT"
  Data.Description = (char *)malloc(256);			// "USB HS Serial Converter"
  Data.SerialNumber = (char *)malloc(256);		// "FT000001" if fixed, or NULL
  ftStatus = FT_EE_Read(ftHandle0, &Data);
  if(ftStatus != FT_OK) {
    printf("FT_EE_Read failed\n");
    FT_Close(ftHandle0);
    return 1;
  }

  if( ser_dbg ) {
//    printf("Cbus0 = 0x%X\n", Data.Cbus0);				// Cbus Mux control
 //   printf("Cbus1 = 0x%X\n", Data.Cbus1);				// Cbus Mux control
  //  printf("Cbus2 = 0x%X\n", Data.Cbus2);				// Cbus Mux control
  //  printf("Cbus3 = 0x%X\n", Data.Cbus3);				// Cbus Mux control
  //  printf("Cbus4 = 0x%X\n", Data.Cbus4);				// Cbus Mux control
  }

  // check that cbus0 and 2 are write
//  if( Data.Cbus0 != 0x0A ) {
//    printf( "Cbus0 is %d, should be %d, updating!\n", Data.Cbus0, 0xA );
//    Data.Cbus0 = 0x0A;
//    need_write = 1;
//  }
  
//  if( Data.Cbus2 != 0x0A ) {
//    printf( "Cbus2 is %d, should be %d, updating!\n", Data.Cbus2, 0xA );
//    Data.Cbus2 = 0x0A;
//    need_write = 1;
//  }

  // check that CBUS3 is power enable
//  if( Data.Cbus3 != 0x01 ) {
//    printf( "Cbus3 is %d, should be %d, updating!\n", Data.Cbus3, 0x1);
//    Data.Cbus2 = 0x0B;
//    need_write = 1;
//  }

//  // not necessary, but for the hell of it, cbus 1 is read
//  if( Data.Cbus1 != 0x0A ) {
 //   printf( "Cbus1 is %d, should be %d, updating!\n", Data.Cbus1, 0xA );
  //  Data.Cbus1 = 0x0A;
 //   need_write = 1;
 // }

  if( need_write ) {
    printf( "Updating EEPROM to correct setting for safecast.\n" );
    ftStatus = FT_EE_Program(ftHandle0, &Data);
    if(ftStatus != FT_OK) {
      printf("FT_EE_Program failed (%d)\n", ftStatus);
      FT_Close(ftHandle0);
      return 1;
    }
    printf( "------> Now that the EEPROM is updated, unplug and replug the device.\n" );
  } else {
    printf( "EEPROM values are up to date, not modifying them\n" );
  }
  return 0;

}


ser_handler openSerialPorts8N1(int baud) {
    char * 	pcBufRead = NULL;
    char * 	pcBufLD[MAX_DEVICES + 1];
    DWORD	dwRxSize = 0;
    DWORD 	dwBytesWritten, dwBytesRead;
    FT_STATUS	ftStatus;
    int	iNumDevs = 0;
    int	i, j;
    int	iDevicesOpen;
    unsigned char ucMode = 0x00;
    
    printf( "warning: opening up to %d ports and assuming all are Safecast devices.\n", MAX_DEVICES );
    printf( "todo: make this more selective and safer.\n" );
    
    for(i = 0; i < MAX_DEVICES; i++) {
        pcBufLD[i] = cBufLD[i];
    }
    pcBufLD[MAX_DEVICES] = NULL;
    
    ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
    
    if(ftStatus != FT_OK) {
        fprintf(stderr,"Error: FT_ListDevices(%d)\n", ftStatus);
        return (ser_handler)-1;
    }
    
    for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
        printf("Device %d Serial Number - %s\n", i, cBufLD[i]);
    }
    
    for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ) ; i++) {
        /* Setup */
        if((ftStatus = FT_OpenEx(cBufLD[i], FT_OPEN_BY_SERIAL_NUMBER, &ftHandle[i])) != FT_OK){
            /*
             This can fail if the ftdi_sio driver is loaded
             use lsmod to check this and rmmod ftdi_sio to remove
             also rmmod usbserial
             */
            fprintf(stderr,"Error FT_OpenEx(%d), device\n", ftStatus, i);
            return (ser_handler)-1;
        }
        
        printf("Opened device %s\n", cBufLD[i]);
        
        //if(getandcheckCBUS(ftHandle[i]) ) {
        //    printf( "getandcheckCBUS failed, exiting.\n" );
        //    return -1;
        //}
        
        iDevicesOpen++;
        if((ftStatus = FT_SetBaudRate(ftHandle[i], baud)) != FT_OK) {
            fprintf(stderr,"Error FT_SetBaudRate(%d), cBufLD[i] = %s\n", ftStatus, cBufLD[i]);
            break;
        }
        
        if((ftStatus = FT_SetDataCharacteristics(ftHandle[i], FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE)) != FT_OK) {
            fprintf(stderr,"Error FT_SetDataCharacteristics(%d) = %s\n", ftStatus, cBufLD[i]);
            break;
        }
        
        FT_SetTimeouts(ftHandle[i], 5000, 5000);	// 500 ms read/write timeout
        
    }
    
    iDevicesOpen = i;
    foundDevices = i; // record this in a global
    
    if(pcBufRead)
        free(pcBufRead);
    return 0;
   // return (ser_handler) ftHandle[0]; // we always use the 0th device for now
}

ser_handler openSerialPorts(int baud) {
  char * 	pcBufRead = NULL;
  char * 	pcBufLD[MAX_DEVICES + 1];
  DWORD	dwRxSize = 0;
  DWORD 	dwBytesWritten, dwBytesRead;
  FT_STATUS	ftStatus;
  int	iNumDevs = 0;
  int	i, j;
  int	iDevicesOpen;	
  unsigned char ucMode = 0x00;

  printf( "warning: opening up to %d ports and assuming all are Safecast devices.\n", MAX_DEVICES );
  printf( "todo: make this more selective and safer.\n" );

  for(i = 0; i < MAX_DEVICES; i++) {
    pcBufLD[i] = cBufLD[i];
  }
  pcBufLD[MAX_DEVICES] = NULL;
  
  ftStatus = FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_SERIAL_NUMBER);
  
  if(ftStatus != FT_OK) {
    fprintf(stderr,"Error: FT_ListDevices(%d)\n", ftStatus);
    return (ser_handler)-1;
  }
  
  for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
    printf("Device %d Serial Number - %s\n", i, cBufLD[i]);
  }
  
  for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ) ; i++) {
    /* Setup */
    if((ftStatus = FT_OpenEx(cBufLD[i], FT_OPEN_BY_SERIAL_NUMBER, &ftHandle[i])) != FT_OK){
      /* 
	 This can fail if the ftdi_sio driver is loaded
	 use lsmod to check this and rmmod ftdi_sio to remove
	 also rmmod usbserial
      */
      fprintf(stderr,"Error FT_OpenEx(%d), device\n", ftStatus, i);
      return (ser_handler)-1;
    }
    
    printf("Opened device %s\n", cBufLD[i]);
    
   // if(getandcheckCBUS(ftHandle[i]) ) {
   //   printf( "getandcheckCBUS failed, exiting.\n" );
   //   return -1;
   // }
    
    iDevicesOpen++;
    if((ftStatus = FT_SetBaudRate(ftHandle[i], baud)) != FT_OK) {
      fprintf(stderr,"Error FT_SetBaudRate(%d), cBufLD[i] = %s\n", ftStatus, cBufLD[i]);
      break;
    }

    if((ftStatus = FT_SetDataCharacteristics(ftHandle[i], FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_EVEN)) != FT_OK) {
      fprintf(stderr,"Error FT_SetDataCharacteristics(%d) = %s\n", ftStatus, cBufLD[i]);
      break;
    }
    
    FT_SetTimeouts(ftHandle[i], 5000, 5000);	// 500 ms read/write timeout

  }
  
  iDevicesOpen = i;
  foundDevices = i; // record this in a global

  if(pcBufRead)
    free(pcBufRead);

  return 0;//(ser_handler) ftHandle[0]; // we always use the 0th device for now
}

// mode = 1 goes to bootloader, mode = 0 goes to normal
void safecast_resetboard(int mode) {
  unsigned char mask = 0;
  unsigned char ucMode;
  FT_STATUS	ftStatus;

  if( mode == 1 ) {
    mask = CBUS_BOOT_MASK;
  } else {
    mask = 0;
  }
  printf( "Resetting MCU and forcing " );
  if( mode == 1 ) {
    printf( "System Mode." );
  } else {
    printf( "normal mode." );
  }
  fflush(stdout);

  ucMode = 0xF0 | mask; // set to system memory mode
  ftStatus = FT_SetBitMode(ftHandle[stm32_ser_id], ucMode, 0x20); // CBUS bitbang mode
  if(ftStatus != FT_OK) {
    printf("Failed to set Bit Mode\n");
  } else {
    if( ser_dbg )
      printf("Set bitbang mode to %02x\n", ucMode );
  }
  Sleep(500);
  printf( "." );
  fflush(stdout);
  
  ucMode = 0xF0 | mask | CBUS_RESET_MASK; // release reset
  ftStatus = FT_SetBitMode(ftHandle[stm32_ser_id], ucMode, 0x20); // CBUS bitbang mode
  if(ftStatus != FT_OK) {
    printf("Failed to set Bit Mode\n");
  } else {
    if( ser_dbg )
      printf("Set bitbang mode to %02x\n", ucMode );
  }
  Sleep(500);
  if( mode ) 
    printf( ".should now be in system mode.\n" );
  else
    printf( ".should now be running as normal.\n" );
  
}

int closeSerialPorts() {
  int i;
  FT_STATUS	ftStatus;

  /* Cleanup */
  for(i = 0; i < foundDevices; i++) {
    if((ftStatus = FT_SetDataCharacteristics(ftHandle[i], FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE)) != FT_OK) {
      printf("Error FT_SetDataCharacteristics(%d) = %s\n", ftStatus, cBufLD[i]);
      break;
    }
    FT_Close(ftHandle[i]);
    printf("Closed device %s\n", cBufLD[i]);
  }
  return 0;
}

void printhelp(char *name, char *note) {
  printf( "Usage: %s \n", name );
}

int fw_abort() {
  closeSerialPorts();
  return 0;
}

// Get data function
static u32 writeh_read_data_page1( u8 *dst, u32 len )
{
  size_t readbytes = 0;

  if( !feof( fp_page1 ) )
    readbytes = fread( dst, 1, len, fp_page1 );
  return ( u32 )readbytes;
}

// Get data function
static u32 writeh_read_data_page4( u8 *dst, u32 len )
{
  size_t readbytes = 0;

  if( !feof( fp_page4 ) )
    readbytes = fread( dst, 1, len, fp_page4 );
  return ( u32 )readbytes;
}

// Progress function
static void writeh_progress( u32 wrote )
{
  unsigned pwrite = ( wrote * 100 ) / fpsize;
  static int expected_next = 10;

  if( pwrite >= expected_next )
  {
    printf( "%d%% ", expected_next );
    expected_next += 10;
  }
}


char *read_to_prompt(ser_handler id) {

  char *data = (char *) malloc(10000);
  int   data_size  = 10000;
  int   wrote_size = 0;
    
    
  int b=0;
  int lb=0;
  for(;;) {
    lb = b;
    b = ser_read_byte((ser_handler)id);
    if(b == -1) return data;
      
    data[wrote_size] = b;
    data[wrote_size+1] = 0;
    wrote_size++;
    printf("read: %d %c\n",b,b);
    if(wrote_size == (data_size-5)) {
        data = (char *) realloc(data,data_size+10000);
        data_size += 10000;
    }
      
    if((lb == 10) && (b == '>')) return data;
  }
    
}

char *do_get_log() {
    
    // open serial ports
    ser_handler id = openSerialPorts8N1(115200);
    
    ser_write((ser_handler)id,(const u8 *) "\r\n\r\n",4);
    Sleep(100);
    ser_set_timeout_ms((ser_handler) id, SER_NO_TIMEOUT );
    while( ser_read_byte((ser_handler)id) != -1 );
    ser_set_timeout_ms((ser_handler) id, STM32_COMM_TIMEOUT );
    
    
    printf("device id: %d\n",id);
    
    // Send Pause log
    ser_write((ser_handler)id,(const u8 *) "LOGPAUSE\n",9);
    free(read_to_prompt((ser_handler)id));
    
    // Send LOGXFER
    ser_write((ser_handler)id,(const u8 *) "LOGXFER\n",8);
    char *logdata = read_to_prompt((ser_handler)id);
    
    // Send LOGSIGN
    ser_set_timeout_ms( (ser_handler)id, SER_INF_TIMEOUT );
    ser_write((ser_handler)id,(const u8 *) "LOGSIG\n",8);
    char *logsig = read_to_prompt((ser_handler)id);
    ser_set_timeout_ms((ser_handler) id, STM32_COMM_TIMEOUT );
    
    // Send Resume log
    ser_write((ser_handler)id,(const u8 *) "LOGRESUME\n",10);
    free(read_to_prompt((ser_handler)id));
        
    // close serial ports
    closeSerialPorts();
    
    char *alldata = (char *) malloc(strlen(logdata)+strlen(logsig)+100);
    
    strcpy(alldata,logdata);
    int loglen = strlen(alldata);
    alldata[loglen]=10;
    alldata[loglen+1]=13;
    strcpy(alldata+loglen+2,logsig);
    
    free(logdata);
    free(logsig);
    return alldata;
}





//getopt from http://notes.sonots.com/Comp/CompLAng/cpp/getopt.html  
#define NULL	0
#define EOF	(-1)
#define ERR(s, c)	if(opterr){\
	char errbuf[2];\
	errbuf[0] = c; errbuf[1] = '\n';\
	fputs(argv[0], stderr);\
	fputs(s, stderr);\
	fputc(c, stderr);}
	//(void) write(2, argv[0], (unsigned)strlen(argv[0]));\
	//(void) write(2, s, (unsigned)strlen(s));\
	//(void) write(2, errbuf, 2);}

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;

int getopt(int argc,char ** argv,char *opts) {
	static int sp = 1;
	register int c;
	register char *cp;

	if(sp == 1)
		if(optind >= argc ||
		   argv[optind][0] != '-' || argv[optind][1] == '\0')
			return(EOF);
		else if(strcmp(argv[optind], "--") == NULL) {
			optind++;
			return(EOF);
		}
	optopt = c = argv[optind][sp];
	if(c == ':' || (cp=strchr(opts, c)) == NULL) {
		ERR(": illegal option -- ", c);
		if(argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if(*++cp == ':') {
		if(argv[optind][sp+1] != '\0')
			optarg = &argv[optind++][sp+1];
		else if(++optind >= argc) {
			ERR(": option requires an argument -- ", c);
			sp = 1;
			return('?');
		} else
			optarg = argv[optind++];
		sp = 1;
	} else {
		if(argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}



int do_flash_main(int argc, char **argv) {

  int aflag = 0;
  char *argval = NULL;
  char infile_name[256];
  int index;
  int c;
  int baud = 115200;
  u8 minor, major;
  u16 version;
  int badness = 0;
  int readflag = 0;
  int readoffset = 0;

  opterr = 0;
  infile_name[0] = '\0';
  ser_dbg = 0;

  while ((c = getopt(argc, argv, "b:f:dr:")) != -1 ) {
    switch (c) {
      
    case 'a': 
      aflag = 1;
      break;
      
    case 'b':
      argval = optarg;
      baud = strtol(argval, NULL, 0);
      if( baud < 1200 || baud > 115200 ) {
	printf( "Baud should be between 1200 and 115200; got: %d\n", baud );
	return 0;
      }
      break;
      
    case 'f':
      strncpy(infile_name, optarg, 256);
      break;
      
    case 'd':
      ser_dbg = 1;
      break;
      
    case 'r':
      argval = optarg;
      readoffset = strtol(argval, NULL, 0);
      readflag = 1;
      break;
      
    case '?':
      printhelp(argv[0], NULL);
      break;

    default:
      printhelp(argv[0], NULL);
      fw_abort();
    }
  }

  if( !readflag ) {
    fp_page1 = fopen( infile_name, "rb" );
    fp_page4 = fopen( infile_name, "rb" );

    if((fp_page1 == NULL) || (fp_page4 == NULL)) {
      fprintf( stderr, "Unable to open %s\n", infile_name );
      exit( 1 );
    }  else    {
      fseek( fp_page4, 0, SEEK_END );
      fpsize = ftell( fp_page4 );
      fseek( fp_page4, 0, SEEK_SET );
    }
    fseek( fp_page4, 6144, SEEK_SET);
  }

  // Connect to bootloader
  printf( "Connect to bootloader.\n" );
  if( stm32_init( NULL, baud ) != STM32_OK )
  {
    fprintf( stderr, "Unable to connect to bootloader\n" );
  }
  printf( "\n" );
  
  // Get version
  printf( "Get version.\n" );
  if( stm32_get_version( &major, &minor ) != STM32_OK )
  {
    fprintf( stderr, "Unable to get bootloader version\n" );
    //    exit( 1 );
    badness = 1;
  }
  else
  {
    printf( "Found bootloader version: %d.%d\n", major, minor );
    if( BL_MKVER( major, minor ) < BL_MINVERSION )
    {
      fprintf( stderr, "Unsupported bootloader version" );
      exit( 1 );
    }
  }
  
  // Get chip ID
  printf( "Get chip ID.\n" );
  if( stm32_get_chip_id( &version ) != STM32_OK )
  {
    fprintf( stderr, "Unable to get chip ID\n" );
    badness = 1;
    //    exit( 1 );
  }
  else
  {
    printf( "Chip ID: %04X\n", version );
    if( version != CHIP_ID )
    {
      fprintf( stderr, "Unsupported chip ID" );
      exit( 1 );
    }
  }
  if( badness )
    exit( 1 );
  
  if( !readflag ) {
    // Write unprotect
    printf( "Write unprotect.\n" );
    if( stm32_write_unprotect() != STM32_OK )
      {
	fprintf( stderr, "Unable to execute write unprotect\n" );
	exit( 1 );
      }
    else
      printf( "Cleared write protection.\n" );

    // Read unprotect
    printf( "Read unprotect.\n" );
    if( stm32_read_unprotect() != STM32_OK )
      {
	fprintf( stderr, "Unable to execute read unprotect\n" );
	exit( 1 );
      }
    else
      printf( "Cleared read protection.\n" );

    // Erase flash
    printf( "Erase flash.\n" );
    int res1 = stm32_erase_flash_page(0,1);     // first page
    int res2 = stm32_erase_flash_page(3,0xFD);  // all the rest
    if(res1 != STM32_OK) {
	    fprintf( stderr, "Unable to erase chip - pre prvkey\n" );
	    exit(1);
    }
    if(res2 != STM32_OK) {
	    fprintf( stderr, "Unable to erase chip - post pk\n" );
	    exit(1);
    }

    printf( "Erased FLASH memory.\n" );

    // Program flash
    setbuf( stdout, NULL );
    printf( "Programming flash ... ");
    if( stm32_write_flash_page(0x08000000,1,writeh_read_data_page1, writeh_progress ) != STM32_OK ) {
      fprintf( stderr, "Unable to program - initial page.\n" );
	    exit(1);
    } else {
      printf( "\nProgram pre-pk Done.\n" );
    }

    if( stm32_write_flash_page(0x08001800,0xFC,writeh_read_data_page4, writeh_progress ) != STM32_OK ) {
      fprintf( stderr, "Unable to program - post pk flash.\n" );
	    exit(1);
    } else {
      printf( "\nProgram post-pk Done.\n" );
    }


  } else {
    printf( "Readback flash memory at offset %x\n", readoffset );
    if(stm32_read_flash( readoffset, 10240 ) != STM32_OK ) {
      fprintf( stderr, "Unable to read FLASH memory.\n" );
      exit( 1 );
    } else {
      printf( "\nDone.\n" );
    }
  }

  // reset back into normal mode
  printf( "\n" );
  safecast_resetboard(0);

  if( !readflag ) {
    fclose( fp_page1 );
    fclose( fp_page4 );
  }

  closeSerialPorts();

  return 0;
}

