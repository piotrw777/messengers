CC := gcc
CFLAGS := -Wall -pthread -std=c11 -Wextra

LIBS := -lm
OUT := prog
OBJ := src1.o thread_functions.o list.o
HEADERS := $(wildcard *.h)

all: $(OUT) copy

$(OUT): main.o $(OBJ) config.h
	$(CC) $(CFLAGS) -o $@ main.o $(OBJ)

main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c $<

%.o: %.c %.h $(HEADERS)
	$(CC) $(CFLAGS) -c $<

copy:
	@cp $(OUT) $(OUT)2
	@cp $(OUT)* /home/piotr/

clean:
	@echo Removing buid files
	rm -v *.o $(OUT)*

clear:
	@echo Removing text files
	rm -v *.txt *csv
run:
	./$(OUT)
