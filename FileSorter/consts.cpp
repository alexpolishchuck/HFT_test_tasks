#include "pch.h"
#include "consts.h"
const char* g_temp_folder = "temp";
const uint64_t g_max_memory_bytes = 100 * 1024 * 1024; // 100 MB
const uint64_t g_reserved_bytes = 10 * 1024 * 1024; // 10 MB, reserved for colleteral performance expenses
const uint64_t g_max_available_bytes = g_max_memory_bytes - g_reserved_bytes;