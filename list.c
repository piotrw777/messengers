#include<stdio.h>
#include<stdlib.h>
#include "list.h"
#include <string.h>
#include <stdbool.h>

int size_of_node = sizeof(node);

//zadane:
List * create_list(void) {
    List * nowa_lista = malloc( sizeof( List ) );
    nowa_lista -> head = NULL;
    nowa_lista -> tail = NULL;
    nowa_lista -> length = 0;
    return nowa_lista;
}

int append_to_list(List * list, char *str) {
    //jeśli lista jest pusta
    if( list -> head == NULL ) {
        list -> head = malloc( size_of_node );
        list -> tail = list -> head;
        list -> head -> next = NULL;

        //allocating memory for the string
        char *tmp;
        tmp = malloc(strlen(str) + 1);
        if (tmp == NULL)
        {
            fprintf(stderr, "Memory error\n");
            return -1;
        }
        strcpy(tmp, str);
        list -> head -> elem = tmp;
        list -> length = 1;
    } else {
        node * wsk_node = list -> tail;
        wsk_node -> next = malloc( size_of_node );
        wsk_node -> next -> next = NULL;
        //allocating memory for the string
        char *tmp;
        tmp = malloc(strlen(str) + 1);
        if (tmp == NULL)
        {
            fprintf(stderr, "Memory error\n");
            return -1;
        }
        strcpy(tmp, str);
        wsk_node -> next -> elem = tmp;
        list -> tail = wsk_node -> next;
        (list -> length)++;
    }
    return 0;
}

void clear_list(List * list) {
    node * wsk_node_1 = list -> head;
    node * wsk_node_2;

    while(wsk_node_1 != NULL) {
        wsk_node_2 = wsk_node_1 -> next;
        free(wsk_node_1->elem);
        free(wsk_node_1);
        wsk_node_1 = wsk_node_2;
    }
    list -> head = NULL;
    list -> tail = NULL;
    list -> length = 0;
}

void destroy_list(List ** list) {
    clear_list(*list);
    free((*list) -> head);
    free((*list) -> tail);
    free(*list);
    *list = NULL;
}

void print_list(List * list) {
    if(list->head == NULL) {
        printf("Lista jest pusta\n");
        return;
    }
    int k = 0;
    node * wsk_node = list -> head;

    while( wsk_node != NULL ) {
        printf( "Element nr %d: %s\n", k, wsk_node->elem );
        wsk_node = wsk_node->next;
        k++;
    }
}
void present_list(List * list) {
    printf("Lista ma %lld elementów\n",list->length);
    print_list(list);

}

char * get_nth_element(List * list, int index) {
#ifdef check_access
    if(list->head == NULL) {
        perror("Proba dostepu do elementu w pustej liscie\nBlad w funkcji get_nth_element");
        exit(1);
    }

    if( (size_t) index >= (list -> length) ) {
        perror("Proba dostepu do nieistniejacego elementu listy\nBlad w funkcji get_nth_element");
        exit(1);
    }

#endif // check_access
    int k = 0;
    node * wsk_node = list -> head;

    while(k < index) {
        k++;
        wsk_node = wsk_node->next;
    };
    return wsk_node -> elem;
} // end of get_nth_element

void reverse_list(List * list) {
    if(list->length <= (size_t) 1) return;

    node * wsk_node1 = NULL;
    node * wsk_node2 = list->head;
    node * wsk_node3 = list->head->next;

    list->tail = list->head;

    while(wsk_node3 != NULL) {
        //odwrócenie strzałki
        wsk_node2->next = wsk_node1;
        //idziemy dalej
        wsk_node1 = wsk_node2;
        wsk_node2 = wsk_node3;
        wsk_node3 = wsk_node3 -> next;
    }
    list->head = wsk_node2;
    wsk_node2->next = wsk_node1;
}



//void insert_to_list(List * list, int elem, int index) {
//#ifdef check_access
//    if(index < 0 || index > list->length) {
//        perror("Proba dodania elementu listy na niewlasciwej pozycji\nBlad w funkcji insert_to_list");
//        exit(1);
//    }
//#endif
//    //dodanie nowego na końcu
//    if(index == list->length) {
//        append_to_list(list, elem);
//        return;
//    }
//    //dodajemy na początku
//    if(index == 0) {
//        node *wsk_node = list->head;
//        list->head = malloc(size_of_node);
//        list->head ->next = wsk_node;
//        list->head->elem=elem;
//        list->length++;
//        return;
//    }
//    if(index < list->length) {
//        node * wsk_node = list->head;
//        node * wsk_node_2;
//        int k = index - 1;
//        //idziemy do miejsca wstawiania
//        while(k--) wsk_node = wsk_node->next;
//        wsk_node_2 = wsk_node->next;
//        //tworzenie nowego elementu
//        wsk_node->next = malloc(size_of_node);
//        wsk_node->next->next=wsk_node_2;
//        wsk_node->next->elem=elem;
//        list->length++;
//        return;
//    }

//} //end insert_to_list

void remove_nth_element(List * list, int index)
{
    //usuwanie elementu o indeksie 0
    if(index == 0)
    {
        //jeśli jest tylko jeden element
        if(list->length == 1)
        {
            free (list->head->elem);
            free (list->head);
            list->head = NULL;
            list->tail = NULL;
        }
        else
        {
            node * wsk_node = list->head->next;
            free (list->head->elem);
            free(list->head);
            list->head = wsk_node;
        }
        list->length--;
        return;
    }
#ifdef check_access
    if(index < 0 || (size_t) index >= list->length)
    {
        perror("Proba usuniecia elementu o nieistniejacym indeksie.\bBlad w funkcji remove_nth_element");
        exit(1);
    }
#endif
    //elementy o indeksie >= 1
    int k = index - 1;
    node * wsk_node = list->head;
    node * wsk_node2;
    //idziemy do miejsca usuwania
    while(k--)
    {
        wsk_node = wsk_node->next;
    }
    wsk_node2 = wsk_node->next->next;
    free(wsk_node->next->elem);
    free(wsk_node->next);
    wsk_node->next = wsk_node2;
    //jeśli usuwamy ostatni zmieniamy tail
    if((size_t) index == list->length - 1)
    {
        list->tail = wsk_node;
    }
    list->length--;
} //end of remove_nth_element

int is_list_empty(List *list)
{
    return list->length == 0;
}

void pop (List *list)
{
    if (!is_list_empty(list))
    {
            remove_nth_element(list, 0);
    }
}
