CC=g++
MPICXX?=/usr/bin/mpic++
CFLAGS = -g -O3 -Wall -Wextra -Wpedantic -fopenmp
#CFLAGS=-Wall -Wextra -g -O0 -fopenmp
#CFLAGS=-Wall -Wextra -pg -O0 -fopenmp
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



#***Makefile for running on cluster: 
# Compiler and MPI setup
# CC=g++
# MPICXX?=g++
# MPI_INC = /usr/include/openmpi-x86_64
# MPI_LIB = /usr/lib64/openmpi/lib
# CXXFLAGS=-std=c++11 -O3 -Wall -Wextra -Wpedantic -I$(MPI_INC)

# Target executable name
# TARGET=main

# Source and object files
# SRCS=$(wildcard *.cpp)
# OBJS = $(patsubst %.cpp,%.o,$(SRCS))

# Libraries for MPI linking
# LIBS=-L$(MPI_LIB) -lmpi_cxx -lmpi

# Build rules
# all: $(TARGET)

# $(TARGET): $(OBJS)
#	$(MPICXX) $(CXXFLAGS) $(OBJS) $(LIBS) -o $(TARGET)

# %.o: %.cpp
#	$(MPICXX) $(CXXFLAGS) -c $< -o $@

# clean:
#	rm -f $(TARGET) $(OBJS)

# .PHONY: all clean
