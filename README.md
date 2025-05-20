# Large file processing

## Tasks
### File Generator
**Description:**  
First task requires generating a large file with single double value per line.

**Challenge:**  
Generating one value per time and writing it to file the same moment is ineffective and generating the whole file in memory requries high usage of RAM.

**Solution:**  
First of all, I decided to generate and write values to the file in chunks. Such approach allowed reduce RAM consuming and decreased execution time from 18 to 17 seconds.
Next, after profiling the application, I found that the most time consuming part of the program was generation of numbers using uniform distribution. 
In order to resolve this issue, I parallelized nubmer generation between threads. This reduced execution time from 17 to 10 seconds.

**Complexity:**  
Time: O(n / number_of_threads), where n - number of doubles.  
Space: O(1) - we use static allocated buffers.  

### Files merger
**Description:**  
This task requires sorting the generated file while not exceeding 100MB of RAM usage.

**Challenge:**  
Application can't load the whole file and sort it in memory and must finish in less than 30 minutes.

**Solution:**  
The application works in two main steps. Firstly, it splits the initial file into smaller sorted files. Secondly, it merges each two files into a single sorted file, until only one file left. The idea is the same as merging step in merge sort algorithm.

**Architecture:**  
*file_sorter* : splits files into sorted subfiles. Uses other classes to merge and produce whole sorted file.  
*files_merger* : merges two files into a single one while maintaing sorted state.  
*double_file_reader* : reads doubles from file in chunks and flushes them into provided buffer.

**Complexity:**  
Time: O(n*log(k)), where n - number of doubles in the initial file, k - number of files after the splitting.  
First we iterate over all the values to split the file (O(n)), then we merge every two files by iterating over double stored in them (O(n)). We repeat last action until all the files are merged (log(k)).  
  
Space: O(1), we only use static buffers the size of which is set in advance.

### Analytics Calculator
**Description:**  
Parse the provided sorted file and calculate mean, standard deviation, median, 01 and 99 percentiles. Mean and standard deviation should be implemented in parallel.

**Challenge**  
Calculation of mean and standard deviation requires finding sum and product of the values. In some cases, such actions may lead to overflow of the double data type.
Unfortunatly, C++ doesn't have its own implementation of large numbers, therefore we either should implement it ourselves or use already ready solutions. I decided to opt for the last option in order to save time for implemetation.
For large numbers I used boost::multiprecision library. 

**Solution:**  
We iterate over the file two times: first to calculate mean and standard deviation, second to calculate percentiles. In order to calcultate mean and standard deviation in parallel we can use [Welford's algorithm]([https://pages.github.com/](https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#:~:text=to%20a%20degree.-,Welford%27s%20online%20algorithm,-%5Bedit%5D)).
This algorithm allows calculating meand and standard deviation at the same time incrementally.

**Complexity:**  
Time: O(n), where n - is the number of doubles in file. We only parse file two times.  
Space: O(1), we only use static buffers.

# How to build
## Prerequisites
**OS:**  Windows.  I assume it may also work on Linux, since solution does not use any Win specific API, however I have not tested it.  
**Tools:**  For building you require Visual Studio 2022, the version I used is 17.13.6. The solution uses C++20 standard.  
**Libraries:**  For last task you need to provide headers for boost::multiprecision. The version of the boost library I used is 1.85.0.

## How to launch
1) Find launch.bat in the solution directory;  
2) Replace value of BOOST_INCLUDE variable with the path to the boost include directory;  
3) Replace value of VS_ROOT variable with the path to the visual studio directory;  
4) Save batch file, execute it.  
  
Alternatively:  
1) Launch .sln file manually;  
2) Go to project settings of AnalyticsCalculator;  
3) Go to C/C++ -> General;  
4) Modify "Additional Include Directories" field to contain path to boost include directory.  


