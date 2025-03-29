CC = g++
CFLAGS = -g -std=c++17 -MMD -Ofast
TARGET = huffman
PROG = $(TARGET) *.o *.d
SRCS = huffman.cpp basic_huffman.cpp adaptive_huffman.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

.PHONY = all clean huffman

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

-include $(DEPS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -DBITS=$(BITS)

clean:
	rm -f $(PROG)
