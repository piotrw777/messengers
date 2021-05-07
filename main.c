#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void * threadFunction(void * arg)
{
    int start = *((int *) arg);
    int n = 10;
    while (n--)
    {
        printf("%d\n", start);
        start++;
        sleep(1);
    }

   pthread_exit(NULL);

}

int main()
{
    int start = 5;
    printf("Create a thread\n");
    pthread_t w;
    pthread_create(&w, NULL, threadFunction, (void *) &start);
    pthread_join(w, NULL);

}
