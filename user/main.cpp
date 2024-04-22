#include "kernelinterface.hpp"
#include <string.h>

int main() {
	kernelInterface Interface;
	PROTECT_PROCESS notepad;
	notepad.Operation = FALSE;
	strcpy_s(notepad.ProcessName,0x20,"notepad.exe");
	notepad.Terminate = TRUE;
	notepad.VM_Read = FALSE;
	notepad.VM_Write = FALSE;


	Interface.Init();
	Interface.ProtectProcess(notepad);
}