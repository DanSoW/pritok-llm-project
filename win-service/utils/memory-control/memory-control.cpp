#include "memory-control.h"

long getPrivateWorkingSet(const char* process) {
	if (process == nullptr) {
		return 0;
	}

	PDH_HQUERY query;
	PDH_HCOUNTER counter;
	PDH_FMT_COUNTERVALUE counterVal;

	const char* name = "\\Process(%s)\\Working Set - Private";
	char buf[1024] = "";

	sprintf_s(buf, 1024, name, process);

	PdhOpenQuery(nullptr, 0, &query);
	PDH_STATUS res = PdhAddEnglishCounter(query, buf, 0, &counter);

	res = PdhCollectQueryData(query);
	PdhGetFormattedCounterValue(counter, PDH_FMT_LONG, nullptr, &counterVal);

	long ret = counterVal.longValue;
	PdhCloseQuery(query);

	return ret;
}

void printMemory() {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

	printf("Private committed memory: %u B\nWorking set: %u B\n", (UINT)pmc.PrivateUsage, (UINT)pmc.WorkingSetSize);
	printf("Private working set: %u B\n", (UINT)getPrivateWorkingSet("Program"));

	HANDLE heaps[20];
	HEAP_SUMMARY hs;

	DWORD cHeaps = GetProcessHeaps(20, heaps);

	for (DWORD i = 0; i < cHeaps; i++) {
		memset(&hs, 0, sizeof(hs));
		hs.cb = sizeof(hs);

		if (!HeapSummary(heaps[i], 0, &hs)) {
			printf("HeapSummary failed");
		}

		printf("Heap #%u size: %u B\n", (UINT)i, (UINT)hs.cbAllocated);
	}

	printf("\n");
}

float getMemoryCurrentProcess() {
	PROCESS_MEMORY_COUNTERS_EX pmc;
	GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

	return (float)pmc.WorkingSetSize;
}

std::string getFormattedMemory(float bytes, short max) {
	if (max == 0) {
		return std::string(to_number_precision(bytes, 2) + " B");
	}

	ushort i = 0;
	std::string marks[4] = {
		"B",
		"Kb",
		"Mb",
		"Gb",
	};

	while (bytes >= 1024) {
		bytes /= 1024;
		i++;

		if ((max > 0) && (i == max)) {
			break;
		}
	}

	return std::string(to_number_precision(bytes, 2) + " " + marks[i]);
}

std::string getMemoryCurrentProcessInfo(short max) {
	return getFormattedMemory(getMemoryCurrentProcess(), max);
}