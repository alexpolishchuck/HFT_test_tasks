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
    bool truncate_file)
{
    std::stringstream chunk_stream;
    auto it = data.begin();
    while (it != data.end())
    {
        std::string double_str = std::format("{:.7e}", *it);
        chunk_stream << double_str;
        chunk_stream << '\n';
        it++;
    }

    auto file_flags = std::ios::binary;
    if (truncate_file)
        file_flags |= std::ios::trunc;
    else 
        file_flags |= std::ios::app;

    std::ofstream output_file(filename, file_flags);
    std::string output_str = chunk_stream.str();
    output_file.write(output_str.c_str(), output_str.size());
}