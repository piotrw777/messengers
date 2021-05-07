#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>


#include "src1.h"
#include "config.h"

extern char message[LENGTH];

double drand(void) {
    double d = rand()/(double) RAND_MAX;
    return d;

}

int random_range(int pocz, int kon)  {
     int k = (kon - pocz + 1) * drand() + pocz;
     return k;
}

bool is_empty(FILE *file)
{
    fseek(file, 0, SEEK_END);
    return (ftell(file) == 0);
}

//maps a set {0, 1, ..51} to letters
char random_letter()
{
    int r = random_range(0, 51);
    if (r < 26)
    {
        return (char) (r + 65);
    }
    else
    {
        return (char) (r + 71);
    }
}

void create_random_message()
{
    int i = 0;

    while (i != LENGTH - 1)
    {
        message[i] = random_letter();
        i++;
    }
    message[LENGTH - 1] = '\0';
}

bool other_instance_running()
{
    unsigned long long k;
    unsigned long long before, after;
    //opening the file
    FILE *file = fopen(COUNTER_FILE, "r");

    if (file == NULL)
    {
        return false;
    }

    if (!fread(&k, sizeof(k), 1, file))
    {
        fprintf(stderr, "Error reading from file\n");
    }
    before = k;
    sleep(3);
    rewind(file);
    if (!fread(&k, sizeof(k), 1, file))
    {
        fprintf(stderr, "Error reading from file\n");
    }
    after = k;
    return (after != before);
}
