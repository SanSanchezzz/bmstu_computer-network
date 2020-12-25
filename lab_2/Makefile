CC := gcc
CFLAGS := -std=c99 -Werror -Wall -ggdb
OBJS := converter.o

all : server client

server : $(OBJS) server.o
	$(CC) $^ -o $@

client : $(OBJS) client.o
	$(CC) $^ -o $@

%.o : %.c *.h
	$(CC) $(CFLAGS) -c $<

clean :
	$(RM) *.o *.exe
