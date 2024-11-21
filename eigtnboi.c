#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024
#define BATCH_SIZE 10
#define NUM_CONTEXTS 16

// Context structure
typedef struct {
    int value;                  // Current value of the context
    pthread_mutex_t lock;       // Mutex to protect the context
    char* log_buffer[BATCH_SIZE]; // Buffer to batch log lines
    int log_count;              // Number of logs in the buffer
    int op_count;               // Number of operations in the queue
    char operations[200][MAX_BUFFER_SIZE]; // Queue of operations for this context
} Context;

// Global variables
Context contexts[NUM_CONTEXTS];
FILE* log_file;
pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER; // Mutex for the log file

// Function prototypes
int is_prime(int num);
int fib(int num);
double compute_pi(int val);
void log_operation(const char* log_line);
void* process_operations(void* arg);

// Prime number helper function
int is_prime(int num) {
    if (num <= 1) return 0;
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) return 0;
    }
    return 1;
}

// Fibonacci function
int fib(int num) {
    if (num <= 0) return 0;
    if (num == 1) return 1;
    int a = 0, b = 1, temp;
    for (int i = 2; i <= num; i++) {
        temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

// Compute pi using the Leibniz series
double compute_pi(int val) {
    double pi = 0.0;
    for (int i = 0; i < val; i++) {
        if (i % 2 == 0)
            pi += 1.0 / (2 * i + 1);
        else
            pi -= 1.0 / (2 * i + 1);
    }
    return pi * 4;
}

// Log operation with thread safety
void log_operation(const char* log_line) {
    pthread_mutex_lock(&log_lock);
    fprintf(log_file, "%s", log_line);
    fflush(log_file); // Ensure immediate disk write
    pthread_mutex_unlock(&log_lock);
}

// Process operations for a specific context
void* process_operations(void* arg) {
    int ctx_id = *((int*)arg);
    free(arg); // Free dynamically allocated memory for ctx_id
    Context* ctx = &contexts[ctx_id];
    //char buffer[MAX_BUFFER_SIZE];

    for (int i = 0; i < ctx->op_count; i++) {
        pthread_mutex_lock(&ctx->lock);

        char operation[10];
        int value;
        sscanf(ctx->operations[i], "%s %d", operation, &value);

        if (strcmp(operation, "set") == 0) {
            ctx->value = value;
            snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: set to value %d\n", ctx_id, value);
        } else if (strcmp(operation, "add") == 0) {
            ctx->value += value;
            snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: add %d (result: %d)\n", ctx_id, value, ctx->value);
        } else if (strcmp(operation, "sub") == 0) {
            ctx->value -= value;
            snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: sub %d (result: %d)\n", ctx_id, value, ctx->value);
        } else if (strcmp(operation, "mul") == 0) {
            ctx->value *= value;
            snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: mul %d (result: %d)\n", ctx_id, value, ctx->value);
        } else if (strcmp(operation, "div") == 0) {
            if (value != 0) {
                ctx->value /= value;
                snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: div %d (result: %d)\n", ctx_id, value, ctx->value);
            } else {
                snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: div by 0\n", ctx_id);
            }
        } else if (strcmp(operation, "fib") == 0) {
            int result = fib(ctx->value);
            snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: fib (result: %d)\n", ctx_id, result);
        } else if (strcmp(operation, "pia") == 0) {
            double pi = compute_pi(ctx->value);
            snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: pia (result %.15f)\n", ctx_id, pi);
        } else if (strcmp(operation, "pri") == 0) {
            snprintf(ctx->log_buffer[ctx->log_count++], MAX_BUFFER_SIZE, "ctx %02d: primes (result: ", ctx_id);
            for (int j = 2; j <= ctx->value; j++) {
                if (is_prime(j)) {
                    char temp[16];
                    snprintf(temp, sizeof(temp), "%d, ", j);
                    strncat(ctx->log_buffer[ctx->log_count - 1], temp, MAX_BUFFER_SIZE - strlen(ctx->log_buffer[ctx->log_count - 1]) - 1);
                }
            }
            ctx->log_buffer[ctx->log_count - 1][strlen(ctx->log_buffer[ctx->log_count - 1]) - 2] = ')';
            strncat(ctx->log_buffer[ctx->log_count - 1], "", MAX_BUFFER_SIZE - strlen(ctx->log_buffer[ctx->log_count - 1]) - 1);
        }

        if (ctx->log_count == BATCH_SIZE) {
            for (int j = 0; j < BATCH_SIZE; j++) {
                log_operation(ctx->log_buffer[j]);
            }
            ctx->log_count = 0;
        }

        pthread_mutex_unlock(&ctx->lock);
    }

    // Flush remaining logs
    for (int i = 0; i < ctx->log_count; i++) {
        log_operation(ctx->log_buffer[i]);
    }

    ctx->log_count = 0;
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input trace> <output trace>\n", argv[0]);
        return 1;
    }

    FILE* input_file = fopen(argv[1], "r");
    if (!input_file) {
        perror("Error opening input file");
        return 1;
    }

    log_file = fopen(argv[2], "w");
    if (!log_file) {
        perror("Error opening output file");
        fclose(input_file);
        return 1;
    }

    // Initialize contexts
    for (int i = 0; i < NUM_CONTEXTS; i++) {
        contexts[i].value = 0;
        pthread_mutex_init(&contexts[i].lock, NULL);
        contexts[i].log_count = 0;
        contexts[i].op_count = 0;
        for (int j = 0; j < BATCH_SIZE; j++) {
            contexts[i].log_buffer[j] = malloc(MAX_BUFFER_SIZE);
        }
    }

    // Read input file
    char buffer[MAX_BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), input_file)) {
        int ctx_id;
        char operation[10];
        int value = 0;
        sscanf(buffer, "%s %d %d", operation, &ctx_id, &value);
        snprintf(contexts[ctx_id].operations[contexts[ctx_id].op_count++], MAX_BUFFER_SIZE, "%s %d", operation, value);
    }
    fclose(input_file);

    // Create threads
    pthread_t threads[NUM_CONTEXTS];
    for (int i = 0; i < NUM_CONTEXTS; i++) {
        int* ctx_id = malloc(sizeof(int));
        *ctx_id = i;
        pthread_create(&threads[i], NULL, process_operations, ctx_id);
    }

    // Join threads
    for (int i = 0; i < NUM_CONTEXTS; i++) {
        pthread_join(threads[i], NULL);
    }

    fclose(log_file);
    return 0;
}
