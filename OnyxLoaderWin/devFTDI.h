namespace Api.FTDI.D2XX
{
	public __gc class devFTDI
	{
	const UInt32 FT_LIST_NUMBER_ONLY = 0x80000000;
	const UInt32 FT_LIST_BY_INDEX	= 0x40000000;
	const UInt32 FT_LIST_ALL			= 0x20000000;
	const UInt32 FT_OPEN_BY_SERIAL_NUMBER = 1;
	const UInt32 FT_OPEN_BY_DESCRIPTION    = 2;

	const UInt32 FT_EVENT_RXCHAR		= 1;
	const UInt32 FT_EVENT_MODEM_STATUS  = 2;

#define FT_PREFIX [DllImport("FTD2XX.dll")]

enum {
    FT_OK,
    FT_INVALID_HANDLE,
    FT_DEVICE_NOT_FOUND,
    FT_DEVICE_NOT_OPENED,
    FT_IO_ERROR,
    FT_INSUFFICIENT_RESOURCES,
    FT_INVALID_PARAMETER,
    FT_INVALID_BAUD_RATE,

    FT_DEVICE_NOT_OPENED_FOR_ERASE,
    FT_DEVICE_NOT_OPENED_FOR_WRITE,
    FT_FAILED_TO_WRITE_DEVICE,
    FT_EEPROM_READ_FAILED,
    FT_EEPROM_WRITE_FAILED,
    FT_EEPROM_ERASE_FAILED,
	FT_EEPROM_NOT_PRESENT,
	FT_EEPROM_NOT_PROGRAMMED,
	FT_INVALID_ARGS,
	FT_NOT_SUPPORTED,
	FT_OTHER_ERROR
};

	typedef void * FT_HANDLE;
	typedef unsigned long DWORD;
	typedef unsigned long FT_STATUS;
	typedef void * LPVOID;
	typedef void * PVOID;
	typedef DWORD * LPDWORD;
	typedef DWORD ULONG;
	typedef unsigned short USHORT;
	typedef unsigned char UCHAR;
	typedef unsigned short WORD;
	typedef WORD * LPWORD;
	typedef unsigned char UCHAR;
	typedef UCHAR * PUCHAR;
	typedef char CHAR;
	typedef CHAR * PCHAR;
	typedef ULONG FT_DEVICE;
	typedef void *HANDLE;
	typedef int BOOL;
	#define FALSE   0
	#define TRUE    1
	// as c++. net is a managed application and our ftd2xx.dll is unmanaged code you must declare the functions here explicitly
	// to allow you to call them within the application. An include file and the .lib file simply wont work with c++.net. Its
	// a similar problem in c#.
	// see http://www.codeguru.com/Cpp/COM-Tech/complus/managed/article.php/c3947/ for more on this.
	// I have only included these 4 functions in this to show you how to do this. For other functions that you require similar
	// declerations will need to go here. 
 
	using namespace System::Runtime::InteropServices;
	FT_PREFIX FT_STATUS FT_Open(int deviceNumber, FT_HANDLE * pHandle);
	FT_PREFIX FT_STATUS FT_OpenEx(PVOID pArg1, DWORD Flags, FT_HANDLE *pHandle);
	FT_PREFIX FT_STATUS FT_ListDevices(PVOID pArg1, PVOID pArg2, DWORD Flags);
	FT_PREFIX FT_STATUS FT_ListDevices(UInt32 pvArg1, void * pvArg2, UInt32 dwFlags);	// FT_ListDevcies by serial number or description by index only
	FT_PREFIX FT_STATUS FT_Close(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS FT_Read(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesReturned);
	FT_PREFIX FT_STATUS FT_Write(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD nBufferSize, LPDWORD lpBytesWritten);
	FT_PREFIX FT_STATUS FT_SetBaudRate(FT_HANDLE ftHandle,	ULONG BaudRate);
	FT_PREFIX FT_STATUS FT_SetDivisor(FT_HANDLE ftHandle,USHORT Divisor);
	FT_PREFIX FT_STATUS FT_SetDataCharacteristics(FT_HANDLE ftHandle, UCHAR WordLength, UCHAR StopBits, UCHAR Parity);
	FT_PREFIX FT_STATUS FT_SetFlowControl(FT_HANDLE ftHandle, USHORT FlowControl, UCHAR XonChar, UCHAR XoffChar	);
	FT_PREFIX FT_STATUS FT_ResetDevice(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS FT_SetDtr(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS FT_ClrDtr(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS FT_SetRts(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS FT_ClrRts(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS FT_GetModemStatus(FT_HANDLE ftHandle,	ULONG *pModemStatus);
	FT_PREFIX FT_STATUS FT_SetChars(FT_HANDLE ftHandle, UCHAR EventChar, UCHAR EventCharEnabled, UCHAR ErrorChar, UCHAR ErrorCharEnabled);
	FT_PREFIX FT_STATUS FT_Purge(FT_HANDLE ftHandle, ULONG Mask);
	FT_PREFIX FT_STATUS FT_SetTimeouts(FT_HANDLE ftHandle,	ULONG ReadTimeout,	ULONG WriteTimeout);
	FT_PREFIX FT_STATUS FT_GetQueueStatus(FT_HANDLE ftHandle, DWORD *dwRxBytes);
	FT_PREFIX FT_STATUS FT_SetEventNotification(FT_HANDLE ftHandle, DWORD Mask,	PVOID Param	);
	FT_PREFIX FT_STATUS FT_GetStatus(FT_HANDLE ftHandle, DWORD *dwRxBytes, DWORD *dwTxBytes, DWORD *dwEventDWord);
	FT_PREFIX FT_STATUS FT_SetBreakOn(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS FT_SetBreakOff(FT_HANDLE ftHandle);	
	FT_PREFIX FT_STATUS FT_SetWaitMask(FT_HANDLE ftHandle, DWORD Mask);
	FT_PREFIX FT_STATUS FT_WaitOnMask(FT_HANDLE ftHandle, DWORD *Mask);
	FT_PREFIX FT_STATUS FT_GetEventStatus(FT_HANDLE ftHandle, DWORD *dwEventDWord);
	FT_PREFIX FT_STATUS FT_ReadEE(FT_HANDLE ftHandle, DWORD dwWordOffset, LPWORD lpwValu);
	FT_PREFIX FT_STATUS FT_WriteEE(FT_HANDLE ftHandle,	DWORD dwWordOffset, WORD wValue);
	FT_PREFIX FT_STATUS FT_EraseEE(FT_HANDLE ftHandle);
// Missed out the programming stuff +++ 

	FT_PREFIX FT_STATUS  FT_EE_UASize(FT_HANDLE ftHandle, LPDWORD lpdwSize);
	FT_PREFIX FT_STATUS  FT_EE_UAWrite(FT_HANDLE ftHandle, PUCHAR pucData, DWORD dwDataLen	);
	FT_PREFIX FT_STATUS  FT_EE_UARead(FT_HANDLE ftHandle, PUCHAR pucData, DWORD dwDataLen, LPDWORD lpdwBytesRead);
	FT_PREFIX FT_STATUS  FT_SetLatencyTimer(FT_HANDLE ftHandle, UCHAR ucLatency);
	FT_PREFIX FT_STATUS  FT_GetLatencyTimer(FT_HANDLE ftHandle, PUCHAR pucLatency);
	FT_PREFIX FT_STATUS  FT_SetBitMode(FT_HANDLE ftHandle, UCHAR ucMask, UCHAR ucEnable);
	FT_PREFIX FT_STATUS  FT_GetBitMode(FT_HANDLE ftHandle, PUCHAR pucMode);
	FT_PREFIX FT_STATUS  FT_SetUSBParameters(FT_HANDLE ftHandle, ULONG ulInTransferSize, ULONG ulOutTransferSize);
	FT_PREFIX FT_STATUS  FT_GetDeviceInfo(FT_HANDLE ftHandle, FT_DEVICE *lpftDevice, LPDWORD lpdwID, PCHAR SerialNumber, PCHAR Description,	LPVOID Dummy);
	FT_PREFIX FT_STATUS  FT_StopInTask(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS  FT_RestartInTask(FT_HANDLE ftHandle);
	FT_PREFIX FT_STATUS  FT_SetResetPipeRetryCount(FT_HANDLE ftHandle, DWORD dwCount);
	FT_PREFIX FT_STATUS  FT_ResetPort(FT_HANDLE ftHandle);

	// need these kernel functions for the Event Handling stuff
	[DllImport("Kernel32.dll")] DWORD WaitForSingleObject(HANDLE hHandle,  DWORD dwMilliseconds);
	[DllImport("Kernel32.dll")] HANDLE CreateEvent(void * pNULL, BOOL bManualReset, BOOL bInitialState, char * pcNULL);
	[DllImport("Kernel32.dll")] BOOL SetEvent(HANDLE hEvent);
	}
}