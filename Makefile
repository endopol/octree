all:
	make lib
	cd src && make	
	make lib/octree.a

lib:
	mkdir lib

lib/octree.a: lib/coded_point.o lib/octree_point.o lib/octree.o lib/octree_graph.o lib/graph_traverse.o
	cd lib && ar rcs octree.a coded_point.o octree_point.o octree.o octree_graph.o

clean:
	rm -rf lib