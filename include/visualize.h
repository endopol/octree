#ifndef VISUALIZE_H
#define VISUALIZE_H

#include "octree.h"
#include <iomanip>
#include <fstream>

const double colours[5][3] =
{
    {  1,   0,   0},
    { .5,  .5,   0},    
    {  0,   1,   0},    
    {  0,  .5,  .5},
    {  0,   0,   1},    
};

// ######################################################################
void rainbow(float r, unsigned int *color);

// ######################################################################
void outputVisualization(OctreeGraph &graph, int imnum, const char *outfile)
{
    init_viz(); // get parameters from config file

    cout << "saving image " << imnum << " at " << outfile << endl;
    fstream out(outfile, fstream::out);

    int n_copies = 1;
    if (edgetype > 0)
        n_copies = 3;
    int point_mult = n_copies;
    if (edgetype == NORMALS)
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

    if (edgetype == GRAPH)
        out << "element face " << graph.getNumEdges() << endl;

    if (edgetype == NORMALS)
        out << "element face " << graph.getNumVertices() << endl;

    if (edgetype > 0)
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
            if (loctype == NOMINAL)
                location = vertices[i]->getNomLocation();
            if (loctype == AVERAGE)
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
    if (edgetype == NORMALS)
    {
        double normal_length = 0;
        if(vertices.size()>0) 
            normal_length = 1/((double)(1<<vertices[0]->getDepth()));

        for (unsigned int i = 0; i < vertices.size(); i++)
        {
            const double *location;
            if (loctype == NOMINAL)
                location = vertices[i]->getNomLocation();
            else // (loctype == AVERAGE)
                location = vertices[i]->getLocation();

            for (int j = 0; j < 3; j++)
            {
                if (isnan((float) vertices[i]->getNormal()[j]))
                    out << location[j] << " ";
                else
                    out << (double) location[j] + normal_length * vertices[i]->getNormal()[j] << " ";
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
    if (edgetype == GRAPH)
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
    r = fmax(0, fmin(1, r))*5;

    int r0 = ((int) floor(r)) % 5, r1 = (r0 + 1) % 5;
    float rf = r - floor(r);

    //cout << r0 << " " << rf << " " << r1 << endl;
    for (int i = 0; i < 3; i++)
        color[i] = (unsigned int) ((1 - rf) * 255 * colours[r0][i]
                                   + rf * 255 * colours[r1][i]);
}

#endif // VISUALIZE_H