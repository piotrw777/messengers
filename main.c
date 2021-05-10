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

volatile bool friend_status;

sig_atomic_t QUIT = false;

char message[LENGTH];
char buffer[LENGTH];

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

}

void make_fifos()
{
    int k1, k2;
    const char *fifo_path1 = FIFO_PATHS[1];
    const char *fifo_path2 = FIFO_PATHS[2];

    k1 = mkfifo(fifo_path1, 0777);
    k2 = mkfifo(fifo_path2, 0777);

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
    COUNTER_FILE = fopen(COUNTER_FILENAMES[prog_nr], "wb+");
    SENT_FILE = fopen(SENT_FILENAMES[prog_nr], "a");
    RECEIVED_FILE = fopen(RECEIVED_FILENAMES[prog_nr], "ab");
    QUEUED_FILE = fopen(QUEUED_FILENAMES[prog_nr], "ab+");
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
int break_counter;
void breakHandler(int sig)
{
    break_counter++;
    write(STDOUT_FILENO,"You broke the program %d times",40);
//    int tmp;

//    printf(BLUE"\nQuiting the program...\n"RESET);

//    QUIT = true;
//    join_threads();
//    if (other_instance_running(&tmp))
//    {
//        unlink(FIFO_PATH1_2);
//        unlink(FIFO_PATH2_1);
//    }
//    //close_files();
//    printf(BLUE"Goodbuy cruel world...\n"RESET);
//    exit(0);
}

/*
 *   mkfifo(argv[1], S_IRWXU | S_IRWXG | S_IRWXO);
  int fifo = open(argv[1], O_RDONLY);

  /* Duplicate the file 'fifo', so that file descriptor 0 points to it.
   * Note that 0 is the file descriptor of stdin.
  dup2(fifo, 0);

  char line[1024];
  int i = 0;
  printf("Reading File %s\n", argv[1]);
  while(fgets(line, 1024, stdin))
    printf("%3d| %s", i++, line);
  printf("\n");
 */

int main()
{

    //handling Ctrl+C quit
    signal(SIGINT, breakHandler);
    srand(time(0));
    //determine the number of the program
//    if ((friend_status = other_instance_running(&prog_nr)))
//    {
//        printf("Another instance is running\n");
//        make_fifos();
//    }

    introduce();
    //open_files();
    //create_threads();
    while (1)
    {
        printf("I'm here!!!\n");
        sleep(1);
    }
    join_threads();
}
