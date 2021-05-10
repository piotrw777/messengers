#ifndef THREAD_FUNCTIONS_H
#define THREAD_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>

#include "config.h"

void * check_friend(void * arg);
void * counter(void * arg);
void * send_messages(void *arg);
void * read_messages(void *arg);

enum FRIEND_STATUS {INACTIVE, ACTIVE};

extern int prog_nr;

extern volatile bool friend_status;

extern volatile sig_atomic_t QUIT;

volatile sig_atomic_t PAUSE;

volatile sig_atomic_t EXIT_ALLOWANCE;

extern char message[LENGTH];

extern char buffer[LENGTH];

extern int my_pid;

extern int friend_pid;

extern const char * const COUNTER_FILENAMES[3];

extern const char * const SENT_FILENAMES[3];

extern const char * const RECEIVED_FILENAMES[3];

extern const char * const QUEUED_FILENAMES[3];

extern const char * const FIFO_PATHS[3];

extern FILE * COUNTER_FILE;
extern FILE * SENT_FILE;
extern FILE * RECEIVED_FILE;
extern FILE * QUEUED_FILE;

#endif // THREAD_FUNCTIONS_H
