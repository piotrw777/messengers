#ifndef SRC1_H
#define SRC1_H

#include <stdbool.h>
#include <stdio.h>
#include "config.h"

extern const char * const COUNTER_FILENAMES[3];

double drand(void); 
int random_range(int pocz, int kon);
bool is_empty(const char *filename);
bool file_exists(const char *filename);
char random_letter();
void create_random_message();
bool other_instance_running();
bool is_friend_running();
unsigned long long get_timestamp();
void create_timestamp(char *dest, char mode);
int nsleep(long miliseconds);


#endif
