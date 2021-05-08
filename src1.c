#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#include "src1.h"
#include "config.h"

extern char message[LENGTH];

double drand(void)
{
    double d = rand()/(double) RAND_MAX;
    return d;
}

int random_range(int pocz, int kon)
{
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

bool other_instance_running(int *prog)
{
    unsigned long  k1, k2;
    unsigned long  before1, after1;
    unsigned long  before2, after2;

    while (true)
    {
        //opening the files
        FILE *file1 = fopen(COUNTER_FILENAME1, "rb");
        FILE *file2 = fopen(COUNTER_FILENAME2, "rb");

        //counter files does not exist
        if (file1 == NULL && file2 == NULL)
        {
            *prog = 1;
            return false;
        }

        //second file does not exists
        if (file2 == NULL)
        {
            if (!fread(&k1, sizeof(k1), 1, file1))
            {
                fprintf(stderr, "Error reading from file (other_instance_running)\n");
                continue;
            }

            before1 = k1;
            sleep(1);
            rewind(file1);

            if (!fread(&k1, sizeof(k1), 1, file1))
            {
                fprintf(stderr, "Error reading from file (other_instance_running)\n");
                continue;
            }

            after1 = k1;
            if (before1 != after1)
            {
                *prog = 2;
                fclose(file1);
                return true;
            }
            *prog = 1;
            fclose(file1);
            return false;
        }

        //first file does not exist
        if (file1 == NULL)
        {
            if (!fread(&k2, sizeof(k2), 1, file2))
            {
                fprintf(stderr, "Error reading from file (other_instance_running)\n");
                continue;
            }

            before2 = k2;
            sleep(1);
            rewind(file2);

            if (!fread(&k2, sizeof(k2), 1, file2))
            {
                fprintf(stderr, "Error reading from file (other_instance_running)\n");
                continue;
            }

            after2 = k2;
            if (before2 != after2)
            {
                *prog = 1;
                fclose(file2);
                return true;
            }
            *prog = 2;
            fclose(file2);
            return false;
        }
        //read values
        if (!fread(&k1, sizeof(k1), 1, file1))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            continue;
        }

        if (!fread(&k2, sizeof(k2), 1, file2))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            continue;
        }

        before1 = k1;
        before2 = k2;

        sleep(1);

        rewind(file1);
        rewind(file2);

        //read values again
        if (!fread(&k1, sizeof(k1), 1, file1))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            continue;
        }

        if (!fread(&k2, sizeof(k2), 1, file2))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            continue;
        }

        after1 = k1;
        after2 = k2;

        if (before1 != after1)
        {
            *prog = 2;
            fclose(file1);
            fclose(file2);
            return true;
        }
        if (before2 != after2)
        {
            *prog = 1;
            fclose(file1);
            fclose(file2);
            return true;
        }
        else
        {
            *prog = 1;
            fclose(file1);
            fclose(file2);
            return false;
        }
    }
}
