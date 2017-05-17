#include <string>
#include <fcntl.h>
#include <algorithm>
#include <climits>
#include <errno.h>
#include <string.h>
#include <unistd.h>

void mergeChunks(std::string *fileNames, const char *output_file, int chunks_num, int chunk_size);

std::string *sortChunks(const char *input_file, int chunks_num, int chunk_size);

void externalSort(const char *input_file, const char *output_file, int chunks_num, int chunk_size);
