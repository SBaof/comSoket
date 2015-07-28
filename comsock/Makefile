.PHONY:clean all

CC=gcc
CFLAGS=-Wall -g
BIN=cli srv
all:$(BIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

cli:cli.o commsock.o sckutil.o
	$(CC) $(CFLAGS) $^ -o $@

srv:srv.o commsock.o sckutil.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(BIN)
