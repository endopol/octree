#include "octree.h"
#include <queue>
#include <cmath>
#include <iomanip>
	
vector<double> dist_from(OctreePoint *start, OctreeGraph &graph, vector<int> &previous, 
	DistFunction df=point_distance,	double MAX_DIST=numeric_limits<double>::infinity()){

	vector<OctreePoint*>& vertices = graph.getVertices();
	previous.resize(vertices.size());
	for (unsigned int i = 0; i < previous.size(); i++)
		previous[i] = i;

	vector<double> distances;
	distances.resize(graph.getNumVertices(), MAX_DIST);

	priority_queue<OctreePoint*> pq;
	pq.emplace(start);
	distances[start->getIndex()] = 0;

	while (!pq.empty()) {
		//       cout<<"size: "<<pq.size()<<endl; cout.flush();
		OctreePoint* base = pq.top();
		pq.pop();

		int base_index = base->getIndex();
	
		// Iterate through neighbors
		for (unsigned int i = 0; i < base->getNeighbors().size(); i++) {
			OctreePoint* neighbor = base->getNeighbor(i);
			if (neighbor == NULL || neighbor == base)
				continue;			

			int neighbor_index = neighbor->getIndex();
			double new_distance = distances[base_index] + df(base, neighbor);

			if ((new_distance < distances[neighbor_index])  && new_distance < MAX_DIST) {
				pq.emplace(neighbor);
				distances[neighbor_index]   = new_distance;
				previous[neighbor_index]    = base_index;
			}
		}
	}



	return distances;
}
