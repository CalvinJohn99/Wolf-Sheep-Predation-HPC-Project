CC=g++
CFLAGS = -g -O3 -Wall -Wextra -Wpedantic
#CFLAGS=-Wall -Wextra -g -O0
#CFLAGS=-Wall -Wextra -pg -O0
SRCS=$(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
EXEC=main

all: $(EXEC)

#$(CC) -pg $^ -o $@
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(EXEC) $(OBJS)

.PHONY: all clean


