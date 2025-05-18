#include <sstream>
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <fstream>
#include <filesystem>
#include <format>

#include "double_file_reader.h"
#include "consts.h"
#include "utils.h"
#include "files_merger.h"

class file_sorter
{
public:
    void sort_file(const std::string& unsorted_file_name, const std::string& sorted_file_name)
    {
        unsorted_file_name_ = unsorted_file_name;
        sorted_file_name_ = sorted_file_name;

        clear_and_create_folder(g_temp_folder);
        split_file_by_sorted_chunks();
        merge_all_files();
    }

public:
    void split_file_by_sorted_chunks()
    {
        std::ifstream input_file(unsorted_file_name_, std::ios::binary);
        if (!input_file.is_open())
            throw std::runtime_error("file_sorter::split_file_by_sorted_chunks. Failed to open input file");
        
        uint64_t chunk_size = g_max_memory_bytes / 10;
        std::vector<double> doubles;
        doubles.reserve(chunk_size / sizeof(double));
        double_file_reader file_reader(unsorted_file_name_, chunk_size);

        while (true)
        {
            file_reader.read(doubles);
            if (!doubles.size())
                break;

            std::sort(doubles.begin(), doubles.end());

            std::string next_file_name = create_next_file_name();
            flush_chunk_to_file(next_file_name, doubles);
            doubles.clear();
        }
    }

    std::string create_next_file_name()
    {
        last_file_id++;

        std::stringstream file_name_stream;
        file_name_stream << g_temp_folder
            << '/'
            << last_file_id;

        std::string file_name = file_name_stream.str();
        file_names_.emplace_back(file_name);

        return file_name;
    }

    void merge_all_files()
    {
        files_merger merger;

        while (file_names_.size() > 1)
        {
            std::string file_a = file_names_.front();
            file_names_.pop_front();

            std::string file_b = file_names_.front();
            file_names_.pop_front();

            std::string file_out = create_next_file_name();

            merger.merge_two_files(file_a, file_b, file_out);

            std::filesystem::remove(file_a);
            std::filesystem::remove(file_b);
        }
    }

private:
    std::string unsorted_file_name_;
    std::string sorted_file_name_;
    std::deque<std::string> file_names_;
    int last_file_id = 0;
};

int main(int argc, char* argv[])
{
    try
    {
        std::string unsorted_file_name;
        std::string sorted_file_name;

        if (argc < 3)
        {
            unsorted_file_name = "unsorted_output.txt";
            sorted_file_name = "sorted_output.txt";
        }
        else if (argc != 3)
            throw std::runtime_error("Invalid input");

        file_sorter sorter;
        sorter.sort_file(unsorted_file_name, sorted_file_name);
    }
    catch (const std::exception& ex)
    {
        std::cout << "main. " << ex.what() << std::endl;
    }
}