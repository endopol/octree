/*
 * =====================================================================================
 *
 *       Filename:  octree_graph.cpp
 *
 *    Description:  Manage a weighted graph based on signatures of points in an octree
 *
 *        Version:  1.0
 *        Created:  09/18/2013 06:35:03 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joshua Hernandez (jah), endopol@gmail.com
 *   Organization:  UCLA Vision Lab (vision.cs.ucla.edu)
 *
 * =====================================================================================
 */
#include "octree.h"
#include <queue>
#include <cmath>

/* #####   OCTREE_EDGE  -  MEMBER FUNCTION DEFINITIONS   ########################## */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  OctreeEdge
 *      Method:  OctreeEdge(OctreePoint*, OctreePoint*, int)
 * Description:  Basic constructor for the OctreeEdge class
 *--------------------------------------------------------------------------------------
 */
OctreeEdge::OctreeEdge(OctreePoint *new_p1, OctreePoint *new_p2)
{
    p1 = new_p1;
    p2 = new_p2;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ostream& operator<<(ostream&, const GraphEdge&)
 *  Description:  Print GraphVertex details to a stream
 * =====================================================================================
 */
ostream &operator<<(ostream &out, const OctreeEdge &edge)
{
    out << "Edge: ";
    out << edge.p1->getIndex() << "-to-" << edge.p2->getIndex() << ".";

    return out;
}


/* #####   OCTREE_GRAPH  -  MEMBER FUNCTION DEFINITIONS   ########################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  OctreeGraph
 *      Method:  OctreeGraph()
 * Description:  Initialize the graph (at present, trivially)
 *--------------------------------------------------------------------------------------
 */
OctreeGraph::OctreeGraph()
{
    frame_indices.push_back(vertices.size());
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  OctreeGraph
 *      Method:  OctreePoint* addPoint(const PointIter, const PointIter, Octree*)
 * Description:  Construct a point with data lying in the vector
 *--------------------------------------------------------------------------------------
 */
void OctreeGraph::addPoint(OctreePoint *p)
{
    vertices.push_back(p);
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  OctreeGraph
 *      Method:  OctreeEdge* addEdge(OctreePoint*, OctreePoint*)
 * Description:  Construct an edge with data lying in the vector
 *--------------------------------------------------------------------------------------
 */
void OctreeGraph::addEdge(OctreePoint *p1, OctreePoint *p2)
{
    edges.push_back(new OctreeEdge(p1, p2));
}


// Traverse cloud and flip normals to point in consistent directions
void OctreeGraph::fixNormals(OctreePoint *start, vector<float> &total_turn)
{
    queue<OctreePoint *> pq;

    if (norm2(start->normal) == 0)
        return;

    pq.push(start);
    total_turn[start->index] = 0;

    while (!pq.empty())
    {
        OctreePoint *base = pq.front();
        pq.pop();

        float base_turn = total_turn[base->index];

        for (unsigned int i = 0; i < base->neighbors.size(); i++)
        {
            OctreePoint *neighbor = base->neighbors[i];
            if (neighbor != NULL && neighbor != base && (long)neighbor > 0xff)
            {


                double dp = dot(base->normal, neighbor->normal),
                       new_turn = fmin(acos(dp), acos(-dp));


                if (base_turn + new_turn < total_turn[neighbor->index])
                {

                    scale(neighbor->normal, sgn(dp));

                    total_turn[neighbor->index] = base_turn + new_turn;
                    pq.push(neighbor);
                }
            }
        }
    }

}

void OctreeGraph::computeNormals()
{
    TIC("Computing normals: ")    
    for (unsigned int i = 0; i < vertices.size(); i++)
        vertices[i]->computeNormal();
    TOC

    return;

    vector<float> total_turn;
    total_turn.resize(vertices.size(), INFINITY);
    for (unsigned int i = 0; i < vertices.size(); i++)
        if (total_turn[i] == INFINITY)
        {
            //             cout << "Fixing at " << i << endl;
            fixNormals(vertices[i], total_turn);
        }

}

void OctreeGraph::computeEdges()
{
    // bool good_vertex = true;
    for (unsigned int i = 0; i < vertices.size(); i++)
    {
        // cout << i << ". (";
        // for (int j = 0; j < NDIM; j++)
        // {
        //     cout << vertices[i]->home->limits[2 * j]
        //          << "<" << vertices[i]->location[j]
        //          << "<" << vertices[i]->home->limits[2 * j + 1] << "\t";
        //     good_vertex = good_vertex && ((vertices[i]->home->limits[2 * j] <= vertices[i]->location[j])
        //                                   && (vertices[i]->home->limits[2 * j + 1] >= vertices[i]->location[j]));            
        // }
        // cout << ") ";
        // if (good_vertex) cout << " X";
        // cout << endl;
        vertices[i]->findNeighbors(*this);

    }

}

/*
const double AFFINITY_THRESH = 0.0000;
void OctreeGraph::thin(){
    for(int i=*(frame_indices.end()-2); i<*(frame_indices.end()-1); i++){
        OctreePoint* p1 = vertices[i];

        float max_affinity = AFFINITY_THRESH;
        OctreePoint* merge_point = NULL;
        for(int j=0; j<NNEI; j++){
            OctreePoint* p2 = p1->neighbors[j];
            if(p2==NULL || p2->frameno == p1->frameno)
                continue;

            float dp[NDIM];

            sub(p1->location, p2->location, dp);
            if(dp>0)
                scale(dp, 1/norm2(dp));

            double affinity = abs(dot(dp, p1->normal)) + abs(dot(dp, p2->normal));
            //cout << "Affinity: " << affinity << endl;
            if(affinity>max_affinity){
                max_affinity = affinity;
                merge_point = p2;
            }

        }
        if(merge_point!=NULL)
            p1->mergeTo(merge_point);
    }
}
*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ostream& operator<<(ostream&, const OctreeGraph&)
 *  Description:  Prints details of an OctreeGraph to a stream
 * =====================================================================================
 */
ostream &operator<<(ostream &out, OctreeGraph &graph)
{
    for (unsigned int i = 0; i < graph.vertices.size(); i++)
        out << *graph.vertices[i] << endl;
    return out;
}

/* Return a reference to the vector of vertices */
vector<OctreePoint *> &OctreeGraph::getVertices()
{
    return vertices;
}

/* Return a reference to the vector of edges */
vector<OctreeEdge *> &OctreeGraph::getEdges()
{
    return edges;
}

OctreePoint *OctreeGraph::getVertex(int i)
{
    return vertices[i];
}

/* Return the number of stored vertices */
int OctreeGraph::getNumVertices() const
{
    return vertices.size();
}

/* Return the total number of stored vertices */
int OctreeGraph::getNumEdges() const
{
    return edges.size();
}


