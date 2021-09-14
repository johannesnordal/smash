CPP=g++
SRC=src
OBJ=obj
BIN=bin
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
DEPS=/usr/local/lib/libbifrost.a -lz -pthread
CFLAGS=-Wall -Ofast -std=c++11

all: $(OBJ) $(BIN) $(BIN)/sketch $(BIN)/dist $(BIN)/atom

$(OBJ):
	mkdir $@

$(BIN):
	mkdir $@

OBJS1=$(OBJ)/CommandSketch.o $(OBJ)/FastaData.o $(OBJ)/MinHash.o $(OBJ)/Sketch.o
$(BIN)/sketch: $(OBJS1)
	$(CPP) $(CFLAGS) $(OBJS1) $(DEPS) -o $@

OBJS2=$(OBJ)/CommandDist.o $(OBJ)/Dist.o $(OBJ)/FastaData.o $(OBJ)/MinHash.o $\
	  $(OBJ)/Sketch.o
$(BIN)/dist: $(OBJS2)
	$(CPP) $(CFLAGS) $(OBJS2) $(DEPS) -o $@

OBJS3=$(OBJ)/Sketch.o $(OBJ)/Atom.o $(OBJ)/MinHash.o $(OBJ)/FastaData.o $\
	  $(OBJ)/Dist.o
$(BIN)/atom: $(OBJS3)
	$(CPP) $(CFLAGS) $(OBJS3) $(DEPS) -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -r $(OBJ)
	rm $(BIN)/sketch
	rm $(BIN)/dist
	rm $(BIN)/atom
