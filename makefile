CC = mpic++
BUILD = ./build
CCFLAGS = -std=c++11

all: a.out

a.out: $(BUILD)/main.o
	$(CC) $(CCFLAGS) $(BUILD)/main.o -o a.out

$(BUILD)/main.o: main.cc
	$(CC) $(CCFLAGS) -c main.cc
	mv main.o $(BUILD)
