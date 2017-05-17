#include "sort.h"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <fcntl.h>

void genData(const char *input_file, int how_many) {
    srand(time(NULL));

    int input = open(input_file, O_CREAT | O_WRONLY | O_TRUNC); //0644
 
    for (int i = 0; i < how_many; i++) {
        int num = rand();
        write(input, &num, sizeof(num));
    }
    close(input);
}

char* getCmdOption(char **begin, char **end, const std::string &option)
{
    char ** itr = find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return nullptr;
}

bool cmdOptionExists(char **begin, char **end, const std::string &option)
{
    return find(begin, end, option) != end;
}

int castInt(char *char_array) {
    std::string str = std::string(char_array);
    std::stringstream stream(str);
    int output;
    stream >> output;
    if (!stream) {
        return -1;
    } else {
        return output;
    }
}
 
int main(int argc, char **argv)
{
    char *cArg = getCmdOption(argv, argv + argc, "-c");
    char *sArg = getCmdOption(argv, argv + argc, "-s");

    if (cArg == nullptr || sArg == nullptr) {
        std::cout << "\
parameters: -s chunks_num -c chunk_size [-g]\n\
\
chunks_num and chunk_size are positive integers\n\
max chunks_num is 9999\n\
chunks_size * chunks_num should be equal to the size of the input vector\n\
-g to generate input.txt" << std::endl;
        exit(0);
    }

    int chunks_num = castInt(cArg);
    int chunk_size = castInt(sArg);

    if (chunks_num <= 0 || chunk_size <= 0 || chunks_num > 9999) {
        std::cout << "invalid arguments" << std::endl;
        exit(0);
    }

    const char *input_file = "input.txt";
    const char *output_file = "output.txt";

    if (cmdOptionExists(argv, argv + argc, "-g")) {
        genData(input_file, chunks_num * chunk_size);
        std::cout << "generated data written to input.txt" << std::endl;
    }

    std::cout << "chunks_num: " << cArg << ", chunk_size: " << sArg << std::endl;
 
    externalSort(input_file, output_file, chunks_num, chunk_size);
}