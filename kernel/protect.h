#include "data.h"
#pragma warning (disable: 4047 4024 4213)



NTSTATUS Get_ProcessId(PIRP irp) {
	PULONGLONG OutPut = (PULONGLONG)irp->AssociatedIrp.SystemBuffer;
	*OutPut = ProcessId_global;
	DbgPrintEx(0, 0, "process id rqeust %lld\n", ProcessId_global);

	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = sizeof(PULONGLONG);
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


BOOL ListInProcName(char* ProcName) {
	for (int i = 0; i < MaxIndex; i++) {
		if (!_strnicmp(ProcName, List[i]->ProcessName, sizeof(ProcName))) {
			return TRUE;
		}
	}

	return FALSE;
}

OB_PREOP_CALLBACK_STATUS PreCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation) {
	UNREFERENCED_PARAMETER(RegistrationContext);

	char ProcName[0x20] = {0,};
	strcpy_s(ProcName, 0x20, (char*)((DWORD64)pOperationInformation->Object + Offset.imageFileName_off));

	for (int i = 0; i < MaxIndex; i++) {
		if (ListInProcName(ProcName)) {
			if (pOperationInformation->Operation == OB_OPERATION_HANDLE_CREATE) {
				if (!List[i]->Terminate && (pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE) {
					pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
					DbgPrintEx(0, 0, "Terminate");
				}
				if (!List[i]->Operation && (pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) == PROCESS_VM_OPERATION) {
					pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
					DbgPrintEx(0, 0, "Operation");
				}
				if (!List[i]->VM_Read &&(pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) == PROCESS_VM_READ) {
					pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
					DbgPrintEx(0, 0, "VM_Read");
				}
				if (!List[i]->VM_Write &&(pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) == PROCESS_VM_WRITE) {
					pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
					DbgPrintEx(0, 0, "Write");
				}
			}

			DbgPrintEx(0,0,"%s\n",List[i]->ProcessName);
		}
	}

	return OB_PREOP_SUCCESS;
}

NTSTATUS PostCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation) {
	UNREFERENCED_PARAMETER(RegistrationContext);

	PLIST_ENTRY pListEntry = { 0, };
	char ProcName[0x20] = { 0, };

	strcpy_s(ProcName, 0x20, (char*)((DWORD64)pOperationInformation->Object + Offset.imageFileName_off));

	for (int i = 0; i < MaxIndex; i++) {
		if (ListInProcName(ProcName)) {
			pListEntry = (PLIST_ENTRY)((DWORD64)pOperationInformation->Object + Offset.ActiveProcessLinks_off);
			if (pListEntry->Blink != NULL && pListEntry->Flink != NULL) {
				pListEntry->Blink->Flink = pListEntry->Flink;
				pListEntry->Flink->Blink = pListEntry->Blink;

				pListEntry->Blink = 0;
				pListEntry->Flink = 0;
			}

		}

	}


	return OB_PREOP_SUCCESS;
}


BOOLEAN GetPEBOffset() {
	int LinkOffset = Offset.ActiveProcessLinks_off;

	UNICODE_STRING routine;
	RtlInitUnicodeString(&routine, szNtQueryInformationProcess);
	NtqueryInformationProcess_t NtQueryInformationProcess = (NtqueryInformationProcess_t)MmGetSystemRoutineAddress(&routine);

	BOOLEAN success = FALSE;
	ULONG Ret;
	PEPROCESS Process = PsGetCurrentProcess(); 
	for (int i = 0; i < 16; i++) {
		PROCESS_BASIC_INFORMATION ProcessInformation = { 0, };
		PLIST_ENTRY ListEntry = (PLIST_ENTRY)((ULONGLONG)Process + LinkOffset);
		Process = (PEPROCESS)((PCHAR)ListEntry->Flink - LinkOffset);
		HANDLE key=NULL;
		if (NT_SUCCESS(ObOpenObjectByPointer(Process, NULL,NULL,NULL, *PsThreadType,KernelMode,&key))) {
			Ret = NULL;
			NtQueryInformationProcess(key,ProcessBasicInformation, &ProcessInformation, sizeof(ProcessInformation), &Ret);
			ZwClose(key);
		}

		if (ProcessInformation.PebBaseAddress) {
			for (int j = Offset.ActiveProcessLinks_off; j < PAGE_SIZE; j+=4) {
				if ((PPEB)((ULONGLONG)Process + j) == ProcessInformation.PebBaseAddress) {
					Offset.PEB_off = j;
					success = TRUE;
					return success;
				}
			}
		}

	}
	return success;
}

