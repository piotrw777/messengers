CC := gcc
CFLAGS := -Wall -pthread -std=c11 -Wextra 

LIBS := -lm
OUT := prog
OBJ := src1.o thread_functions.o

all: $(OUT) copy 

$(OUT): main.o $(OBJ) config.h
	$(CC) $(CFLAGS) -o $@ main.o $(OBJ)

main.o: main.c config.h
	$(CC) $(CFLAGS) -c $< 

%.o: %.c %.h config.h
	$(CC) $(CFLAGS) -c $< 

copy:
	@cp $(OUT) /home/piotr/
	@cp /home/piotr/$(OUT) /home/piotr/$(OUT)2

clean:
	@echo Removing buid files
	rm -v *.o $(OUT)*

run:
	./$(OUT)
