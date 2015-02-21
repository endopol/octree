HEADERS=../include/octree.h
INCLUDE_DIR=../include

DEBUG=-g
RELEASE=-O4 -DNDebug
FLAGS=-std=c++0x -Wall -pedantic $(RELEASE) -I$(INCLUDE_DIR)

pcd_io.o: $(HEADERS) pcd_io.cpp
	g++ -c $(FLAGS) pcd_io.cpp

visualize.o: $(HEADERS) visualize.cpp
	g++ -c $(FLAGS) visualize.cpp