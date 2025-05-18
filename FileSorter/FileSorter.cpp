#include <sstream>
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <fstream>
#include <filesystem>
#include <format>

const char* g_temp_folder = "temp";

void clear_and_create_folder(const char* folder_path)
{
	if (std::filesystem::exists(folder_path))
		std::filesystem::remove_all(folder_path);

	if (!std::filesystem::create_directory(folder_path))
		throw std::runtime_error("file_sorter::create_output_folder. Failed to create temporary directory");
}

class double_file_reader
{
public:
	explicit double_file_reader(
		const std::string& filename, 
		uint64_t chunk_size_bytes)
		: chunk_size_bytes_(chunk_size_bytes)
		, buffer_(chunk_size_bytes)
	{
		file_.open(filename, std::ios::binary);
		if (!file_.is_open())
			throw std::runtime_error("file_reader: failed to open file");
	}

	void read(std::vector<double>& output)
	{
		file_.read(buffer_.data(), buffer_.size());
		raw_bytes_read_ = file_.gcount();
		if (!raw_bytes_read_)
			return;

		std::streamsize bytes_moved_back = 0;
		if (!check_end_of_line() && !file_.eof())
		{
			bytes_moved_back = move_back_extra_bytes_after_endl();
		}

		raw_bytes_read_ -= bytes_moved_back;
		
		raw_buffer_to_double_buffer(output);
	}

	const std::vector<char> get_buffer() const
	{
		return buffer_;
	}

private:
	bool check_end_of_line() const
	{
		std::streamsize bytes_read = file_.gcount();
		return buffer_[bytes_read - 1] == '\n';
	}

	std::streamsize move_back_extra_bytes_after_endl()
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

		return bytes_moved_back;
	}

	void raw_buffer_to_double_buffer(
		std::vector<double>& output) const
	{
		std::istringstream iss(std::string(buffer_.data(), raw_bytes_read_));
		double value = 0;
		while (true)
		{
			if (output.size() == output.capacity())
			{
				std::stringstream ss;
				ss << "double_file_reader::raw_buffer_to_double_buffer.";
				ss << " Size of output buffer is too small. This may affect performance\n";
				std::cout << ss.str();
			}
			
			if (iss >> value)
				output.emplace_back(value);
			else
				break;
		}
	}

private:
	std::streamsize raw_bytes_read_ = 0;
	std::vector<char> buffer_;
	uint64_t chunk_size_bytes_ = 0;
	std::ifstream file_;
};

class file_sorter
{
public:
	void sort_file(const std::string& unsorted_file_name, const std::string& sorted_file_name)
	{
		unsorted_file_name_ = unsorted_file_name;
		sorted_file_name_ = sorted_file_name;

		clear_and_create_folder(g_temp_folder);
		split_file_by_sorted_chunks(g_temp_folder);
		merge_all_files();
	}

private:
	void split_file_by_sorted_chunks(const std::string folder_path)
	{
		std::ifstream input_file(unsorted_file_name_, std::ios::binary);
		if (!input_file.is_open())
			throw std::runtime_error("file_sorter::split_file_by_sorted_chunks. Failed to open input file");

		uint64_t chunk_size = 100 * 1024 * 1024; // 10 MB
		chunk_size /= sizeof(double);

		std::vector<double> doubles;
		doubles.reserve(chunk_size);

		double_file_reader file_reader(unsorted_file_name_, chunk_size);
		while (true)
		{
			file_reader.read(doubles);
			if (!doubles.size())
				break;

			std::sort(doubles.begin(), doubles.end());

			std::string next_file_name = create_next_file_name(folder_path);
			flush_chunk_to_file(next_file_name, doubles);
			doubles.clear();
		}
	}

	std::string create_next_file_name(const std::string folder_path)
	{
		last_file_id++;

		std::stringstream file_name_stream;
		file_name_stream << folder_path
			<< '/'
			<< last_file_id;

		std::string file_name = file_name_stream.str();
		file_names_.emplace_back(file_name);

		return file_name;
	}

	void flush_chunk_to_file(
		const std::string& filename, 
		const std::vector<double>& data) const
	{
		std::stringstream chunk_stream;
		auto it = data.begin();
		while (it != data.end())
		{
			std::string double_str = std::format("{:.7e}", *it);
			if (it != data.begin())
			{
				chunk_stream << '\n';
			}

			chunk_stream << double_str;
			it++;
		}

		std::ofstream output_file(filename, std::ios::trunc | std::ios::binary);
		std::string output_str = chunk_stream.str();
		output_file.write(output_str.c_str(), output_str.size());
	}

	void merge_all_files()
	{
		while (file_names_.size() > 1)
		{
			std::string file_a = file_names_.front();
			file_names_.pop_front();

			std::string file_b = file_names_.front();
			file_names_.pop_front();

			merge_two_files(file_a, file_b);
		}
	}

	void merge_two_files(const std::string& file_a, const std::string& file_b)
	{
		uint64_t chunk_size = 100 * 1024 * 1024; // 10 MB
		chunk_size /= sizeof(double);
		chunk_size /= 2;

		std::vector<double> doubles;
		doubles.reserve(chunk_size);
		double_file_reader file_reader_a(file_a, chunk_size);
		double_file_reader file_reader_b(file_b, chunk_size);
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