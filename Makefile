CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
TARGET = tor_network
OBJS = main.o node.o client.o network_utils.o

all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c tor_types.h node.h client.h
	$(CC) $(CFLAGS) -c main.c

node.o: node.c node.h tor_types.h network_utils.h
	$(CC) $(CFLAGS) -c node.c

client.o: client.c client.h tor_types.h network_utils.h
	$(CC) $(CFLAGS) -c client.c

network_utils.o: network_utils.c network_utils.h tor_types.h
	$(CC) $(CFLAGS) -c network_utils.c


clean:
	rm -f $(OBJS) $(TARGET)


.PHONY: all clean
