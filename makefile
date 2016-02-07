CC = mpic++
BUILD = ./build

all: a.out

a.out: $(BUILD)/main.o
	$(CC) $(BUILD)/main.o -o a.out

$(BUILD)/main.o: main.cc
	$(CC) -c main.cc
	mv main.o $(BUILD)
