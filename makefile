CC = h5c++ -I/usr/lib/openmpi/include -I/usr/lib/openmpi/include/openmpi -pthread -L/usr//lib -L/usr/lib/openmpi/lib -lmpi_cxx -lmpi -ldl -lhwloc
BUILD = ./build
CCFLAGS = -std=c++11
LIBS = -lhdf5_cpp -lopenmpi

all: a.out

a.out: $(BUILD)/main.o $(BUILD)/h5out.o $(BUILD)/grid.o $(BUILD)/logger.o
	$(CC) $(CCFLAGS) $(BUILD)/main.o $(BUILD)/h5out.o $(BUILD)/grid.o $(BUILD)/logger.o -o a.out 

$(BUILD)/main.o: main.cc h5out.cc logger.cc
	$(CC) $(CCFLAGS) -c main.cc $(LIBS)
	mv main.o $(BUILD)

$(BUILD)/h5out.o: h5out.cc h5out.h
	$(CC) $(CCFLAGS) -c h5out.cc $(LIBS)
	mv h5out.o $(BUILD)

$(BUILD)/grid.o: grid.cc grid.h
	$(CC) $(CCFLAGS) -c grid.cc $(LIBS)
	mv grid.o $(BUILD)

$(BUILD)/logger.o: logger.cc logger.h grid.cc
	$(CC) $(CCFLAGS) -c logger.cc $(LIBS)
	mv logger.o $(BUILD)


clean:
	rm build/*
