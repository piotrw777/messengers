#ifndef SRC1_H
#define SRC1_H

#include <stdbool.h>
#include <stdio.h>

double drand(void); 
int random_range(int pocz, int kon);
bool is_empty(FILE *file);
char random_letter();
void create_random_message();
bool other_instance_running();

#endif