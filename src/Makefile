HEADERS=../include/linalg.h ../include/octree.h 
INCLUDE_DIR=../include
DEBUG=-g
RELEASE=-O4 -DNDebug
FLAGS=-std=c++0x -Wall -pedantic $(RELEASE) -I$(INCLUDE_DIR)

all: ../lib/coded_point.o ../lib/octree_point.o ../lib/octree.o ../lib/octree_graph.o

../lib/coded_point.o: $(HEADERS) coded_point.cpp
	g++ -c $(FLAGS) coded_point.cpp
	mv coded_point.o ../lib

../lib/octree_point.o: $(HEADERS) octree_point.cpp
	g++ -c $(FLAGS) octree_point.cpp
	mv octree_point.o ../lib

../lib/octree.o: $(HEADERS) octree.cpp
	g++ -c $(FLAGS) octree.cpp
	mv octree.o ../lib

../lib/octree_graph.o: $(HEADERS) octree_graph.cpp
	g++ -c $(FLAGS) octree_graph.cpp
	mv octree_graph.o ../lib