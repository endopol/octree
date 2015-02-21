all: lib lib/octree.a

lib:
	mkdir lib

lib/octree.a: lib/coded_point.o lib/octree_point.o lib/octree.o lib/octree_graph.o
	cd lib && ar rcs octree.a coded_point.o octree_point.o octree.o octree_graph.o


lib/coded_point.o:
	cd src && make coded_point.o
	mv src/coded_point.o lib/coded_point.o

lib/octree_point.o:
	cd src && make octree_point.o
	mv src/octree_point.o lib/octree_point.o

lib/octree.o:
	cd src && make octree.o
	mv src/octree.o lib/octree.o

lib/octree_graph.o:
	cd src && make octree_graph.o
	mv src/octree_graph.o lib/octree_graph.o

clean:
	rm -f lib/*