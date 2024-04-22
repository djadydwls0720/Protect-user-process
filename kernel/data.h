#pragma once
#include <string.h>
#include <ntifs.h>
#include <ntdef.h>
#include <windef.h>
#include <ntddk.h>
//#ifndef A

NTSTATUS UnloadDriver(PDRIVER_OBJECT pDriverObject);
#define PROCESS_TERMINATE    0x0001
#define PROCESS_VM_OPERATION 0x0008
#define PROCESS_VM_READ      0x0010
#define PROCESS_VM_WRITE	 0x0020
#define MAX_PROCESS 0x30

#define IO_REQUEST_PROCESSID CTL_CODE(FILE_DEVICE_UNKNOWN, 0x666, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_REQUEST_PROCESS_PROTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x667, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define IO_REQUEST_PROCESS_UNPROTECT CTL_CODE(FILE_DEVICE_UNKNOWN, 0x668, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)


NTSTATUS InitObRegExample();
OB_PREOP_CALLBACK_STATUS PreCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation);
NTSTATUS PostCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation);
BOOLEAN GetPEBOffset();
BOOLEAN GetOffset(PEPROCESS Process);
NTSTATUS IoControl(PDEVICE_OBJECT DeviceObject, PIRP irp);
NTSTATUS CloseCall(PDEVICE_OBJECT DeviceObject, PIRP irp);
NTSTATUS CreateCall(PDEVICE_OBJECT DeviceObject, PIRP irp);
NTSTATUS Get_ProcessId(PIRP irp);
OB_PREOP_CALLBACK_STATUS PreCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation);

//#define  A
//#endif // !A


ULONGLONG ProcessId_global;
UNICODE_STRING dev;
UNICODE_STRING dos;
PDEVICE_OBJECT pDeviceObject = NULL;




typedef NTSTATUS(*NtqueryInformationProcess_t) (
	HANDLE ProcessHandle,
	PROCESSINFOCLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength,
	PULONG ReturnLength
	);

typedef struct _IMPORT_OFFSET {
	int UniqueProcessid_off;
	int ActiveProcessLinks_off;
	int imageFileName_off;
	int PEB_off;
}IMPORT_OFFSET;


typedef struct _PROTECT_PROCESS_PACKET {
	char ProcessName[0x20];
	BOOL Terminate;
	BOOL VM_Read;
	BOOL VM_Write;
	BOOL Operation;
}PROTECT_PROCESS_PACKET, * PPROTECT_PROCESS_PACKET;

typedef struct _PROTECT_PROCESS {
	char ProcessName[0x20];
	BOOL Terminate;
	BOOL VM_Read;
	BOOL VM_Write;
	BOOL Operation;
}PROTECT_PROCESS, * PPROTECT_PROCESS;


ULONG MaxIndex = 0;
PPROTECT_PROCESS List[0x30];
HANDLE hPid;
IMPORT_OFFSET Offset;
const char szSystem[] = "System";
const wchar_t szNtQueryInformationProcess[] = L"NtQueryInformationProcess";
