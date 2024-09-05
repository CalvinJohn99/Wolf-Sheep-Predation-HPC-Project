CC=g++
#CFLAGS=-Wall -Wextra -g
#CFLAGS=-Wall -Wextra -O1
#CFLAGS=-Wall -Wextra -O2
CFLAGS=-Wall -Wextra -pg -O0
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


