#include "pch.h"
#include "files_merger.h"
#include "double_file_reader.h"
#include "utils.h"

void files_merger::merge_two_files(
    const std::string& file_a,
    const std::string& file_b,
    const std::string& file_out,
    uint64_t max_memory_bytes)
{
    file_out_ = file_out;
    std::filesystem::remove(file_out_);

    chunk_size_ = max_memory_bytes / 5; // two file readers + 3 vectors
    double_file_reader file_reader_a(file_a, chunk_size_);
    double_file_reader file_reader_b(file_b, chunk_size_);

    chunk_size_ /= sizeof(double);
    doubles_a_.clear();
    doubles_a_.reserve(chunk_size_);

    doubles_b_.clear();
    doubles_b_.reserve(chunk_size_);

    doubles_out_.clear();
    doubles_out_.reserve(chunk_size_);

    file_reader_a.read(doubles_a_);
    file_reader_b.read(doubles_b_);

    cursor_a_ = 0;
    cursor_b_ = 0;

    while (doubles_a_.size() || doubles_b_.size())
    {
        if (doubles_a_.size() && doubles_b_.size())
        {
            merge_buffers();
        }
        else if (doubles_a_.size())
        {
            flush_to_output_buffer(doubles_a_, cursor_a_);
        }
        else
        {
            flush_to_output_buffer(doubles_b_, cursor_b_);
        }

        if (doubles_out_.capacity() == doubles_out_.size())
            flush_output_to_file();

        if (cursor_a_ == doubles_a_.size())
        {
            file_reader_a.read(doubles_a_);
            cursor_a_ = 0;
        }

        if (cursor_b_ == doubles_b_.size())
        {
            file_reader_b.read(doubles_b_);
            cursor_b_ = 0;
        }
    }

    flush_output_to_file();
}

void files_merger::flush_to_output_buffer(std::vector<double>& input, size_t& cursor)
{
    auto it_source_beg = input.begin() + cursor;
    size_t available_bytes_in_input = static_cast<size_t>(input.end() - it_source_beg);

    size_t available_bytes_in_output = doubles_out_.capacity() - doubles_out_.size();
    size_t bytes_to_read = std::min(available_bytes_in_input, available_bytes_in_output);

    cursor += bytes_to_read;
    auto it_source_end = it_source_beg + bytes_to_read;
    std::copy(it_source_beg, it_source_end, std::back_inserter(doubles_out_));
}

void files_merger::merge_buffers()
{
    size_t size_a = doubles_a_.size();
    size_t size_b = doubles_b_.size();
    while (cursor_a_ != size_a && cursor_b_ != size_b)
    {
        if (doubles_a_[cursor_a_] < doubles_b_[cursor_b_])
        {
            doubles_out_.emplace_back(doubles_a_[cursor_a_]);
            cursor_a_++;
        }
        else
        {
            doubles_out_.emplace_back(doubles_b_[cursor_b_]);
            cursor_b_++;
        }

        if (doubles_out_.size() == chunk_size_)
            return;
    }
}

void files_merger::flush_output_to_file()
{
    bool truncate = false;
    flush_chunk_to_file(file_out_, doubles_out_, truncate);
    doubles_out_.clear();
}