CC = gcc
CFLAGS = -Wall -g
OBJS = charset.o tagcontainer.o mptag.o
PROGRAM = mptag.exe

all:	$(PROGRAM)

$(PROGRAM):	$(OBJS)
	$(CC) $(OBJS) $(CFLAGS) -o $(PROGRAM)
