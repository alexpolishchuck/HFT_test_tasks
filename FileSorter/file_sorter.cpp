#include "pch.h"
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
        cleanup();
    }

public:
    void split_file_by_sorted_chunks()
    {
        std::ifstream input_file(unsorted_file_name_, std::ios::binary);
        if (!input_file.is_open())
            throw std::runtime_error("file_sorter::split_file_by_sorted_chunks. Failed to open input file");
        
        uint64_t chunk_size = g_max_available_bytes / 3;
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
            flush_chunk_to_file(next_file_name, doubles, chunk_size);
            doubles.clear();
        }
    }

    std::string create_next_file_name()
    {
        last_file_id_++;
        return create_file_name(last_file_id_);
    }

    std::string create_file_name(int file_id) const
    {
        std::stringstream file_name_stream;
        file_name_stream << g_temp_folder
            << '/'
            << file_id;

        std::string file_name = file_name_stream.str();
        return file_name;
    }

    void merge_all_files()
    {
        files_merger merger;

        for(int i = 1; i < last_file_id_; i += 2)
        {
            std::string file_a = create_file_name(i);
            std::string file_b = create_file_name(i + 1);
            std::string file_out = create_next_file_name();

            auto chunk_size = g_max_available_bytes / 2;
            merger.merge_two_files(file_a, file_b, file_out, chunk_size);

            std::filesystem::remove(file_a);
            std::filesystem::remove(file_b);
        }
    }

    void cleanup()
    {
        std::string final_file_path = create_file_name(last_file_id_);
        if (!std::filesystem::exists(final_file_path))
            throw std::runtime_error("file_sorter::cleanup. Final sorted file doesn't exist");

        std::filesystem::rename(final_file_path, sorted_file_name_);

        std::filesystem::remove_all(g_temp_folder);
    }

private:
    std::string unsorted_file_name_;
    std::string sorted_file_name_;
    int last_file_id_ = 0;
};

int main(int argc, char* argv[])
{
    try
    {
        std::string unsorted_file_name = "unsorted_output.txt";
        std::string sorted_file_name = "sorted_output.txt";

        if (argc == 3)
        {
            unsorted_file_name = argv[1];
            sorted_file_name = argv[2];
        }

        file_sorter sorter;
        sorter.sort_file(unsorted_file_name, sorted_file_name);
    }
    catch (const std::exception& ex)
    {
        std::cout << "main. " << ex.what() << std::endl;
    }
}