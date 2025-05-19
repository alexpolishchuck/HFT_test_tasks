#pragma once
#include <sstream>
#include <vector>
#include <fstream>

class double_file_reader
{
public:
    double_file_reader(
        const std::string& filename,
        uint64_t max_memory_bytes);

    void read(std::vector<double>& output);

private:
    bool populate_buffer();
    bool check_buffer_stream_empty() const;
    void roll_back_file_cursor_to_last_endl();
    void read_raw_buffer_to_double_buffer(std::vector<double>& output);
    void flush_stream(std::vector<double>& output);

private:
    std::streamsize raw_bytes_read_ = 0;
    uint64_t chunk_size_bytes_;
    std::vector<char> buffer_;
    std::stringstream buffer_stream_;
    std::ifstream file_;
};

