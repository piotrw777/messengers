CC := gcc
CFLAGS := -Wall -pthread -std=c11 -Wextra 
LIBS := -lm
OUT := prog

all: $(OUT)

$(OUT): main.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c
	$(CC) $(CFLAGS) -c $< 

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< 

clean:
	@echo Removing buid files
	rm -v *.o $(OUT)*

run:
	./$(OUT)
