#pragma once

#include <type_traits>
#include <string>
#include "../types.h"
#include "../string-processing/string-processing.h"
#include <Windows.h>
#include <stdio.h>
#include <Psapi.h>
#include <Pdh.h>

#pragma comment(lib, "Pdh.lib")

using namespace string_processing;

/* Memory control for pointer (with reference) :) */
template<typename T>
void releaseMemory(T*& memory) {
	if (memory != nullptr) {
		delete memory;
		memory = nullptr;
	}
}

/* Memory control other :| */
template<typename K = std::nullptr_t>
void releaseMemory(K memory) noexcept(false) {
	if (std::is_pointer<K>::value && memory != nullptr) {
		delete memory;
	}
	else if (
		std::is_compound<K>::value &&
		std::is_reference<K>::value &&
		!std::is_function<K>::value &&
		memory != nullptr
		) {
		delete memory;
		memory = nullptr;
	}
}

long getPrivateWorkingSet(const char* process = nullptr);
void printMemory();

float getMemoryCurrentProcess();
std::string getFormattedMemory(float bytes, short max);
std::string getMemoryCurrentProcessInfo(short max = 1);