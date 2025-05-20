#include <random>
#include <fstream>
#include <sstream>
#include <format>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

const char* g_unsorted_file_name = "unsorted_output.txt";

class random_double_generator
{
public:
    explicit random_double_generator(int number_of_threads)
        : threads_(number_of_threads)
        , double_distribution_(
            std::numeric_limits<double>::min(), 
            std::numeric_limits<double>::max())
        , sign_distribution_(-1, 1)
    {
        std::random_device rd;
        generator_ = std::minstd_rand(rd());
    }

    ~random_double_generator()
    {
        wait_for_finish();
    }

    void generate_random_doubles_file(size_t file_size_bytes, const std::string& filename)
    {
        total_size_ = 0;
        file_size_bytes_ = file_size_bytes;
        output_file_.open(filename, std::ios::trunc | std::ios::binary);
        if (!output_file_.is_open())
            throw std::runtime_error("async_generate_random_doubles_file. Failed to open file.");

        launch_threads();
        wait_for_finish();
    }

private:
    double generate_sign()
    {
        double sign = sign_distribution_(generator_);
        if (sign < 0)
            sign = -1;
        else if (sign > 0)
            sign = 1;

        return sign;
    }

    void wait_for_finish()
    {
        for (auto& thread : threads_)
        {
            if (thread.joinable())
                thread.join();
        }

        output_file_.close();
    }

    void launch_threads()
    {
        auto threads_size = threads_.size();
        for (int i = 0; i < threads_size; i++)
        {
            threads_[i] = std::thread(&random_double_generator::work, this);
        }
    }

    void work()
    {
        int current_batch_size = 0;
        size_t current_batch_string_size = 0;
        int max_batch_size = 0;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            max_batch_size = max_batch_size_;
        }

        std::stringstream ss;
        while (true)
        {
            double sign = generate_sign();
            double rand_double = sign * double_distribution_(generator_);

            std::string rand_double_str = std::format("{:.7e}", rand_double);
            ss << rand_double_str << '\n';
            current_batch_string_size += rand_double_str.size() + 1; // + 1 for '\n'
            current_batch_size++;

            if (current_batch_size == max_batch_size)
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (total_size_ >= file_size_bytes_)
                    return;

                output_file_ << ss.rdbuf();
                std::stringstream().swap(ss);

                total_size_ += current_batch_string_size;
                current_batch_size = 0;
                current_batch_string_size = 0;

                if (total_size_ >= file_size_bytes_)
                    return;
            }
        }
    }

private:
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::ofstream output_file_;
    size_t file_size_bytes_ = 0;
    size_t total_size_ = 0;
    const int max_batch_size_ = 100000;
    std::uniform_real_distribution<double> double_distribution_;
    std::uniform_real_distribution<double> sign_distribution_;
    std::minstd_rand generator_;
};

int main()
{
    try
    {
        int one_gb = 1024 * 1024 * 1024;

        int number_of_threads = std::thread::hardware_concurrency();
        random_double_generator gen(number_of_threads);
        gen.generate_random_doubles_file(one_gb, g_unsorted_file_name);
    }
    catch (const std::exception& ex)
    {
        std::cout << "main. " << ex.what() << std::endl;
    }
}
