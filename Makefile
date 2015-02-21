all: lib lib/octree.a lib/pcd_io.o lib/visualize.o

lib:
	mkdir lib

lib/pcd_io.o:
	cd helper && make pcd_io.o
	mv helper/pcd_io.o lib/pcd_io.o

lib/visualize.o:
	cd helper && make visualize.o
	mv helper/visualize.o lib/visualize.o

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