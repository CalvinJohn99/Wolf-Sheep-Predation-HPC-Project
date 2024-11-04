CC=g++
MPICXX?=/usr/bin/mpic++
CFLAGS = -g -O3 -Wall -Wextra -Wpedantic
#CFLAGS=-Wall -Wextra -g -O0
#CFLAGS=-Wall -Wextra -pg -O0
SRCS=$(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
EXEC=main

all: $(EXEC)

#$(MPICXX) -pg $^ -o $@
$(EXEC): $(OBJS)
	$(MPICXX) $(CFLAGS) $^ -o $@


%.o: %.cpp
	$(MPICXX) $(CFLAGS) -c $< -o $@

clean:
	rm $(EXEC) $(OBJS)

.PHONY: all clean


