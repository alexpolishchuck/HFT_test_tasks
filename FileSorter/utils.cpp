#include "pch.h"
#include "utils.h"

void clear_and_create_folder(const char* folder_path)
{
    if (std::filesystem::exists(folder_path))
        std::filesystem::remove_all(folder_path);

    if (!std::filesystem::create_directory(folder_path))
        throw std::runtime_error("file_sorter::create_output_folder. Failed to create temporary directory");
}

void flush_chunk_to_file(
    const std::string& filename,
    const std::vector<double>& data,
    const size_t chunk_size,
    bool truncate_file)
{
    auto file_flags = std::ios::binary;
    if (truncate_file)
        file_flags |= std::ios::trunc;
    else
        file_flags |= std::ios::app;

    std::ofstream output_file(filename, file_flags);

    std::stringstream chunk_stream;
    auto it = data.begin();
    size_t current_chunk_size = 0;
    
    while (it != data.end())
    {
        std::string double_str = std::format("{:.7e}", *it);
        chunk_stream << double_str;
        chunk_stream << '\n';

        current_chunk_size += double_str.size() + 1; // + 1 for '\n'
        if (current_chunk_size >= chunk_size)
        {
            output_file << chunk_stream.str();
            chunk_stream.str(std::string());
            current_chunk_size = 0;
        }

        it++;
    }

    if(current_chunk_size)
        output_file << chunk_stream.str();
}