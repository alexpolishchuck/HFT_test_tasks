#pragma once
#include <string>
#include <vector>

void clear_and_create_folder(const char* folder_path);
void flush_chunk_to_file(
    const std::string& filename,
    const std::vector<double>& data, 
    bool truncate_file = true);
