#include "sort.h"

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

void mergeChunks(std::string *fileNames, const char *output_file, int chunks_num, int chunk_size)
{
    int in[chunks_num];
    for (int i = 0; i < chunks_num; i++)
    {
        const char *fileName = fileNames[i].c_str();
        in[i] = open(fileName, O_RDONLY);
    }
 
    int out = open(output_file, O_CREAT | O_WRONLY | O_TRUNC);
 
    MinHeapNode* harr = new MinHeapNode[chunks_num];
    int heap_size, i;
    for (i = 0; i < chunks_num; i++)
    {
        int bytes_read = read(in[i], &harr[i].element, sizeof(harr[i].element));
        if (bytes_read != sizeof(harr[i].element)) {
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
        printf("%d ", root.element);
        write(out, &root.element, sizeof(root.element));
 
        int bytes_read = read(in[root.i], &root.element, sizeof(root.element));
        if (bytes_read != sizeof(root.element)) {
            root.element = INT_MAX;
            count++; // in case of EOF
        }
 
        hp.replaceMin(root); // can switch root.i and input file
    }

    printf("\n");
 
    for (int i = 0; i < chunks_num; i++) {
        close(in[i]);
    }
    close(out);
    delete[] harr;
}

std::string *sortChunks(const char *input_file, int chunks_num, int chunk_size)
{
    int in = open(input_file, O_RDONLY);
 
    int *out = (int *) malloc(chunks_num * sizeof(*out));
    std::string *fileNames = new std::string[chunks_num];
    for (int i = 0; i < chunks_num; i++)
    {
        char fileName[] = "tmp/sort_tmpXXXXXX";
        out[i] = mkstemp(fileName);
        if (out[i] == -1) {
            printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
        }
        fileNames[i] = std::string(fileName);
    }
 
    // array for a single chunk data
    int arr[chunk_size];
 
    bool more_input = true;
    int next_out_index = 0;
 
    while (more_input) {
        // read data of a single chunk
        int bytesize = chunk_size * sizeof(*arr);
        int bytes_read = read(in, arr, bytesize);
        int nums_read = bytes_read / sizeof(*arr);

        if (bytes_read != bytesize) {
            more_input = false;
        }
        if (bytes_read == 0) {
            break;
        }

        // sort a single chunk
        std::sort(arr, arr + chunk_size);

        write(out[next_out_index], arr, nums_read * sizeof(*arr));
        next_out_index++;
    }
 
    for (int i = 0; i < chunks_num; i++) {
        close(out[i]);
    }
 
    close(in);

    free(out);

    return fileNames;
}
 
void externalSort(const char *input_file, const char *output_file, int chunks_num, int chunk_size)
{
    std::string *fileNames = sortChunks(input_file, chunks_num, chunk_size);
    mergeChunks(fileNames, output_file, chunks_num, chunk_size);
    delete[] fileNames;
}