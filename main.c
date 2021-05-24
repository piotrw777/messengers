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

#include <errno.h>
#include <fcntl.h>

#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

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

int prog_nr;

volatile bool friend_status = false;

volatile sig_atomic_t QUIT = false;

volatile sig_atomic_t PAUSE_SENDING = false;

volatile sig_atomic_t QUIT_SENDING = false;

volatile sig_atomic_t EXIT_ALLOWANCE_READ = false;

volatile sig_atomic_t EXIT_ALLOWANCE_SEND = false;


char message[LENGTH];
char buffer[LENGTH];

static const char semname_quit[] = "semfile";
static const char *semnames_counter[3] = {
                                        ""
                                        ,"semfile_counter1"
                                        ,"semfile_counter2"
                                        };

sem_t *sem_quit;
sem_t *sems_counter[3];

int my_pid;
int friend_pid;

static bool i_am_last;
static bool semaphores_created;

pthread_t counting_thread;
pthread_t check_friend_thread;
pthread_t sender;
pthread_t reader;

void introduce()
{
    printf("I am program nr %d\n", prog_nr);
}

void make_fifos()
{
    int k1, k2;
    const char *fifo_path1 = FIFO_PATHS[1];
    const char *fifo_path2 = FIFO_PATHS[2];

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

void create_counter_files()
{
    FILE *file1, *file2;
    unsigned long k = 0;
    //create a file if it does not exit
    if (!file_exists(COUNTER_FILENAMES[1]))
    {
        file1 = fopen(COUNTER_FILENAMES[1], "wb");
        fwrite(&k, sizeof(k), 1, file1);
        fclose(file1);
    }

    if (!file_exists(COUNTER_FILENAMES[2]))
    {
        file2 = fopen(COUNTER_FILENAMES[2], "wb");
        fwrite(&k, sizeof(k), 1, file2);
        fclose(file2);
    }
}

void make_headers()
{
    FILE * SENT_FILE;
    FILE * RECEIVED_FILE;
    FILE * QUEUED_FILE;

    const char header[] = "date,time,microsecond,mode,message\n";

    if (!file_exists(SENT_FILENAMES[prog_nr]))
    {
        SENT_FILE = fopen(SENT_FILENAMES[prog_nr], "w");
        fprintf(SENT_FILE, header);
        fclose(SENT_FILE);
    }

    if (!file_exists(RECEIVED_FILENAMES[prog_nr]))
    {
        RECEIVED_FILE = fopen(RECEIVED_FILENAMES[prog_nr], "w");
        fprintf(RECEIVED_FILE, header);
        fclose(RECEIVED_FILE);
    }

    if (!file_exists(QUEUED_FILENAMES[prog_nr]))
    {
        QUEUED_FILE = fopen(QUEUED_FILENAMES[prog_nr], "w");
        fclose(QUEUED_FILE);
    }
}

void create_threads()
{
    pthread_create(&counting_thread, NULL, counter, NULL);
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
void breakHandler(int);
void breakHandler(int sig)
{
    //prevent from exiting too early
    if(semaphores_created == false)
    {
        printf("Not ready for exiting...\n");
        signal(sig, breakHandler);
        return;
    }
    signal(sig, SIG_IGN);
    int sem_val;
    sem_getvalue(sem_quit, &sem_val);
    if (sem_val == 0)
    {
        printf("Don't be so quick!\n");
    }
    sem_wait(sem_quit);

    if (is_friend_running())
    {
        i_am_last = false;
        printf("I am not last\n");
        kill(friend_pid, SIGUSR1);
        QUIT_SENDING = true;
    }
    else
    {
        printf("I'm  last\n");
        i_am_last = true;
        QUIT = true;
    }
}

void pause_handler(int sig)
{
    signal(sig, SIG_IGN);
    PAUSE_SENDING = true;
}

void quit_handler(int sig)
{
    signal(sig, SIG_IGN);
    QUIT = true;
}

//create semaphores if you are program1
void create_semaphores1()
{
    //create quit semaphore
    sem_unlink(semname_quit);
    sem_quit = sem_open(semname_quit, O_CREAT | O_EXCL, S_IRUSR|S_IWUSR, 1);
    if (sem_quit == SEM_FAILED)
    {
        perror("sem_open error");
        exit(1);
    }

    //create counter semaphores
    for (int i = 1; i < 3; i++)
    {
        sem_unlink(semnames_counter[i]);
        sems_counter[i] = sem_open(semnames_counter[i], O_CREAT | O_EXCL, S_IRUSR|S_IWUSR, 1);
        if (sems_counter[i] == SEM_FAILED)
        {
            perror("sem_open error");
            exit(1);
        }
    }
}

//create semaphores if you are program2
void create_semaphores2()
{
    //open quit procedure sem
    sem_quit = sem_open(semname_quit, 0);
    if (sem_quit == SEM_FAILED)
    {
        perror("sem_open error: ");
        exit(1);
    }

    //open counter semaphores
    for (int i = 1; i < 3; i++)
    {
        sems_counter[i] = sem_open(semnames_counter[i], 0);
        if (sems_counter[i] == SEM_FAILED)
        {
            perror("sem_open error: ");
            exit(1);
        }
    }
}

void sem_cleanup()
{
    if(i_am_last == false)
    {
        //close the semaphores
        sem_post(sem_quit);
        if (sem_close(sem_quit) == -1)
        {
            perror("sem close");
        }
        for (int i = 1; i < 3; i++)
        {
            if (sem_close(sems_counter[i]) == -1)
            {
                perror("sem_counter close");
            }
        }
    }
    else
    {
       sem_post(sem_quit);
       sem_destroy(sem_quit);
       sem_destroy(sems_counter[1]);
       sem_destroy(sems_counter[2]);
    }
}

int main()
{
    create_counter_files();
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
        //i am second - open existing semaphores
        create_semaphores2();
    }
    else
    {
        //i am first - create semaphores
        create_semaphores1();
    }
    semaphores_created = true;

    introduce();
    make_headers();
    create_threads();
    join_threads();
    printf(BLUE"Quiting the program...\n"RESET);
    sem_cleanup();
}

