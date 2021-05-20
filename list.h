#ifndef LIST_H
#define LIST_H

#define check_access 1
#define ull unsigned long long

typedef struct node {
    struct node * next;
    char *elem;
} node;

typedef struct List {
    node * head;  //wskaźnik na początek listy
    node * tail;  //wskaźnik na koniec listy
    unsigned long long length; //ilość elementów
} List;

List * create_list(void);
bool append_to_list(List * list, char *str);
void clear_list(List * list);
void print_list(List * list);
void reverse_list(List * list);
void pop (List *list);
int is_list_empty(List *list);
void destroy_list(List ** list);

#endif // LIST_H
