CC = g++
CFLAGS = -g -std=c++17
TARGET = huffman
PROG = $(TARGET)

all:
	$(CC) $(CFLAGS) $(TARGET).cpp -o $(TARGET) -DBITS=$(BITS)

clean:
	rm -f $(PROG)
