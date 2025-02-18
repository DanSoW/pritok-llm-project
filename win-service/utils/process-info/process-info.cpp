#include "process-info.h"

#ifdef WIN_OS

int get_current_thread_count() {
	// First determine the id of the current process
	const DWORD id = GetCurrentProcessId();

	// Then get a process list snapshot
	const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);

	// Initialize the process entry structure
	PROCESSENTRY32 entry = { 0 };
	entry.dwSize = sizeof(entry);

	// Get the current process info
	BOOL ret = true;
	ret = Process32First(snapshot, &entry);

	while (ret && entry.th32ProcessID != id) {
		ret = Process32Next(snapshot, &entry);
	}

	CloseHandle(snapshot);

	return ret ? entry.cntThreads : -1;
}

#endif