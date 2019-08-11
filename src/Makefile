CC=gcc
CFLAGS = -Wall -pthread -g -O0

all: ps_server tc

ps_server: ps_server2.c linklist.c stringmatch.c
	$(CC) $(CFLAGS) ps_server2.c linklist.c stringmatch.c -o ps_server

tc: test_client.c
	$(CC) $(CFLAGS) test_client.c -o tc

clean:
	rm ps_server tc
