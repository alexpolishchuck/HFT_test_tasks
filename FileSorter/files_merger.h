#pragma once
#include <string>
#include <vector>

#include "consts.h"

class files_merger
{
public:
    void merge_two_files(
        const std::string& file_a,
        const std::string& file_b,
        const std::string& file_out,
        uint64_t max_memory_bytes = g_max_available_bytes);

private:
    void flush_to_output_buffer(std::vector<double>& input, size_t& cursor);
    void merge_buffers();
    void flush_output_to_file();

private:
    std::string file_out_;
    uint64_t chunk_size_ = 0;
    std::vector<double> doubles_a_;
    std::vector<double> doubles_b_;
    std::vector<double> doubles_out_;
    size_t cursor_a_ = 0;
    size_t cursor_b_ = 0;
};

