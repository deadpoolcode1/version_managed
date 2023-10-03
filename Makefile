# Output binary
OUTPUT ?= version_check.bin

# Compiler
CXX ?= /usr/bin/g++

# Flags
CXXFLAGS ?= -Og -g -std=c++11

# Libraries
LIBS = -lxxhash -lpthread 

# Source files
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(OBJS) $(LIBS) 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OUTPUT) $(OBJS)
