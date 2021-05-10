#include "thread_functions.h"
#include "src1.h"
/*
void * counter(void * arg)
{
    int sleep_time = 150000;
    FILE *file;
    unsigned long k = 1;

    int n = 1e6;
    while (n-- && QUIT == false)
    {
        file = fopen(COUNTER_FILENAMES[prog_nr], "wb+");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[prog_nr]);
            sleep(1);
            continue;
        }
        if (is_empty(file))
        {
            fwrite(&k, sizeof(k), 1, file);
            //printf("Value written: %lu\n", k);
            usleep(sleep_time);
            k++;
        }

        //read current value
        rewind(file);
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file\n");
        }

        //printf("Value read: %lu from file: \n", k);
        rewind(file);
        //increment value
        k++;
        //write new value
        fwrite(&k, sizeof(k), 1, file);
        //printf("Value written: %lu\n", k);
        fclose(file);
        usleep(sleep_time);
    }

   // fclose(file);
    printf("Thread counter exiting\n");
    pthread_exit(NULL);
    return NULL;
}
*/
void * counter(void * arg)
{
    int sleep_time = 150000;
    FILE *file = (FILE *) arg;
    unsigned long k = 1;

    int n = 1e6;
    while (n-- && QUIT == false)
    {
        if (is_empty(file))
        {
            fwrite(&k, sizeof(k), 1, file);
            //printf("Value written: %lu\n", k);
            usleep(sleep_time);
            k++;
        }

        //read current value
        rewind(file);
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file\n");
        }

        //printf("Value read: %lu from file: \n", k);
        rewind(file);

        //increment value
        k++;

        //write new value
        fwrite(&k, sizeof(k), 1, file);
        //printf("Value written: %lu\n", k);
        usleep(sleep_time);
    }
    fclose(file);
    printf("Exiting the thread counter\n");
    pthread_exit(NULL);
    return NULL;
}

void * check_friend(void * arg)
{
    unsigned long k;
    unsigned long before, after;
    int friend_nr = 3 - prog_nr;
    bool status = INACTIVE;

    FILE *file;
    while (QUIT == false)
    {
        //open the counter file of the friend if it's not opened
        file = fopen(COUNTER_FILENAMES[friend_nr], "rb");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[friend_nr]);
            sleep(1);
            continue;
        }

        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            //continue;
        }
        fclose(file);
        before = k;
        //printf("Check friend before: %lu\n", k);

        sleep(1);

        file = fopen(COUNTER_FILENAMES[friend_nr], "rb");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[friend_nr]);
            sleep(1);
            continue;
        }

        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            //continue;
        }
        fclose(file);
        after = k;
        //printf("Check friend after: %lu\n", k);

        if (before != after && status == INACTIVE)
        {
            printf(GREEN"Other program is running!!!\n"RESET);
            friend_status = status = ACTIVE;
        }

        else if (before == after && status == ACTIVE)
        {
            printf(RED"Other program stopped!!!\n"RESET);
            friend_status = status = INACTIVE;
        }

    }
   // fclose(file);
    printf("Thread check_friend exiting\n");
    pthread_exit(NULL);
    return NULL;
}

void * send_messages(void *arg)
{
    static char timestamp_str[TIMESTAMP_LENGTH + 1];
    int fd, wr;
    const char * fifo_path = FIFO_PATHS[prog_nr];

    while (QUIT == false)
    {
        if (friend_status)
        {
            //create string from timestamp
            sprintf(timestamp_str, "%018lli", get_timestamp());
            //generate message
            create_random_message();
            printf(ORANGE"Message generated: %s\n"RESET, message);

            //write message to fifo

            fd = open(fifo_path, O_RDWR);
            if (fd < 0)
            {
                perror("Error opening the fifo: ");
            }
            wr = write(fd, message, LENGTH);

            if (wr < 0)
            {
                perror("Error writing to fifo: ");
            }
            if (QUIT == false)
            {
                close(fd);
            }
       }

        //write entry to the file
        fprintf(SENT_FILE, "%s,%s\n", timestamp_str, message);

        sleep(1);
    }

    printf("Thread send messages exiting\n");
    pthread_exit(NULL);
    return NULL;
}

void * read_messages(void *arg)
{
    const char * fifo_path = FIFO_PATHS[3 - prog_nr];
    int fd, rd;

    while (QUIT == false)
    {
        if (friend_status)
        {
            fd = open(fifo_path, O_RDWR);
            if (fd < 0)
            {
                perror("Error opening the fifo: ");
            }
            rd = read(fd, buffer, LENGTH);
            if (rd < 0)
            {
                perror("Error reading from fifo");
            }
            else if (rd < LENGTH)
            {
                perror("Some part of a message missing\n");
            }
            printf(BOLD_GREEN"Received message: %s\n"RESET, buffer);
            if (QUIT == false)
            {
                close(fd);
            }

            usleep(1e5);
        }
    }

    //close(fd);
    printf("Thread read messages exiting\n");
    pthread_exit(NULL);
    return NULL;
}
