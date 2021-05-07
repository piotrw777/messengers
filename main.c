#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#include "src1.h"
#include "config.h"

FILE *file;
int prog_nr;
char message[LENGTH];

void * counter(void * arg)
{
    unsigned long long k = 123;

    //checking if the file exists
    file = fopen(COUNTER_FILE, "rb+");
    if (file == NULL)
    {
        fprintf(stderr, "File %s does not exists\n", COUNTER_FILE);
        printf("Creating the file %s\n", COUNTER_FILE);
        file = fopen(COUNTER_FILE, "wb+");
        if (file == NULL)
        {
            fprintf(stderr, "Error creating the file %s\n", COUNTER_FILE);
        }
        fwrite(&k, sizeof(k), 1, file);
    }
    else if (is_empty(file))
    {
        printf("File %s is empty\n", COUNTER_FILE);
        fwrite(&k, sizeof(k), 1, file);
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
        fwrite(&k, sizeof(k), 1, file);
        printf("Value written: %lld\n", k);
        sleep(1);
    }

    pthread_exit(NULL);
    fclose(file);
}
void * reader(void *arg)
{



}

void * writer(void *arg)
{



}
int main()
{

    if (other_instance_running())
    {
        printf("Another instance is running\n");
        prog_nr = 2;
        printf("Closing program\n");
        return 1;
    }
    prog_nr = 1;
    printf("Create a thread\n");
    pthread_t w;
    pthread_create(&w, NULL, counter, NULL);
    pthread_join(w, NULL);
}
