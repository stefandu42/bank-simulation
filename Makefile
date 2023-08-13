CC=gcc

CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror -g

ALL= server client pdr maint

all: $(ALL)

server : server.o utils_v1.o utils.o
	$(CC) $(CCFLAGS) -o server server.o utils_v1.o utils.o
server.o: server.c utils.h
	$(CC) $(CCFLAGS) -c server.c

client : client.o utils_v1.o utils.o
	$(CC) $(CCFLAGS) -o client client.o utils_v1.o utils.o
client.o: client.c utils.h
	$(CC) $(CCFLAGS) -c client.c

pdr : pdr.o utils_v1.o utils.o
	$(CC) $(CCFLAGS) -o pdr pdr.o utils_v1.o utils.o
pdr.o: pdr.c utils.h
	$(CC) $(CCFLAGS) -c pdr.c

maint : maint.o utils_v1.o utils.o
	$(CC) $(CCFLAGS) -o maint maint.o utils_v1.o utils.o
maint.o: maint.c utils.h
	$(CC) $(CCFLAGS) -c maint.c

utils_v1.o: utils_v1.c utils_v1.h
	$(CC) $(CCFLAGS) -c utils_v1.c

utils.o: utils.c utils.h
	$(CC) $(CCFLAGS) -c utils.c

clean:
	rm -f *.o
	rm -f $(ALL)		