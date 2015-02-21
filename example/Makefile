INCLUDE=-I../include -I../helper
LIBS=../lib/octree.a

DEBUG=-g
RELEASE=-O4 -DNDebug
FLAGS=-std=c++0x -Wall -pedantic $(RELEASE) $(INCLUDE)

octree: octree_test.cpp
	cd .. && make all
	g++ $(FLAGS) -o octree octree_test.cpp $(LIBS)

clean:
	rm -f octree
	cd .. && make clean
