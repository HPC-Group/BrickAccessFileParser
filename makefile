# source files.
SRC = BrickAccessFile.cpp \
	SampleMain.cpp \

# include directories
INCLUDEDIRS = ./

# object files
OBJ = $(SRC:.cpp=.o)

# output file
OUT = BrickAccessFileParser.a

# set up C++11 compiler and std libraries
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
CCFLAGS = -g -O2 -Wall -std=c++11 -stdlib=libc++
CCC = clang++
else
CCFLAGS = -g -O2 -Wall -std=c++0x
CCC = g++
endif

# set up linked libraries and paths
LDFLAGS = -lm

.SUFFIXES: .cpp
.PHONY: clean

# default target
all: $(OUT)

%.o: %.cpp
	$(CCC) -I $(INCLUDEDIRS) $(CCFLAGS) -c $< -o $@

$(OUT): $(OBJ)
	$(CCC) $(CCFLAGS) -o $(OUT) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(OUT)
