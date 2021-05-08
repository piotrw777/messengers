#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>

#include "src1.h"
#include "config.h"

const char * const COUNTER_FILENAMES[3] = {
                                    ""
                                    ,COUNTER_FILENAME1
                                    ,COUNTER_FILENAME2
                                    };

const char * const SENT_FILENAMES[3] = {
                                    ""
                                    ,SENT_FILENAME1
                                    ,SENT_FILENAME2
                                    };

const char * const RECEIVED_FILENAMES[3] = {
                                    ""
                                    ,RECEIVED_FILENAME1
                                    ,RECEIVED_FILENAME2
                                    };

const char * const QUEUED_FILENAMES[3] = {
                                    ""
                                    ,QUEUED_FILENAME1
                                    ,QUEUED_FILENAME2
                                    };

FILE * COUNTER_FILES[3];
FILE * SENT_FILES[3];
FILE * RECEIVED_FILES[3];
FILE * QUEUED_FILES[3];

FILE * COUNTER_FILE;
FILE * SENT_FILE;
FILE * RECEIVED_FILE;
FILE * QUEUED_FILE;

enum FRIEND_STATUS {INACTIVE, ACTIVE};

int prog_nr;

static bool friend_status;

static bool QUIT = false;

char message[LENGTH];

void * counter(void * arg)
{
    int sleep_time = 1e5;
    FILE *file = (FILE *) arg;
    unsigned long k = 1;

    int n = 975164;
    while (n--)
    {
        if (is_empty(file))
        {
            fwrite(&k, sizeof(k), 1, file);
            printf("Value written: %lu\n", k);
            usleep(sleep_time);
            k++;
        }

        //read current value
        rewind(file);
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file\n");
        }

        //printf("Value read: %lu from file: \n", k);
        rewind(file);

        //increment value
        k++;

        //write new value
        fwrite(&k, sizeof(k), 1, file);
        //printf("Value written: %lu\n", k);
        usleep(sleep_time);
    }
    QUIT = true;
    pthread_exit(NULL);
    fclose(file);
    return NULL;
}

void * check_friend(void * arg)
{
    unsigned long k;
    unsigned long before, after;
    int friend_nr = 3 - prog_nr;
    bool status = INACTIVE;


    FILE *file;

    while (QUIT == false)
    {
        //open the counter file of the friend if it's not opened
        file = fopen(COUNTER_FILENAMES[friend_nr], "r");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened ",COUNTER_FILENAMES[friend_nr]);
            continue;
        }

        rewind(file);
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            continue;
        }

        before = k;
        sleep(1);

        rewind(file);
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            continue;
        }

        after = k;

        if (before != after && status == INACTIVE)
        {
            printf(GREEN"Other program is running!!!\n"RESET);
            friend_status = status = ACTIVE;
        }
        else if (before == after && status == ACTIVE)
        {
            printf(RED"Other program stopped!!!\n"RESET);
            friend_status = status = INACTIVE;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

//void * read_messages(void *arg)
//{

//    return NULL;


//}

unsigned long long get_timestamp()
{
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    unsigned long long microseconds = te.tv_sec * 10000000LL + te.tv_usec * 10 + prog_nr;
    return microseconds;
}

void * write_messages(void *arg)
{
    static char timestamp_str[TIMESTAMP_LENGTH + 1];

    while (QUIT == false)
    {
        //create string from timestamp
        sprintf(timestamp_str, "%018lli", get_timestamp());

        //generate message
        create_random_message();
        printf(ORANGE"Message generated: %s\n"RESET, message);

        //write entry
        fprintf(SENT_FILE, "%s,%s\n", timestamp_str, message);

        sleep(1);
    }
    pthread_exit(NULL);
    return NULL;
}

void introduce()
{
    printf("I am program nr %d\n", prog_nr);
}

int main()
{
    //determine the number of the program
    if ((friend_status = other_instance_running(&prog_nr)))
    {
        printf("Another instance is running\n");
    }

    introduce();

    //opening the files
    COUNTER_FILE = fopen(COUNTER_FILENAMES[prog_nr], "wb+");
    SENT_FILE = fopen(SENT_FILENAMES[prog_nr], "a");
    RECEIVED_FILE = fopen(RECEIVED_FILENAMES[prog_nr], "ab");
    QUEUED_FILE = fopen(QUEUED_FILENAMES[prog_nr], "ab+");

    //create counting thread
    pthread_t counting_thread;
    pthread_create(&counting_thread, NULL, counter, (void *) COUNTER_FILE);

    //create thread for responding of running or exiting another instance
    pthread_t check_friend_thread;
    pthread_create(&check_friend_thread, NULL, check_friend, (void *) &prog_nr);

    //create sender thread
    pthread_t sender;
    pthread_create(&sender, NULL, write_messages, NULL);

    //joining threads
    pthread_join(counting_thread, NULL);
    pthread_join(check_friend_thread, NULL);
    pthread_join(sender, NULL);
}
