INCLUDE=-I../include -I../helper
LIBS=../lib/octree.a

DEBUG=-g
RELEASE=-O4 -DNDebug
FLAGS=-std=c++0x -Wall -pedantic $(RELEASE) $(INCLUDE)


all: octree_test.cpp
	cd .. && make all
	make octree

octree: octree_test.cpp ../include/octree.h ../include/globals.h ../include/linalg.h ../include/pcd_io.h ../include/visualize.h
	g++ $(FLAGS) -o octree octree_test.cpp $(LIBS)

clean:
	rm -f octree
	cd .. && make clean
