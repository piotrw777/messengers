#ifndef SRC1_H
#define SRC1_H

#include <stdbool.h>
#include <stdio.h>
#include "config.h"

double drand(void); 
int random_range(int pocz, int kon);
bool is_empty(FILE *file);
bool file_exists(const char *filename);
char random_letter();
void create_random_message();
bool other_instance_running();
unsigned long long get_timestamp();
void create_timestamp(char *dest, char mode);
int nsleep(long miliseconds);

#endif
