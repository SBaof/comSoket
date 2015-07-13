.PHONY:clean all
CC=gcc
CFLAGS=-Wall -g
BIN=srvDemo
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

srvDemo:srvDemo.o sckutil.o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o $(BIN)
