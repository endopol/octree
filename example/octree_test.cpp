#include "octree.h"
#include <algorithm>
#include <string>
#include <ctime>
#include <iostream>

#include <cmath>
#include <sstream>
#include "globals.h"
#include "pcd_io.h"
#include "visualize.h"

using namespace std;

int visualize(vector<string> filenames);

int main(){
    parse_globals("default.cfg");

    Octree tree(lims, DEPTH); // stores points, builds mesh
    OctreeGraph graph;  // Tree iterator - Keeps a list of nodes and edges
    if(!load_points_from_pxx("bun180.ply", tree, graph))
        return -1;
    graph.computeNormals();

    // Color vertices by their normal direction
    for(int i=0; i<graph.getNumVertices(); i++){
        const float* normal = graph.getVertex(i)->getNormal();
        int* color = graph.getVertex(i)->color;
        for(int j=0; j<3; j++)
            color[j] = (int) 255*normal[j];
    }

    /** VIEW OUTPUTS **/
    vector<string> outputFiles;
    const char *outFile = "outfile.ply";
    outputVisualization(graph, 0, outFile);
    outputFiles.push_back(string(outFile));

    visualize(outputFiles);

    return 1;
}

int visualize(vector<string> filenames){
    ostringstream os("meshlab ");
    os << "meshlab ";
    //     for(int i=0; i<filenames.size(); i++)
    os << filenames.back() << " >/dev/null 2>/dev/null";
    cout<<"About to run command: "<<os.str()<<endl;
    return system(os.str().c_str());
}



