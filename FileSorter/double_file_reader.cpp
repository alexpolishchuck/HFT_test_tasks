#include <iostream>

#include "double_file_reader.h"

bool is_symbol_endl(std::vector<char>& buffer, size_t pos)
{
    return buffer[pos] == '\n';
}

double_file_reader::double_file_reader(
    const std::string& filename,
    uint64_t chunk_size_bytes)
    : chunk_size_bytes_(chunk_size_bytes / 2)
    , buffer_(chunk_size_bytes_)
{
    file_.open(filename, std::ios::binary);
    if (!file_.is_open())
        throw std::runtime_error("file_reader: failed to open file");
}

void double_file_reader::read(std::vector<double>& output)
{
    output.clear();

    if (!check_buffer_stream_empty())
    {
        flush_stream(output);
        if (output.size() == output.capacity())
            return;
    }

    if (!populate_buffer())
        return;

    read_raw_buffer_to_double_buffer(output);
}

bool double_file_reader::populate_buffer()
{
    file_.read(buffer_.data(), buffer_.size());
    raw_bytes_read_ = file_.gcount();

    if (!raw_bytes_read_)
        return false;

    bool is_last_symbol_endl = buffer_[raw_bytes_read_ - 1] == '\n';
    if (!is_last_symbol_endl && !file_.eof())
    {
        roll_back_file_cursor_to_last_endl();
    }

    return raw_bytes_read_;
}

bool double_file_reader::check_buffer_stream_empty() const
{
    bool is_error_in_stream = !buffer_stream_;
    if (is_error_in_stream)
        return true;

    bool is_eof = buffer_stream_.eof();
    if (is_eof)
        return true;

    bool is_buf_empty = !buffer_stream_.rdbuf()->in_avail();
    return is_buf_empty;
}

void double_file_reader::roll_back_file_cursor_to_last_endl()
{
    std::streamsize bytes_read = file_.gcount();
    int cur_pos = bytes_read - 1;
    while (cur_pos > 0)
    {
        if (buffer_[cur_pos] == '\n')
        {
            cur_pos++;
            break;
        }

        cur_pos--;
    }

    std::streamsize bytes_moved_back = bytes_read - cur_pos;
    file_.seekg(-bytes_moved_back, std::ios::cur);

    raw_bytes_read_ -= bytes_moved_back;
}

void double_file_reader::read_raw_buffer_to_double_buffer(
    std::vector<double>& output)
{
    buffer_stream_.clear();
    buffer_stream_.str(std::string(buffer_.data(), raw_bytes_read_));
    flush_stream(output);
}

void double_file_reader::flush_stream(std::vector<double>& output)
{
    double value = 0;
    while (true)
    {
        if (output.size() == output.capacity())
        {
            return;
        }

        if (buffer_stream_ >> value)
            output.emplace_back(value);
        else
            break;
    }
}