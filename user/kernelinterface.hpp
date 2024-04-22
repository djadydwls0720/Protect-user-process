#pragma once
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define IO_REQUEST_PROCESSID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x666, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_REQUEST_PROCESS_PROTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x667, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_REQUEST_PROCESS_UNPROTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x668, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

typedef struct _PROTECT_PROCESS_PACKET {
	char ProcessName[0x30];
	BOOL Terminate;
	BOOL VM_Read;
	BOOL VM_Write;
	BOOL Operation;
}PROTECT_PROCESS_PACKET, * PPROTECT_PROCESS_PACKET;

typedef struct _PROTECT_PROCESS {
	char ProcessName[0x30];
	BOOL Terminate;
	BOOL VM_Read;
	BOOL VM_Write;
	BOOL Operation;
}PROTECT_PROCESS, * PPROTECT_PROCESS;


class kernelInterface {
public:
	HANDLE hDevice;
	BOOL Init();
	BOOL ProtectProcess(PROTECT_PROCESS Process);
	BOOL UnProtectProcess(PROTECT_PROCESS Process);
};