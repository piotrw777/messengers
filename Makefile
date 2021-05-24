CC := gcc
CFLAGS := -Wall -pthread -std=c11 -Wextra
OUT := prog
OBJ := src1.o thread_functions.o list.o
HEADERS := $(wildcard *.h)

all: $(OUT)

$(OUT): main.o $(OBJ) $(HEADERS)
	$(CC) $(CFLAGS) -o $@ main.o $(OBJ)

main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

%.o: %.c %.h $(HEADERS)
	$(CC) $(CFLAGS) -c $<

clean:
	@echo Removing buid files
	rm -v *.o $(OUT)

clear:
	@echo Removing text files
	rm -v *.txt *csv
run:
	./$(OUT)
