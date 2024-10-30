CC=g++
MPICXX?=/usr/bin/mpic++
CFLAGS=-Wall -Wextra -g -O0
#CFLAGS=-Wall -Wextra -pg -O0
SRCS=$(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
EXEC=main

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -pg $^ -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(EXEC) $(OBJS)

.PHONY: all clean


