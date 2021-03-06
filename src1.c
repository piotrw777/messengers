#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>

#include "src1.h"
#include "config.h"

extern char message[LENGTH];
extern int prog_nr;

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

bool is_empty(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("is_empty");
        exit(2);
    }
    fseek(file, 0, SEEK_END);
    if(ftell(file) == 0)
    {
        fclose(file);
        return true;
    }
    else
    {
        fclose(file);
        return false;
    }
}

bool file_exists(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }
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
        if (i == 2 || i == 5)
        {
            message[i] = '-';
        }
        else
        {
            message[i] = random_letter();
        }
        i++;
    }
    message[LENGTH - 1] = '\0';
}

//void create_random_message()
//{
//    int i = 0;
//    char c = message[0];
//    if (c == 0)
//    {
//        memset(message, 65, LENGTH - 1);
//    }
//    else
//    {
//        memset(message, (c + 1) % 65 + 65, LENGTH - 1);
//    }

//    message[LENGTH - 1] = '\0';
//}

bool is_friend_running()
{
    int sleep_time = 9e8;
    unsigned long k;
    unsigned long before, after;
    int friend_nr = 3 - prog_nr;
    bool result_determined = false;

    FILE *file;
    while(result_determined == false)
    {
        nsleep(sleep_time);
        sem_wait(sems_counter[friend_nr]);
        //open the counter file of the friend if it's not opened
        file = fopen(COUNTER_FILENAMES[friend_nr], "rb");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[friend_nr]);

            //continue;
            exit(1);
        }
        //read value before
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            //nsleep(sleep_time);
            //continue;
            exit(1);
        }
        fclose(file);
        sem_post(sems_counter[friend_nr]);
        before = k;
        //wait
        nsleep(sleep_time);

        sem_wait(sems_counter[friend_nr]);
        file = fopen(COUNTER_FILENAMES[friend_nr], "rb");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[friend_nr]);
            nsleep(sleep_time);
            continue;
        }
        //read value after
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            //continue;
            exit(1);
        }
        fclose(file);
        sem_post(sems_counter[friend_nr]);
        after = k;

        if (before != after)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

//determine the number of the program
bool other_instance_running(int *prog)
{
    int sleep_time = 5e8;
    unsigned long  k1, k2;
    unsigned long  before1, after1;
    unsigned long  before2, after2;

    while (true)
    {
        //opening the files
        FILE *file1 = fopen(COUNTER_FILENAME1, "r");
        FILE *file2 = fopen(COUNTER_FILENAME2, "r");

        //counter files does not exist
        if (file1 == NULL || file2 == NULL)
        {
            fprintf(stderr, "Error opening counter files\n");
            exit(1);
        }

        //read values
        if (!fread(&k1, sizeof(k1), 1, file1))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            //continue;
            exit(1);
        }

        if (!fread(&k2, sizeof(k2), 1, file2))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            //continue;
            exit(1);
        }

        before1 = k1;
        before2 = k2;

        //wait
        nsleep(sleep_time);

        rewind(file1);
        rewind(file2);

        //read values again
        if (!fread(&k1, sizeof(k1), 1, file1))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            //continue;
            exit(1);
        }

        if (!fread(&k2, sizeof(k2), 1, file2))
        {
            fprintf(stderr, "Error reading from file (other_instance_running)\n");
            //continue;
            exit(1);
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
unsigned long long get_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    unsigned long long microseconds = te.tv_sec * 10000000LL + te.tv_usec * 10 + prog_nr;
    return microseconds;
}

void create_timestamp(char *dest, char mode)
{
    //get current date
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    //get current microsecond
    struct timeval te;
    gettimeofday(&te, NULL);
    long microseconds = te.tv_usec;

    sprintf(dest, "%02d-%02d-%02d,%02d:%02d:%02d,%06ld,%c", tm.tm_mday, tm.tm_mon + 1,tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, microseconds,mode);
}

int nsleep(long nanoseconds)
{
    struct timespec req, rem;

    //   if(miliseconds > 999)
    //   {
    //        req.tv_sec = (int)(miliseconds / 1000);                            /* Must be Non-Negative */
    //        req.tv_nsec = (miliseconds - ((long)req.tv_sec * 1000)) * 1000000; /* Must be in range of 0 to 999999999 */
    //   }
    //   else
    //   {
    //        req.tv_sec = 0;                         /* Must be Non-Negative */
    //        req.tv_nsec = miliseconds * 1000000;    /* Must be in range of 0 to 999999999 */
    //   }
    req.tv_sec = 0;
    req.tv_nsec = nanoseconds;

    return nanosleep(&req , &rem);
}



//#include <fcntl.h>
//#include <errno.h>

//bool CheckForAnotherInstance()
//{
//    int fd;
//    struct flock fl;
//    fd = open("LOCK_FILE_NAME", O_RDWR);
//    if(fd == -1)
//    {
//        return false;
//    }
//    fl.l_type   = F_WRLCK;
//    fl.l_whence = SEEK_SET;
//    fl.l_start  = 0;
//    fl.l_len    = 0;
//    fl.l_pid    = getpid();
//    // try to create a file lock
//    if( fcntl(fd, F_SETLK, &fl) == -1)
//    {
//        // we failed to create a file lock, meaning it's already locked //
//        if( errno == EACCES || errno == EAGAIN)
//        {
//            return true;
//        }
//    }
//    return false;
//}
