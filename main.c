#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//#define COUNTER_FILE "/tmp/counter.txt"
#define COUNTER_FILE "/mnt/b/C++/messengers/counter.txt"

void * threadFunction(void * arg)
{
    unsigned long long k;
    char c = 'A';


    //open the file
    FILE *file = fopen(COUNTER_FILE, "r+");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening the file");
        exit(1);
    }
    printf("Created the file %s\n", COUNTER_FILE);

    //checking if the file exists
    if ( !fread(&c, sizeof(c), 1, file) )
    {
        printf("File is empty!!!\n");

    }


    int n = 10;
    while (n--)
    {
        //read current value
        fread(&c, sizeof(c), 1, file);
        //fread(buffer, sizeof(*buffer), 32, file);
        //sprintf(k_string,"%lld", k);
        printf("Value read: %c\n", c);
        //increment value
        c++;

        //write new value
        //fwrite(&c, sizeof(c), 1, file);
        fprintf(file, "%c\n", c);
        printf("Value written: %c\n", c);

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
