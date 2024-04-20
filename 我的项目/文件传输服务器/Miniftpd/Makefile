CC=gcc
CFLAGS=-Wall -g
BIN=Miniftpd
LIBS=-lcrypt
OBJS=main.o sysutil.o session.o ftpproto.o privparent.o str.o parseconf.o tunable.o privsock.o hash.o
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
 %.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
.PHONY:clean
clean:
	rm -fr *.o $(BIN)
