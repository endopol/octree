HEADERS=../include/octree.h  ../include/linalg.h
INCLUDE_DIR=../include

DEBUG=-g
RELEASE=-O4 -DNDebug
FLAGS=-std=c++0x -Wall -pedantic $(RELEASE) -I$(INCLUDE_DIR)

coded_point.o: $(HEADERS) coded_point.cpp
	g++ -c $(FLAGS) coded_point.cpp

octree_point.o: $(HEADERS) octree_point.cpp
	g++ -c $(FLAGS) octree_point.cpp

octree.o: $(HEADERS) octree.cpp
	g++ -c $(FLAGS) octree.cpp

octree_graph.o: $(HEADERS) octree_graph.cpp
	g++ -c $(FLAGS) octree_graph.cpp