#include "thread_functions.h"
#include "src1.h"
#include "list.h"

void * counter(void * arg)
{
    int sleep_time = 150000;
    FILE *file = (FILE *) arg;
    unsigned long k = 1;

    while (QUIT == false)
    {
        if (is_empty(file))
        {
            fwrite(&k, sizeof(k), 1, file);
            fwrite(&my_pid, sizeof(my_pid), 1, file);
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
        fwrite(&my_pid, sizeof(my_pid), 1, file);
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
            //fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[friend_nr]);
            sleep(1);
            continue;
        }
        //read value before
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            sleep(1);
            continue;
        }

        fclose(file);
        before = k;
        sleep(1);

        file = fopen(COUNTER_FILENAMES[friend_nr], "rb");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[friend_nr]);
            sleep(1);
            continue;
        }
        //read value after
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            continue;
        }
        //read friend_pid
        if (!fread(&friend_pid, sizeof(friend_pid), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            sleep(1);
            continue;
        }
        fclose(file);
        after = k;

        if (before != after && status == INACTIVE)
        {
            printf(GREEN"Other program is running!!!\n"RESET);

            printf("Friend's pid: %d\n", friend_pid);
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
    List *queued_msgs = create_list();
    int fd, wr;
    const char * fifo_path = FIFO_PATHS[prog_nr];
    static char timestamp_str[TIMESTAMP_LENGTH + 1];
    char *queued_msg;
    FILE *file;

    while (QUIT == false)
    {
        //if friend is active
        if (friend_status)
        {
            //no messages in the queue to send
            if(is_list_empty(queued_msgs))
            {
                //generate new message
                create_timestamp(timestamp_str, 'N');
                create_random_message();
                printf(ORANGE"Message generated: %s\n"RESET, message);

                //write message to fifo
                fd = open(fifo_path, O_RDWR);
                //fd = open(fifo_path, O_WRONLY);

                if (fd < 0)
                {
                    perror("Error opening the fifo: ");
                }
                wr = write(fd, message, LENGTH);

                if (wr <= 0)
                {
                    perror("Error writing to fifo: ");
                }
                //succesfull write to FIFO
                else
                {
                    //write entry to the file
                    file = fopen(SENT_FILENAMES[prog_nr], "a");
                    fprintf(file, "%s,%s\n", timestamp_str, message);
                    fclose(file);
                }

                if (QUIT == false)
                {
                    close(fd);
                }
            }
            //sending a message from a queue
            else
            {
                queued_msg = queued_msgs->head->elem;
                //write queued message to fifo
                fd = open(fifo_path, O_RDWR);

                if (fd < 0)
                {
                    perror("Error opening the fifo: ");
                }
                wr = write(fd, queued_msg, LENGTH);

                if (wr <= 0)
                {
                    perror("Error writing to fifo: ");
                }
                //if send succesfull remove message from queue
                else
                {
                    create_timestamp(timestamp_str, 'Q');
                    printf(MAGENDA"Message from queue sent: %s\n"RESET, queued_msg);
                    //write entry to the file
                    file = fopen(SENT_FILENAMES[prog_nr], "a");
                    fprintf(file, "%s,%s\n", timestamp_str, queued_msg);
                    fclose(file);

                    if (QUIT == false)
                    {
                        close(fd);
                    }
                    pop(queued_msgs);
                }
            }
       }
        //friend inactive - saving messages to queue
        else
        {
            create_random_message();
            append_to_list(queued_msgs, message);
            printf(YELLOW"Message %s saved in queue\n"RESET, message);

            if (QUIT == false)
            {
                close(fd);
            }
        }
        while (PAUSE == true)
        {
            close(fd);
            printf("hmm...no sending\n");

            if (friend_status == INACTIVE)
            {
                PAUSE = false;
            }
            EXIT_ALLOWANCE_SEND = true;
            if (EXIT_ALLOWANCE_READ == true && EXIT_ALLOWANCE_SEND == true)
            {
                kill(friend_pid, SIGUSR2);
                EXIT_ALLOWANCE_READ = EXIT_ALLOWANCE_SEND = false;
            }
            sleep(1);
        }
        sleep(1);
    }
    //save queued messages to the file
    while (queued_msgs->head != NULL)
    {
        FILE *file_que;
        file_que = fopen(QUEUED_FILENAMES[prog_nr], "a");
        fprintf(file_que, "%s\n", queued_msgs->head->elem);
        fclose(file_que);
        pop(queued_msgs);
    }

    //clear_list(queued_msgs);
    destroy_list(&queued_msgs);
    printf("Thread send messages exiting\n");
    pthread_exit(NULL);
    return NULL;
}

void * read_messages(void *arg)
{
    int fd, rd;
    const char * fifo_path = FIFO_PATHS[3 - prog_nr];
    static char timestamp_str[TIMESTAMP_LENGTH + 1];
    FILE *file;

    while (QUIT == false)
    {
        if (friend_status)
        {
            create_timestamp(timestamp_str, 'R');

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
            //succesfull read
            else
            {
                printf(BOLD_GREEN"Received message: %s\n"RESET, buffer);

                //save read message to file
                file = fopen(RECEIVED_FILENAMES[prog_nr], "a");
                fprintf(file, "%s,%s\n",timestamp_str,buffer);
                fclose(file);
            }

            if (QUIT == false)
            {
                close(fd);
            }

            usleep(1e5);
        }
        while (PAUSE == true)
        {
            close(fd);
            printf("hmm...no reading\n");
            EXIT_ALLOWANCE_READ = true;
            if (friend_status == INACTIVE)
            {
                PAUSE = false;
            }
            if (EXIT_ALLOWANCE_READ == true && EXIT_ALLOWANCE_SEND == true)
            {
                kill(friend_pid, SIGUSR2);
                EXIT_ALLOWANCE_READ = EXIT_ALLOWANCE_SEND = false;
            }
            sleep(1);
        }
    }
    //close(fd);
    printf("Thread read messages exiting\n");
    pthread_exit(NULL);
    return NULL;
}
