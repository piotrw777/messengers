#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>

#include "src1.h"
#include "thread_functions.h"
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

const char * const FIFO_PATHS[3] = {
                                    ""
                                    ,FIFO_PATH1_2
                                    ,FIFO_PATH2_1
                                    };

FILE * COUNTER_FILES[3];
FILE * SENT_FILES[3];
FILE * RECEIVED_FILES[3];
FILE * QUEUED_FILES[3];

FILE * COUNTER_FILE;
FILE * SENT_FILE;
FILE * RECEIVED_FILE;
FILE * QUEUED_FILE;

int prog_nr;

volatile bool friend_status = false;

volatile sig_atomic_t QUIT = false;

volatile sig_atomic_t PAUSE = false;

volatile sig_atomic_t EXIT_ALLOWANCE_READ = false;

volatile sig_atomic_t EXIT_ALLOWANCE_SEND = false;

char message[LENGTH];
char buffer[LENGTH];

int my_pid;
int friend_pid;

pthread_t counting_thread;
pthread_t check_friend_thread;
pthread_t sender;
pthread_t reader;

void introduce()
{
    printf("I am program nr %d\n", prog_nr);
}

void close_files()
{
    if(COUNTER_FILE != NULL)
    {
        fclose(COUNTER_FILE);
    }
    if(SENT_FILE != NULL)
    {
        fclose(SENT_FILE);
    }
    if(RECEIVED_FILE != NULL)
    {
        fclose(RECEIVED_FILE);
    }
    if(QUEUED_FILE != NULL)
    {
        fclose(QUEUED_FILE);
    }
    printf("Closing the files\n");

}

void make_fifos()
{
    int k1, k2;
    const char *fifo_path1 = FIFO_PATHS[1];
    const char *fifo_path2 = FIFO_PATHS[2];

//    k1 = mkfifo(fifo_path1, 0777);
//    k2 = mkfifo(fifo_path2, 0777);

    k1 = mkfifo(fifo_path1, 0666);
    k2 = mkfifo(fifo_path2, 0666);

    if (k1 == 0)
    {
        printf("Fifo %s created succesfully\n", fifo_path1);
    }
    if (k2 == 0)
    {
        printf("Fifo %s created succesfully\n", fifo_path2);
    }
}

void open_files()
{
    FILE * COUNTER_FILE;
    FILE * SENT_FILE;
    FILE * RECEIVED_FILE;
    FILE * QUEUED_FILE;

    const char header[] = "date,time,microsecond,mode,message\n";

    COUNTER_FILE = fopen(COUNTER_FILENAMES[prog_nr], "wb+");

    SENT_FILE = fopen(SENT_FILENAMES[prog_nr], "w");
    fprintf(SENT_FILE, "date,time,microsecond,mode,message\n");
    fclose(SENT_FILE);

    RECEIVED_FILE = fopen(RECEIVED_FILENAMES[prog_nr], "w");
    fprintf(RECEIVED_FILE,"date,time,microsecond,mode,message\n");
    fclose(RECEIVED_FILE);

    QUEUED_FILE = fopen(QUEUED_FILENAMES[prog_nr], "w");
    fprintf(QUEUED_FILE, "date,time,microsecond,mode,message\n");
    fclose(QUEUED_FILE);
}

void create_threads()
{
    pthread_create(&counting_thread, NULL, counter, (void *) COUNTER_FILE);
    pthread_create(&check_friend_thread, NULL, check_friend, NULL);
    pthread_create(&sender, NULL, send_messages, NULL);
    pthread_create(&reader, NULL, read_messages, NULL);
}

void join_threads()
{
    pthread_join(counting_thread, NULL);
    pthread_join(check_friend_thread, NULL);
    pthread_join(sender, NULL);
    pthread_join(reader, NULL);
}

void breakHandler(int sig)
{
    signal(sig, SIG_IGN);
    if (friend_status == ACTIVE)
    {
        kill(friend_pid, SIGUSR1);
    }
    else
    {
        QUIT = true;
    }
}

void pause_handler(int sig)
{
    PAUSE = true;
}

void quit_handler(int sig)
{
    QUIT = true;
}

int main()
{
    my_pid = getpid();
    printf("My pid is: %d\n", my_pid);

    //handling Ctrl+C quit
    signal(SIGINT, breakHandler);

    signal(SIGUSR1, pause_handler);

    signal(SIGUSR2, quit_handler);

    signal(SIGPIPE, SIG_IGN);
    srand(time(0));

    //determine the number of the program
    if ((friend_status = other_instance_running(&prog_nr)))
    {
        printf("Another instance is running\n");
        make_fifos();
    }

    introduce();
    //open_files();
    create_threads();
    join_threads();
    close_files();
    printf(BLUE"\nQuiting the program...\n"RESET);
}
