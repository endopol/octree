#include "octree.h"
#include <iomanip>
#include <fstream>


const double colours[12][3] =
{
    {  1,   0,   0},
    {.75, .25,   0},
    { .5,  .5,   0},
    {.25, .75,   0},
    {  0,   1,   0},
    {  0, .75, .25},
    {  0,  .5,  .5},
    {  0, .25, .75},
    {  0,   0,   1},
    {.25,   0, .75},
    { .5,   0,  .5},
    {.75,   0, .25},
};

// ######################################################################
void rainbow(float r, unsigned int *color);

// ######################################################################
void outputVisualization(OctreeGraph &graph, int imnum, const char *outfile)
{
    cout << "saving image " << imnum << " at " << outfile << endl;
    fstream out(outfile, fstream::out);

    enum edgetype {NONE, NORMALS, GRAPH};
    edgetype edges = NORMALS;

    enum loctype {AVERAGE, NOMINAL};
    loctype loc = AVERAGE;

    int n_copies = 1;
    if (edges > 0)
        n_copies = 3;
    int point_mult = n_copies;
    if (edges == NORMALS)
        point_mult--;

    out << "ply" << endl;
    out << "format ascii 1.0" << endl;
    out << "comment written by ucla vision lab solid objects" << endl;
    out << "element vertex " << n_copies *graph.getNumVertices() << endl;
    out << "property float32 x" << endl;
    out << "property float32 y" << endl;
    out << "property float32 z" << endl;

    out << "property uchar red" << endl;
    out << "property uchar green" << endl;
    out << "property uchar blue" << endl;

    if (edges == GRAPH)
        out << "element face " << graph.getNumEdges() << endl;

    if (edges == NORMALS)
        out << "element face " << graph.getNumVertices() << endl;

    if (edges > 0)
        out << "property list uchar int vertex_index " << endl;

    out << "end_header" << endl;

    int depth = graph.getVertex(0)->getDepth();
    int prec = ceil(.75 * depth);
    vector<OctreePoint *> vertices = graph.getVertices();
    for (int n = 0; n < point_mult; n++)
        for (unsigned int i = 0; i < vertices.size(); i++)
        {
            out << setprecision(prec) << fixed;

            const double *location;
            if (loc == NOMINAL)
                location = vertices[i]->getNomLocation();
            if (loc == AVERAGE)
                location = vertices[i]->getLocation();

                           // coordinates
            for (int j = 0; j < 3; j++)            
                out << setw(prec + 3) << (float) location[j] << " ";                                    

            // color
            for (int j = 0; j < 3; j++)            
                out << setw(3) << vertices[i]->color[j] << " ";            

            out << endl;
        }
    // normal vectors
    if (edges == NORMALS)
    {
        double normal_length = 0;
        if(vertices.size()>0) 
            normal_length = 10/((double)(1<<vertices[0]->getDepth()));

        cout << "adding normals: \n";
        for (unsigned int i = 0; i < vertices.size(); i++)
        {
            const double *location;
            if (loc == NOMINAL)
                location = vertices[i]->getNomLocation();
            if (loc == AVERAGE)
                location = vertices[i]->getLocation();

            for (int j = 0; j < 3; j++)
            {
                if (isnan((float) vertices[i]->getNormal()[j]))
                    out << location[j] << " ";
                else
                    out << (float) location[j] + normal_length * vertices[i]->getNormal()[j] << " ";
            }

            for (int j = 0; j < 3; j++)        
                out << 255 << " ";                

            out << endl;
        }

        int num_points = graph.getNumVertices();
        for (int i = 0; i < graph.getNumVertices(); i++)
            out << "3 "
                << i << " "
                << i + num_points << " "
                << i + 2 * num_points << endl;

        out.close();
        return;
    }


    int num_points = graph.getNumVertices();
    // edges
    if (edges == GRAPH)
    {
        vector<OctreeEdge *> edges = graph.getEdges();
        for (unsigned int i = 0; i < edges.size(); i++)
        {
            OctreePoint *p1 = edges[i]->p1, *p2 = edges[i]->p2;

            out << "3 "
                << p1->getIndex() << " "
                << p2->getIndex() << " "
                << p1->getIndex() + num_points << endl;
        }
    }

    out.close();
}

// ######################################################################
void rainbow(float r, unsigned int *color)
{

    if (r == 0)
    {
        for (int i = 0; i < 3; i++)
        {
            color[i] = 0;
        }
        return;
    }

    int r0 = ((int) floor(r)) % 12, r1 = (r0 + 1) % 12;
    float rf = r - floor(r);

    //cout << r0 << " " << rf << " " << r1 << endl;
    for (int i = 0; i < 3; i++)
        color[i] = (unsigned int) ((1 - rf) * 255 * colours[r0][i]
                                   + rf * 255 * colours[r1][i]);
    // for(int i=0; i<3; i++)
    //         color[i] = (unsigned int) colours[(int)r][i];

    // cout<<"GOT COLOR: "<<r<<" and set: "<<color[0] <<" "<<color[1]<<" "<<color[2]<<endl;

}

