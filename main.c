#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

//#define COUNTER_FILE "/tmp/counter.txt"
#define COUNTER_FILE "./counter.txt"

bool is_empty(FILE *file)
{
    fseek(file, 0, SEEK_END);
    return (ftell(file) == 0);
}

void * threadFunction(void * arg)
{
    unsigned long long k = 0;

    //checking if the file exists
    FILE *file = fopen(COUNTER_FILE, "r+");
    if (file == NULL)
    {
        fprintf(stderr, "File %s does not exists\n", COUNTER_FILE);
        printf("Creating the file %s\n", COUNTER_FILE);
        file = fopen(COUNTER_FILE, "w+");
        fprintf(file, "%lld\n", k);
        if (file == NULL)
        {
            fprintf(stderr, "Error creating the file %s\n", COUNTER_FILE);
        }
    }
    else if (is_empty(file))
    {
        printf("File %s is empty\n", COUNTER_FILE);
        fprintf(file, "%lld", k);
        rewind(file);
    }
    int n = 10;
    while (n--)
    {
        //read current value
        rewind(file);
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file\n");
        }

        printf("Value read: %lld\n", k);
        rewind(file);

        //increment value
        k++;

        //write new value
        fprintf(file, "%lld", k);
        printf("Value written: %lld\n", k);
        sleep(1);
    }

    pthread_exit(NULL);
    fclose(file);

}

int main()
{
    int start = 5;
    printf("Create a thread\n");
    pthread_t w;
    pthread_create(&w, NULL, threadFunction, (void *) &start);
    pthread_join(w, NULL);
}
