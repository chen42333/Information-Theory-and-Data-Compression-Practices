CC = g++
CFLAGS = -g -std=c++17 -MMD
TARGET = huffman
PROG = $(TARGET) *.o *.d

ALGO = BASIC
ifeq ($(ALGO),ADAPTIVE)
    ALGO_LIB = adaptive_huffman
else
	ALGO_LIB = basic_huffman
endif

SRCS = $(TARGET).cpp $(ALGO_LIB).cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

.PHONY = all clean huffman

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

-include $(DEPS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -DBITS=$(BITS) -D$(ALGO)

clean:
	rm -f $(PROG)
