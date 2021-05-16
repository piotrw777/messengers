#ifndef CONFIG_H
#define CONFIG_H


#define LENGTH  9
#define TIMESTAMP_LENGTH 26
#define CSV_ENTRIES 2
#define CSV_ENTRY_SIZE 30

#define FIFO_PATH "./fifo"

#define COUNTER_FILENAME1 "./counter1.txt"
#define COUNTER_FILENAME2 "./counter2.txt"

#define SENT_FILENAME1 "./sent1.csv"
#define RECEIVED_FILENAME1 "./received1.csv"

#define SENT_FILENAME2 "./sent2.csv"
#define RECEIVED_FILENAME2 "./received2.csv"

#define QUEUED_FILENAME1 "./queued1.csv"
#define QUEUED_FILENAME2 "./queued2.csv"

#define FIFO_PATH1_2 "./fifo1_2"
#define FIFO_PATH2_1 "./fifo2_1"

#define RESET "\x1b[0m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define ORANGE "\x1b[33m"
#define YELLOW "\x1b[38;5;11m"
#define BG_VIOLET "\x1b[48;5;57m"
#define MAGENDA "\x1b[35m"
#define BLUE "\033[1;34m"
#define BOLD_GREEN "\033[1;32m"

#endif // CONFIG_H