BOOLEAN GetOffset(PEPROCESS Process) {
	BOOLEAN success = FALSE;
	HANDLE PID = PsGetCurrentProcessId();
	PLIST_ENTRY ListEntry = { 0, };
	PLIST_ENTRY NextEntry = { 0, };

	for (int i = 0; i < PAGE_SIZE; i++) {
		if (*(PHANDLE)((ULONGLONG)Process + i) == PID) {
			ListEntry = (PLIST_ENTRY)((ULONGLONG)Process + i + 0x8);
			if (MmIsAddressValid(ListEntry) && MmIsAddressValid(ListEntry->Flink)) {
				NextEntry = ListEntry->Flink;
				if (ListEntry == NextEntry->Blink) {
					Offset.UniqueProcessid_off = i;
					Offset.ActiveProcessLinks_off = i + 8;
					success = TRUE;
				}
			}
		}
	}

	if (!success) {
		DbgPrintEx(0, 0, "[*] Not Found");
		return FALSE;
	}

	for (int i = Offset.ActiveProcessLinks_off; i < PAGE_SIZE; i++) {
		if (!strncmp((PCHAR)Process+i, szSystem, 6)) {
			Offset.imageFileName_off = i;
			success = TRUE;
			break;
		}
	}
	if (!success)
		return FALSE;

	if (GetPEBOffset())
		return success;
	return success;
}


VOID append(PPROTECT_PROCESS Process) {
	List[MaxIndex] = Process;
	MaxIndex++;
}

VOID remove(PPROTECT_PROCESS Process) {
	int i;
	for (i = 0; i < MaxIndex; i++) {
		if (List[i] == Process)
			break;
	}
	if (MaxIndex == i - 1)
		return;
	for (int j = i; j < MaxIndex; j++) {
		List[j] = List[j+1];
	}
	MaxIndex--;
	ExFreePool(Process);
}

NTSTATUS ProtectPrcoess(PIRP irp) 
{
	PPROTECT_PROCESS Protect_Process;
	Protect_Process = (PPROTECT_PROCESS)ExAllocatePool(NonPagedPool, sizeof(PPROTECT_PROCESS));
	if (Protect_Process == NULL) {
		DbgPrintEx(0, 0, "[*] ProtectPrcoess failed");
		IoCompleteRequest(irp, IO_NO_INCREMENT);
		return STATUS_UNSUCCESSFUL;
	}
	

	RtlCopyMemory(Protect_Process, irp->AssociatedIrp.SystemBuffer, sizeof(PROTECT_PROCESS));
	for (int i = 0; i < MaxIndex; i++) {
		if (!strcmp(List[i]->ProcessName, Protect_Process->ProcessName)) {
			DbgPrintEx(0, 0, "[*] duplication Process!");
			irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			irp->IoStatus.Information = sizeof(PROTECT_PROCESS);
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return STATUS_UNSUCCESSFUL;
		}
	}

	append(Protect_Process);


	for (int i = 0; i < MaxIndex; i++) {
		DbgPrintEx(0, 0, "[*] Process Name: %s", List[i]->ProcessName);

	}
	DbgPrintEx(0, 0, "[*] MaxIndex: %d", MaxIndex);


	DbgPrintEx(0, 0, "[*] ProtectPrcoess");
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = sizeof(PROTECT_PROCESS);

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS UnProtectPrcoess(PIRP irp)
{
	PPROTECT_PROCESS Protect_Process = (PPROTECT_PROCESS)irp->AssociatedIrp.SystemBuffer;
	for (int i = 0; i < MaxIndex; i++) {
		if (!strcmp(List[i]->ProcessName, Protect_Process->ProcessName)) {

			DbgPrintEx(0, 0, "[*] Remove Process %s", List[i]->ProcessName);
			remove(List[i]);
			irp->IoStatus.Status = STATUS_SUCCESS;
			irp->IoStatus.Information = sizeof(PROTECT_PROCESS);
			IoCompleteRequest(irp, IO_NO_INCREMENT);
			return STATUS_SUCCESS;
		}
	}


	DbgPrintEx(0, 0, "[*] Process is NULL..");

	irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	irp->IoStatus.Information = sizeof(PROTECT_PROCESS);
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_UNSUCCESSFUL;
}