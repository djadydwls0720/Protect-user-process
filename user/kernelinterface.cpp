#include "kernelinterface.hpp"
#include <string.h>

BOOL kernelInterface::Init() {
    this->hDevice = CreateFileA(
        "\\\\.\\userprotect",
        GENERIC_ALL,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device: %x\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}
BOOL kernelInterface::ProtectProcess(PROTECT_PROCESS Process) {
    if (this->hDevice == NULL) 
        printf("Device is NULL\n");

    PROTECT_PROCESS_PACKET packet;
    strcpy_s(packet.ProcessName, 0x30, Process.ProcessName);
    packet.Operation = Process.Operation;
    packet.Terminate = Process.Terminate;
    packet.VM_Read = Process.VM_Read;
    packet.VM_Write = Process.VM_Write;



    if (DeviceIoControl(
        this->hDevice,
        IO_REQUEST_PROCESS_PROTECT,
        &packet,
        sizeof(packet),
        NULL,
        NULL,
        NULL,
        NULL
    )) {
        printf("[*] Process Protect success! %s\n", Process.ProcessName);
        return TRUE;
    }


    return FALSE;
}


BOOL kernelInterface::UnProtectProcess(PROTECT_PROCESS Process) {
    if (this->hDevice == NULL)
        printf("Device is NULL\n");

    PROTECT_PROCESS_PACKET packet;
    strcpy_s(packet.ProcessName, 0x30, Process.ProcessName);
    packet.Operation = Process.Operation;
    packet.Terminate = Process.Terminate;
    packet.VM_Read = Process.VM_Read;
    packet.VM_Write = Process.VM_Write;



    if (DeviceIoControl(
        this->hDevice,
        IO_REQUEST_PROCESS_UNPROTECT,
        &packet,
        sizeof(packet),
        NULL,
        NULL,
        NULL,
        NULL
    )) {
        printf("[*] Process UnProtect success! %s\n", Process.ProcessName);
        return TRUE;
    }


    return FALSE;
}