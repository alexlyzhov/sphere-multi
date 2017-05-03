#include <cstdlib>
#include <iostream>
#include <climits>
#include <algorithm>
#include <sstream>

using namespace std;
 
struct MinHeapNode
{
    int element, i;
};
 
void swap(MinHeapNode* x, MinHeapNode* y);
 
class MinHeap
{
    MinHeapNode* harr;
    int heap_size;
 
public:
    MinHeap(MinHeapNode a[], int size);
 
    void MinHeapify(int);
 
    int left(int i) {
        return (2 * i + 1);
    }
 
    int right(int i) {
        return (2 * i + 2);
    }
 
    MinHeapNode getMin() {
        return harr[0];
    }
 
    void replaceMin(MinHeapNode x) {
        harr[0] = x;
        MinHeapify(0);
    }
};
 
MinHeap::MinHeap(MinHeapNode a[], int size)
{
    heap_size = size;
    harr = a;
    int i = (heap_size - 1) / 2;
    while (i >= 0) { // traverses all subtrees
        MinHeapify(i);
        i--;
    }
}

void MinHeap::MinHeapify(int i)
{
    int l = left(i);
    int r = right(i);
    int smallest = i;
    if (l < heap_size && harr[l].element < harr[i].element) {
        smallest = l; // left child is smaller
    }
    if (r < heap_size && harr[r].element < harr[smallest].element) {
        smallest = r; // right child is smaller
    }
    if (smallest != i) {
        swap(&harr[i], &harr[smallest]);
        MinHeapify(smallest);
    }
}
 
void swap(MinHeapNode* x, MinHeapNode* y)
{
    MinHeapNode temp = *x;
    *x = *y;
    *y = temp;
}
 
void merge(int arr[], int l, int m, int r) { // merge arr[1..m] and arr[m+1..r]
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;
 
    int left_tmp[n1], right_tmp[n2];
 
    // copy data to temp arrays

    for(i = 0; i < n1; i++) {
        left_tmp[i] = arr[l + i];
    }

    for(j = 0; j < n2; j++) {
        right_tmp[j] = arr[m + 1 + j];
    }
 
    // merge temp arrays back into arr[l..r]
    i = 0; // index of first subarray
    j = 0; // index of second subarray
    k = l; // index of merged subarray
    while (i < n1 && j < n2) {
        if (left_tmp[i] <= right_tmp[j]) {
            arr[k++] = left_tmp[i++];
        } else {
            arr[k++] = right_tmp[j++];
        }
    }

    // copy possible remaining elements

    while (i < n1) {
        arr[k++] = left_tmp[i++];
    }

    while (j < n2) {
        arr[k++] = right_tmp[j++];
    }
}

void mergeSort(int arr[], int l, int r)
{
    if (l < r)
    {
        int m = l + (r - l) / 2; // (l + r) / 2, avoids overflow
 
        // Sort halves first recursively
        mergeSort(arr, l, m);
        mergeSort(arr, m + 1, r);
 
        merge(arr, l, m, r);
    }
}
 
FILE* openFile(const char* fileName, char* mode)
{
    FILE* fp = fopen(fileName, mode);
    if (fp == NULL)
    {
        perror("Error while opening a file.\n");
        exit(1);
    }
    return fp;
}

void mergeChunks(const char *output_file, int chunks_num, int chunk_size)
{
    FILE* in[chunks_num]; // open all chunks first
    for (int i = 0; i < chunks_num; i++)
    {
        char fileName[10];
        snprintf(fileName, sizeof(fileName), "%d.dat", i);
        in[i] = openFile(fileName, "r");
    }
 
    FILE *out = openFile(output_file, "w");
 
    MinHeapNode* harr = new MinHeapNode[chunks_num];
    int heap_size, i;
    for (i = 0; i < chunks_num; i++)
    {
        if (fscanf(in[i], "%d ", &harr[i].element) != 1) { // if read fails, break => i = actual num of chunks
            break;
        }
 
        harr[i].i = i; // set file index inside minheap element structure
    }
    heap_size = i;
    MinHeap hp(harr, heap_size); // new minheap with number of nodes = number of input files
 
    int count = 0;
 
    while (count != heap_size)
    {
        MinHeapNode root = hp.getMin(); // write min from minheap
        fprintf(out, "%d ", root.element);
 
        if (fscanf(in[root.i], "%d ", &root.element) != 1) { // take next element from the same input file
            root.element = INT_MAX;
            count++; // in case of EOF
        }
 
        hp.replaceMin(root); // can switch root.i and input file
    }
 
    for (int i = 0; i < chunks_num; i++)
        fclose(in[i]);
 
    fclose(out);
}

void sortChunks(const char *input_file, int chunks_num, int chunk_size)
{
    FILE *in = openFile(input_file, "r");
 
    FILE* out[chunks_num]; // open chunk files here
    char fileName[10];
    for (int i = 0; i < chunks_num; i++)
    {
        snprintf(fileName, sizeof(fileName), "%d.dat", i);
        out[i] = openFile(fileName, "w");
    }
 
    // array for a single chunk data
    int* arr = (int*)malloc(chunk_size * sizeof(int));
 
    bool more_input = true;
    int next_out_index = 0;
 
    int i;
    while (more_input) {
        // read data of a single chunk
        for (i = 0; i < chunk_size; i++) {
            if (fscanf(in, "%d ", &arr[i]) != 1) {
                more_input = false;
                break;
            }
        }

        // sort a single chunk
        mergeSort(arr, 0, i - 1);
 
        for (int j = 0; j < i; j++) {
            fprintf(out[next_out_index], "%d ", arr[j]);
        }
 
        next_out_index++;
    }
 
    for (int i = 0; i < chunks_num; i++) {
        fclose(out[i]);
    }
 
    fclose(in);
}
 
void externalSort(const char *input_file, const char *output_file, int chunks_num, int chunk_size)
{
    sortChunks(input_file, chunks_num, chunk_size);
    mergeChunks(output_file, chunks_num, chunk_size);
}

void genData(const char *input_file, int how_many) {
    srand(time(NULL));

    FILE* input = openFile(input_file, "w");
 
    for (int i = 0; i < how_many; i++) {
        fprintf(input, "%d ", rand());
    }
    fclose(input);
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
    string str = string(char_array);
    stringstream stream(str);
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
        cout << "\
parameters: -s chunks_num -c chunk_size [-g]\n\
\
chunks_num and chunk_size are positive integers\n\
max chunks_num is 9999\n\
chunks_size * chunks_num should be equal to the size of the input vector\n\
-g to generate input.txt" << endl;
        exit(0);
    }

    int chunks_num = castInt(cArg);
    int chunk_size = castInt(sArg);

    if (chunks_num <= 0 || chunk_size <= 0 || chunks_num > 9999) {
        cout << "invalid arguments" << endl;
        exit(0);
    }

    const char *input_file = "input.txt";
    const char *output_file = "output.txt";

    if (cmdOptionExists(argv, argv + argc, "-g")) {
        genData(input_file, chunks_num * chunk_size);
        cout << "generated data written to input.txt" << endl;
    }

    cout << "chunks_num: " << cArg << ", chunk_size: " << sArg << endl;
 
    externalSort(input_file, output_file, chunks_num, chunk_size);
}