#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

int contexts[16]; // contexts 

char** tokenize_input(char* input) {
    char** tokens = NULL;
    char* token = strtok(input, " ");
    int num_tokens = 0;

    while (token != NULL) {
        num_tokens++;
        tokens = realloc(tokens, num_tokens * sizeof(char*));
        tokens[num_tokens - 1] = malloc(strlen(token) + 1);
        strcpy(tokens[num_tokens - 1], token);
        token = strtok(NULL, " ");
    }

    num_tokens++;
    tokens = realloc(tokens, num_tokens * sizeof(char*));
    tokens[num_tokens - 1] = NULL;

    return tokens;
}

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2);
    }
}

int isPrime(int num) {
    if (num <= 1) {
        return 0;
    }
    for (int i = 2; i * i <= num; i++) {
        if (num % i == 0) {
            return 0;
        }
    }
    return 1;
}

int* findPrime(int n, int* counter) {
    int* primes = (int*)malloc(n * sizeof(int));
    for (int i = 2; i <= n; i++) {
        if (isPrime(i)) {
            primes[*counter] = i;
            (*counter)++;
        }
    }
    return primes;
}

double leibniz(int n) {
    double out = 0.0;
    int sign = 1;
    for (int i = 0; i < n; i++) {
        out += sign / (2 * i + 1.0);
        sign *= -1;
    }
    out *= 4;
    return out;
}


int main(int argc, char* argv[]) {
    const char usage[] = "Usage: mathserver.out <input trace> <output trace>\n";
    char* input_trace;
    char* output_trace;
    char buffer[1024];

    // Parse command line arguments
    if (argc != 3) {
        printf("%s", usage);
        return 1;
    }
    
    input_trace = argv[1];
    output_trace = argv[2];

    // Open input and output files
    FILE* input_file = fopen(input_trace, "r");
    if (!input_file) {
        perror("Error opening input file");
        return 1;
    }

    FILE* output_file = fopen(output_trace, "w");

    if (!output_file) {
        perror("Error opening output file");
        fclose(input_file);
        return 1;
    }

    while ( !feof(input_file) ) {
        // Read input file line by line
        char *rez = fgets(buffer, sizeof(buffer), input_file);
        if ( !rez )
            break;
        else {
            // Remove endline character
            buffer[strlen(buffer) - 1] = '\0';
        }
        char** tokens = tokenize_input(buffer);
        // CONTINUE...
        // int test = 1234;
        // fprintf(output_file, "%s\n", tokens[0]);

        if (strcmp(tokens[0], "set") == 0){
            int tempContext = atoi(tokens[1]);
            contexts[tempContext] = atoi(tokens[2]);
            fprintf(output_file, "ctx %.2d: set to value %d\n", tempContext, contexts[tempContext]);
            fflush(output_file);

        } else if (strcmp(tokens[0], "add") == 0){
            int tempContext = atoi(tokens[1]);
            contexts[tempContext] += atoi(tokens[2]);
            fprintf(output_file, "ctx %.2d: add %d (result: %d)\n", tempContext, atoi(tokens[2]), contexts[tempContext]);
            fflush(output_file);

        } else if (strcmp(tokens[0], "sub") == 0){
            int tempContext = atoi(tokens[1]);
            contexts[tempContext] -= atoi(tokens[2]);
            fprintf(output_file, "ctx %.2d: sub %d (result: %d)\n", tempContext, atoi(tokens[2]), contexts[tempContext]);
            fflush(output_file);


        } else if (strcmp(tokens[0], "mul") == 0){
            int tempContext = atoi(tokens[1]);
            contexts[tempContext] *= atoi(tokens[2]);
            fprintf(output_file, "ctx %.2d: mul %d (result: %d)\n", tempContext, atoi(tokens[2]), contexts[tempContext]);
            fflush(output_file);

        } else if (strcmp(tokens[0], "div") == 0){
            int tempContext = atoi(tokens[1]);
            contexts[tempContext] /= atoi(tokens[2]);
            fprintf(output_file, "ctx %.2d: div %d (result: %d)\n", tempContext, atoi(tokens[2]), contexts[tempContext]);
            fflush(output_file);

        } else if (strcmp(tokens[0], "div") == 0){
            int tempContext = atoi(tokens[1]);
            contexts[tempContext] /= atoi(tokens[2]);
            fprintf(output_file, "ctx %.2d: div %d (result: %d)\n", tempContext, atoi(tokens[2]), contexts[tempContext]);
            fflush(output_file);

        } else if (strcmp(tokens[0], "fib") == 0){
            int tempContext = atoi(tokens[1]);
            int tempFib = fibonacci(contexts[tempContext]);
            fprintf(output_file, "ctx %.2d: fib (result: %d)\n", tempContext, tempFib);
            fflush(output_file);

        } else if (strcmp(tokens[0], "pia") == 0){
            int tempContext = atoi(tokens[1]);
            double tempPi = leibniz(contexts[tempContext]);
            fprintf(output_file, "ctx %.2d: pia (result %.15f)\n", tempContext, tempPi);
            fflush(output_file);


        } else if (strcmp(tokens[0], "pri") == 0){
            int tempContext = atoi(tokens[1]);
            int counter = 0;
            int* tempPrime = findPrime(contexts[tempContext], &counter);
            fprintf(output_file, "ctx %.2d: primes (result:", tempContext);
            
            fprintf(output_file, " %d", tempPrime[0]);
            for (int i = 1; i < counter; i++)
            {
                fprintf(output_file, ", %d", tempPrime[i]);
            }
            fprintf(output_file, ")\n");
            fflush(output_file);


            free(tempPrime);
        }

        for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
        free(tokens);
    }
    fclose(input_file);
    fclose(output_file);

    return 0;
}