CC = h5c++
BUILD = ./build
CCFLAGS = -std=c++11
LIBS = -lhdf5_cpp

all: a.out

a.out: $(BUILD)/main.o $(BUILD)/h5out.o $(BUILD)/grid.o
	$(CC) $(CCFLAGS) $(BUILD)/main.o $(BUILD)/h5out.o $(BUILD)/grid.o -o a.out 

$(BUILD)/main.o: main.cc h5out.cc
	$(CC) $(CCFLAGS) -c main.cc $(LIBS)
	mv main.o $(BUILD)

$(BUILD)/h5out.o: h5out.cc h5out.h
	$(CC) $(CCFLAGS) -c h5out.cc $(LIBS)
	mv h5out.o $(BUILD)

$(BUILD)/grid.o: grid.cc grid.h
	$(CC) $(CCFLAGS) -c grid.cc $(LIBS)
	mv grid.o $(BUILD)

clean:
	rm build/*
