CC=arm-none-linux-gnueabi-gcc 
CFLAGS=-static
OBJS=main.o
TARGET=20161664

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o:  hw.h main.c
	$(CC) $(CFLAGS) -c main.c

clean :
	rm -rf *.o $(TARGET)
