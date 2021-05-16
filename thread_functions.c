#include "thread_functions.h"
#include "src1.h"
#include "list.h"

void * counter(void * arg)
{
    int sleep_time = 50;
    unsigned long k = 0;
    int friend_status_before = INACTIVE;
    int friend_status_after = INACTIVE;

    //clear the counter file
    FILE *file = fopen(COUNTER_FILENAMES[prog_nr], "wb");
    fwrite(&k, sizeof(k), 1, file);
    fwrite(&my_pid, sizeof(my_pid), 1, file);
    k += 2;
    nsleep(sleep_time);
    fclose(file);

    while (QUIT == false)
    {
        //read current value
        file = fopen(COUNTER_FILENAMES[prog_nr], "rb");
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file\n");
        }
        fclose(file);

        //increment value
        k += 2;

        //if friend is active communicate to friend
        //that you know he's active
        //so that he can start read your messages
        friend_status_after = friend_status;
        if (friend_status_after == ACTIVE && friend_status_before == INACTIVE)
        {
            k += 1;
            read_ready = true;
            printf("Ready for reading...\n");
            friend_status_before = ACTIVE;
        }
        if (friend_status_after == INACTIVE && friend_status_before == ACTIVE)
        {
            k -= 1;
            read_ready = false;
            printf("Stop reading...\n");
            friend_status_before = INACTIVE;
        }
        //write new value
        file = fopen(COUNTER_FILENAMES[prog_nr], "wb");
        fwrite(&k, sizeof(k), 1, file);
        fwrite(&my_pid, sizeof(my_pid), 1, file);
        fclose(file);
        //printf("Value written: %lu\n", k);
        nsleep(sleep_time);
    }
    printf("Exiting the thread counter\n");
    pthread_exit(NULL);
    return (arg = NULL);
}

void * check_friend(void * arg)
{
    int sleep_time = 500;
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
            nsleep(sleep_time);
            continue;
        }
        //read value before
        if (!fread(&k, sizeof(k), 1, file))
        {
            fprintf(stderr, "Error reading from file (check_friend)\n");
            nsleep(sleep_time);
            continue;
        }

        fclose(file);
        before = k;
        //check if your friend is ready for reading
        if (k % 2 == 1)
        {
            friend_read_ready = true;
        }
        else
        {
            friend_read_ready = false;
        }
        //wait
        nsleep(sleep_time);

        file = fopen(COUNTER_FILENAMES[friend_nr], "rb");
        if (file == NULL)
        {
            fprintf(stderr, "File %s not opened\n",COUNTER_FILENAMES[friend_nr]);
            nsleep(sleep_time);
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
            nsleep(sleep_time);
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
    printf("Thread check_friend exiting\n");
    pthread_exit(NULL);
    return (arg = NULL);
}

void * send_messages(void *arg)
{

    List *queued_msgs = create_list();
    int fd, wr;
    const char * fifo_path = FIFO_PATHS[prog_nr];
    static char timestamp_str[TIMESTAMP_LENGTH + 1];
    static char pause_message[LENGTH] = {0,'P'};
    static char quit_message[LENGTH] = {0,'Q'};

    char *queued_msg;
    FILE *file;

    while (QUIT == false)
    {
        //if friend is active
        if (friend_status && friend_read_ready)
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
                create_timestamp(timestamp_str, 'Q');
                wr = write(fd, queued_msg, LENGTH);

                if (wr <= 0)
                {
                    perror("Error writing to fifo: ");
                }
                //if send succesfull remove message from queue
                else
                {
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
        }
        //prepare for exit
        if (QUIT_SENDING == true)
        {
            //send pause message (for reading function)
            fd = open(fifo_path, O_RDWR);

            if (fd < 0)
            {
                perror("Error opening the fifo: ");
            }
            wr = write(fd, pause_message, LENGTH);

            if (wr <= 0)
            {
                perror("Error writing to fifo: ");
            }

            close(fd);
            //wait for exit allowance from oter program
            while (QUIT == false)
            {
                printf("Waiting for exit...\n");
                nsleep(SLEEPTIME_SEND);
            }
        }
        else if (PAUSE_SENDING == true)
        {
            //send quit message
            fd = open(fifo_path, O_RDWR);
            if (fd < 0)
            {
                perror("Error opening the fifo: ");
            }
            wr = write(fd, quit_message, LENGTH);
            if (wr <= 0)
            {
                perror("Error writing to fifo: ");
            }
            close(fd);
            EXIT_ALLOWANCE_SEND = true;
            printf("hmm...no sending\n");
            while (PAUSE_SENDING == true)
            {
                if (EXIT_ALLOWANCE_READ == true && EXIT_ALLOWANCE_SEND == true)
                {
                    kill(friend_pid, SIGUSR2);
                    EXIT_ALLOWANCE_READ = EXIT_ALLOWANCE_SEND = false;
                }
                if (friend_status == INACTIVE)
                {
                    PAUSE_SENDING = false;
                }
                nsleep(SLEEPTIME_SEND);                           }
        }
        nsleep(SLEEPTIME_SEND);
    }
    //save queued messages to the file
    printf("Saving unsent messages to the file\n");
    while (queued_msgs->head != NULL)
    {
        FILE *file_que;
        file_que = fopen(QUEUED_FILENAMES[prog_nr], "a");
        fprintf(file_que, "%s\n", queued_msgs->head->elem);
        fclose(file_que);
        pop(queued_msgs);
    }
    unlink(FIFO_PATHS[prog_nr]);
    destroy_list(&queued_msgs);
    printf("Thread send messages exiting\n");
    pthread_exit(NULL);
    return (arg = NULL);
}

void * read_messages(void *arg)
{


    int fd, rd;
    const char * fifo_path = FIFO_PATHS[3 - prog_nr];
    static char timestamp_str[TIMESTAMP_LENGTH + 1];
    //static char pause_message[LENGTH] = {0,'P'};
    //static char quit_message[LENGTH] = {0,'Q'};

    FILE *file;

    while (QUIT == false)
    {
        if (friend_status && read_ready)
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
            //succesfull read
            //read pause message
            else if (buffer[0] == 0 && buffer[1] == 'P')
            {
                close(fd);
                printf("hmm...no reading\n");
                PAUSE_READING = true;
                PAUSE_SENDING = true;
                EXIT_ALLOWANCE_READ = true;
                while (PAUSE_READING == true)
                {

                    if (friend_status == INACTIVE)
                    {
                        PAUSE_READING = false;
                    }
                    if (EXIT_ALLOWANCE_READ == true && EXIT_ALLOWANCE_SEND == true)
                    {
                        kill(friend_pid, SIGUSR2);
                        EXIT_ALLOWANCE_READ = EXIT_ALLOWANCE_SEND = false;
                    }
                    nsleep(SLEEPTIME_READ);
                }
            }
            //read quit message
            else if (buffer[0] == 0 && buffer[1] == 'Q')
            {
                close(fd);
                while (QUIT == false)
                {
                    printf("Wait for exit allowance...\n");
                    nsleep(SLEEPTIME_READ);
                }
            }
            //normal message
            else
            {
                create_timestamp(timestamp_str, 'R');
                printf(BOLD_GREEN"Received message: %s\n"RESET, buffer);
                //save read message to file
                file = fopen(RECEIVED_FILENAMES[prog_nr], "a");
                fprintf(file, "%s,%s\n",timestamp_str,buffer);
                fclose(file);
            }
            nsleep(SLEEPTIME_READ);
        }
    }
    printf("Thread read messages exiting\n");
    pthread_exit(NULL);
    return (arg = NULL);
}
