INCLUDE_DIR=-I../include 
INCLUDES=../include/octree.h ../include/globals.h ../include/linalg.h ../include/pcd_io.h ../include/visualize.h
LIBS=../lib/octree.a

DEBUG=-g
RELEASE=-O4 -DNDebug
FLAGS=-std=c++0x -Wall -pedantic $(RELEASE) $(INCLUDE_DIR)


all: octree_test.cpp
	cd .. && make all
	make octree

octree: octree_test.cpp $(LIBS) $(INCLUDES)
	g++ $(FLAGS) -o octree octree_test.cpp $(LIBS)

clean:
	rm -f octree
	cd .. && make clean
