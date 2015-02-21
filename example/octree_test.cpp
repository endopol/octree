#include "octree.h"
#include <algorithm>
#include <string>
#include <ctime>
#include <iostream>

#undef NDEBUG
#include <assert.h>
#include <cmath>
#include <sstream>
#include "pcd_io.cpp"
#include "visualize.cpp"

using namespace std;

int visualize(vector<string> filenames);

vector<int> previous;
double  lims[]              = {-1,1, -1,1, -1,1};
int depth = 10;


int main(){

    Octree tree(lims, depth); // stores points, builds mesh
    OctreeGraph graph;  // Tree iterator - Keeps a list of nodes and edges
    if(!load_points_from_pxx("bun180.ply", tree, graph))
        return -1;
    graph.computeNormals();

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



