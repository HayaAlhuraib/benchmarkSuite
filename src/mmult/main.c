/* Standard C includes */
#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sched.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* Include all implementations declarations */
#include "impl/naive.h"  // Updated for mmult naive implementation

/* Include common headers */
#include "common/types.h"
#include "common/macros.h"

/* Include application-specific headers */
#include "include/types.h"

const int SIZE_DATA = 256; // Default matrix size (if not provided)

/* Helper function to print a matrix */
void print_matrix(const char* name, float* matrix, size_t size) {
    printf("%s:\n", name);
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            printf("%.2f ", matrix[i * size + j]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char** argv) {
    /* Set the buffer for printf to NULL */
    setbuf(stdout, NULL);

    /* Arguments */
    int nthreads = 1;
    int cpu      = 0;

    int nruns    = 10000;
    int nstdevs  = 3;

    /* Data */
    int data_size = SIZE_DATA; // Holds the size passed to the program

    /* Parse arguments */
    void* (*impl_scalar_naive_ptr)(void* args) = impl_scalar_naive;  // Updated

    /* Chosen */
    void* (*impl)(void* args) = NULL;
    const char* impl_str      = NULL;

    bool help = false;
    for (int i = 1; i < argc; i++) {
        /* Implementations */
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--impl") == 0) {
            assert(++i < argc);
            if (strcmp(argv[i], "naive") == 0) {
                impl = impl_scalar_naive_ptr; impl_str = "scalar_naive";
            } else {
                impl = NULL; impl_str = "unknown";
            }
            continue;
        }

        /* Input/output data size */
        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--size") == 0) {
            assert(++i < argc); // Ensure an argument follows the flag
            data_size = atoi(argv[i]); // Parse size as an integer
            continue;
        }
    }

    if (help || impl == NULL) {
        printf("Usage: %s {-i | --impl} naive [Options]\n", argv[0]);
        printf("  Options:\n");
        printf("    -h | --help      Print this message\n");
        printf("    -s | --size      Size of input matrices (default = %d)\n", SIZE_DATA);
        exit(help ? 0 : 1);
    }

    /* Initialize matrices */
    size_t size = data_size;  // Square matrix dimensions
    srand((unsigned int)time(NULL)); // Seed the random number generator

    float* input = malloc(2 * size * size * sizeof(float)); // Allocate A and B in contiguous memory
    float* R = malloc(size * size * sizeof(float)); // Allocate result matrix

    if (!input || !R) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    /* Set A and B inside input */
    float* A = input;                           // A starts at the beginning
    float* B = input + size * size;             // B starts right after A

    /* Initialize input matrices */
    for (size_t i = 0; i < size * size; i++) {
        A[i] = (float)(rand() % 10);            // Random numbers between 0 and 9
        B[i] = (float)(rand() % 10);            // Random numbers between 0 and 9
    }

    /* Print input matrices */
    print_matrix("Matrix A", A, size);
    print_matrix("Matrix B", B, size);

    /* Prepare arguments */
    args_t args = {
        .input = (void*)input, // Pass A and B as contiguous memory
        .output = (void*)R,
        .size = size,
        .cpu = cpu,
        .nthreads = nthreads
    };

    /* Measure runtime */
    clock_t start = clock(); // Start timer
    (*impl)((void*)&args);   // Run matrix multiplication
    clock_t end = clock();   // End timer

    /* Print result matrix */
    print_matrix("Result Matrix R", R, size);

    /* Print runtime */
    double runtime = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Runtime: %.6f seconds\n", runtime);

    /* Free memory */
    free(input);
    free(R);

    return 0;
}
