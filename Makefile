CC = gcc
CFLAGS = -Wall -g
TARGET = smallsh
OBJS = smallsh.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

smallsh.o: smallsh.c smallsh.h
	$(CC) $(CFLAGS) -c smallsh.c

clean:
	rm -f $(TARGET) $(OBJS) prog1
