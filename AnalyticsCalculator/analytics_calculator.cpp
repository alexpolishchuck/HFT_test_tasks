#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <functional>

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/fwd.hpp>

#include "double_file_reader.h"

using cpp_bin_float_double_t = boost::multiprecision::cpp_bin_float_double_extended;

const uint64_t g_10_megabytes = 10 * 1024 * 1024;

struct percentile_pos
{
    size_t left_pos = 0;
    size_t right_pos = 0;
};

percentile_pos create_percentile_pos(size_t number_of_values, double percentile)
{
    auto left_pos = static_cast<size_t>(std::floor(number_of_values * percentile / 100));
    auto right_pos = static_cast<size_t>(std::ceil(number_of_values * percentile / 100));

    return { left_pos , right_pos };
}

double calculate_percentile(
    double current_value,
    double prev_value,
    percentile_pos pos)
{
    if (pos.left_pos == pos.right_pos)
    {
        auto res = (cpp_bin_float_double_t(current_value) + cpp_bin_float_double_t(prev_value)) / 2;
        return res.convert_to<double>();
    }

    return prev_value;
}

class running_statistics
{
public:
    void run_statistics(const std::string& input_file_name)
    {
        calculate_mean_and_sqrd_sum(input_file_name);
        calculate_percentiles(input_file_name);
    }

    std::string get_formatted_statistics() const
    {
        std::stringstream ss;

        ss << "Mean: " << mean_ << '\n';
        ss << "Standard deviation: " << calculate_standard_deviation() << '\n';
        ss << "Median: " << median_ << '\n';
        ss << "99th percentile: " << percentile_99_ << '\n';
        ss << "1st percentile: " << percentile_01_;

        return ss.str();
    }

private:
    void calculate_mean_and_sqrd_sum(const std::string& input_file_name)
    {
        double_file_reader file_reader(input_file_name, g_10_megabytes);
        std::vector<double> buffer(g_10_megabytes / sizeof(double));
        while (true)
        {
            file_reader.read(buffer);
            if (!buffer.size())
                break;

            element_counter_ += buffer.size();
            calculate_mean_and_sqrd_sum_from_chunk(buffer);
        }
    }

    void calculate_mean_and_sqrd_sum_from_chunk(const std::vector<double>& chunk)
    {
        for (const auto& value : chunk)
        {
            double prev_mean = mean_;

            mean_ += (value - prev_mean) / element_counter_;
            squared_sum_ += cpp_bin_float_double_t(value - prev_mean) * cpp_bin_float_double_t(value - mean_);
        }
    }

    double calculate_standard_deviation() const
    {
        cpp_bin_float_double_t res = boost::multiprecision::sqrt(squared_sum_ / (element_counter_ - 1));
        return res.convert_to<double>();
    }

    double calculate_percentiles(const std::string& input_file_name)
    {
        double_file_reader file_reader(input_file_name, g_10_megabytes);
        std::vector<double> buffer(g_10_megabytes / sizeof(double));

        percentile_pos percentile_01_pos = create_percentile_pos(element_counter_, 1);
        percentile_pos median_pos = create_percentile_pos(element_counter_, 50);
        percentile_pos percentile_99_pos = create_percentile_pos(element_counter_, 99);

        double prev_value = 0;
        int current_file_pos = 0;

        while (true)
        {
            file_reader.read(buffer);
            if (!buffer.size())
                break;

            for (const auto& current_value : buffer)
            {
                current_file_pos++;

                if (current_file_pos == percentile_01_pos.right_pos)
                {
                    percentile_01_ = calculate_percentile(current_value, prev_value, percentile_01_pos);
                }
                else if (current_file_pos == median_pos.right_pos)
                {
                    median_ = calculate_percentile(current_value, prev_value, median_pos);
                }
                else if (current_file_pos == percentile_99_pos.right_pos)
                {
                    percentile_99_ = calculate_percentile(current_value, prev_value, percentile_99_pos);
                }

                prev_value = current_value;
            }
        }

        return 0;
    }

private:
    size_t element_counter_ = 0;
    double mean_ = 0;
    cpp_bin_float_double_t squared_sum_ = 0;
    double median_ = 0;
    double percentile_99_ = 0;
    double percentile_01_ = 0;
};

int main(int argc, char* argv[]) 
{
    std::string input_file_name = "sorted_output.txt";
    std::string output_file_name = "statistics_output.txt";
    if (argc == 3)
    {
        input_file_name = argv[1];
        output_file_name = argv[2];
    }

    try 
    {
        running_statistics statistics;
        statistics.run_statistics(input_file_name);

        std::string formatted_statistics = statistics.get_formatted_statistics();
        std::cout << formatted_statistics;

        std::ofstream output_file(output_file_name, std::ios::binary | std::ios::trunc);
        output_file << formatted_statistics;
    }
    catch (const std::exception& ex)
    {
        std::cout << "main. " << ex.what() << std::endl;
    }

    return 0;
}