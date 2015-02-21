/*
 * =====================================================================================
 *
 *       Filename:  octree.h
 *
 *    Description:  Classes for efficiently handling point clouds
 *
 *        Version:  1.0
 *        Created:  09/18/2013 04:12:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joshua Hernandez (jah), endopol@gmail.com
 *   Organization:  UCLA Vision (vision.cs.ucla.edu)
 *
 * =====================================================================================
 */
#ifndef OCTREE_H
#define OCTREE_H

#include <cmath>
#include <algorithm>
#include <vector>
#include <list>
#include <iostream>
#include <stdlib.h>
#include <map>
#include <set>

using namespace std;

/* #####   EXPORTED MACROS   ######################################################## */

#define NDIM 3 /* = # of dimensions */
#define FOOT 3 /* Radius of footprint */
#define DIAM 7 /* 1+2*FOOT */

#if NDIM == 2
#define NDIV 4 /* = 2^NDIM */
#define NNEI 8  /* = DIAM^NDIM */
#elif NDIM == 3
#define NDIV 8
#define NNEI 343
#endif

#define EPS 0.00000001

#include "linalg.h"

/* #####   EXPORTED TYPE DEFINITIONS   ############################################## */

typedef unsigned long long codestring;    // In case "unsigned long" is too short


/* #####   EXPORTED FUNCTION DECLARATIONS   ######################################### */

template<typename T>
void printBinary(T n, ostream &out);
codestring locationToCode(const long *location, int max_depth);
void codeToLocation(codestring code, long *location, int max_depth);


/* #####   EXPORTED CLASS DEFINITIONS   ############################################# */

class OctreePoint;
class Octree;
class OctreeEdge;
class OctreeGraph;

/*
 * =====================================================================================
 *        Class:  CodedPoint
 *  Description:  Specially-encoded points for octrees
 * =====================================================================================
 */
class CodedPoint
{
    double location[NDIM];   // Point coordinates
    long int_location[NDIM];   // Discretized coordinates
    float normal[NDIM];
    codestring code;        // Encoded location for fast comparisons

    friend class Octree;
    friend class OctreePoint;

    friend bool operator<(const CodedPoint &p1, const CodedPoint &p2);
    friend ostream &operator<<(ostream &out, const CodedPoint &p);

public:
    bool good_point;

    CodedPoint(const double *new_location, const double *limits, int max_depth);
    CodedPoint(const double *new_location, const float *new_normal,
               const double *limits, int max_depth);
    CodedPoint(codestring new_code);
    CodedPoint(const long* new_int_location, int max_depth);
    CodedPoint();

    codestring get_code();
};

#define ENOUGH_POINTS 20
typedef vector<CodedPoint> PointBuffer;
typedef PointBuffer::iterator PointIter;
/*
 * =====================================================================================
 *        Class:  OctreePoint
 *  Description:  Accumulates a history of CodedPoints and
 * =====================================================================================
 */
class OctreePoint
{
    Octree *home;
    vector<OctreePoint*> neighbors;
    double nom_location[NDIM];   // Nominal location
    double location[NDIM];       // Averaged coordinates
    float normal[NDIM];         // Average normal vector

    codestring address;         // Encoded location of this point
    int index,                  // Index in the graph
        num_points,             // Total number of points assigned to this
        depth;

    friend class Octree;
    friend class OctreeEdge;
    friend class OctreeGraph;
    friend ostream &operator<<(ostream &out, const OctreePoint &p);
    friend float point_distance(const OctreePoint *p1, const OctreePoint *p2);
    friend vector<double> dist_from(OctreePoint *start, OctreeGraph &graph, vector<int> &previous);
public:
    int color[3];

    OctreePoint();

    OctreePoint(const PointIter begin, const PointIter end, Octree *new_home);
    ~OctreePoint();

    void add(const PointIter begin, const PointIter end);

    // Accessor methods

    vector<OctreePoint*> getNeighbors();
    OctreePoint *getNeighbor(int i) const;
    int getIndex() const;

    const double *getLocation() const;
    const double *getNomLocation() const;
    double &getLocation(int i);
    double &getNomLocation(int i);

    const float *getNormal() const;

    int getDepth() const;

private:
    void findNeighbors(OctreeGraph &graph);
    void computeCovariance(double cov[NDIM][NDIM]);
    void computeNormal();
};

template <typename t> int sgn(t val)
{
    return (t(0) < val) - (val < t(0));
}

/*
 * =====================================================================================
 *        Class:  OctreeEdge
 *  Description:  Data structure holding adjacency data
 * =====================================================================================
 */
class OctreeEdge
{

    friend class OctreeGraph;

    friend ostream &operator<<(ostream &out, const OctreeEdge &edge);

public:
    OctreePoint *p1, *p2;    // Index of adjacent vertices
    OctreeEdge(OctreePoint *new_p1, OctreePoint *new_p2);
};

#define EDGES_PER_VERTEX  (NNEI/3)
/*
 * =====================================================================================
 *        Class:  OctreeGraph
 *  Description:  Data structure for interfacing Octrees with MRF inference
 * =====================================================================================
 */
class OctreeGraph
{
    vector<OctreePoint *> vertices;  // Storage for OctreePoints
    vector<OctreeEdge *> edges;      // Storage for OctreeEdges
    vector<int> frame_indices;

    friend ostream &operator<<(ostream &out, OctreeGraph &graph);

public:
    OctreeGraph();

    void addPoint(OctreePoint *p);
    void addEdge(OctreePoint *p1, OctreePoint *p2);

    void computeEdges();
    void computeNormals();
    void reserve(int new_capacity);

    vector<OctreePoint *> &getVertices();
    vector<OctreeEdge *> &getEdges();
    OctreePoint *getVertex(int i);

    int getNumVertices() const;
    int getNumEdges() const;

private:
    void fixNormals(OctreePoint *start, vector<float> &total_turn);
};


#define VOXEL_CUBE_TYPE 1
#define VOXEL_BOX_TYPE 2
typedef vector<CodedPoint> PointBuffer;
typedef PointBuffer::iterator PointIter;
/*
 * =====================================================================================
 *        Class:  Octree
 *  Description:  Easily-searchable tree
 * =====================================================================================
 */
class Octree
{
    Octree *parent,             // Parent node
           **children;          // Dynamic array of child pointers (pre-allocated)

    codestring depth_bit,       // Useful constant: 2^(max_depth-depth)
               address;         // Lowest address of a point assignable to this tree
    long int_location[NDIM];    // Discretized coordinates               

    int index,                  // Index of this tree in the graph
        depth,                  // Depth of this tree
        num_descendants;        // Number of descendant trees

    OctreePoint *data;          // Vector of OctreePoints assigned to this tree

    double limits[2 * NDIM];     // Limits on the points in this volume;

    friend class OctreePoint;
    friend class OctreeGraph;

    friend ostream &operator<<(ostream &out, Octree &tree);

public:
    int max_depth;              // Max depth allowable in this tree
    Octree(const double *new_limits, int new_max_depth);
    Octree(const double *temp_limits, const float *resolutions, int new_max_depth,
           int voxel_type);
    Octree(const PointIter new_begin, const PointIter new_end, Octree *new_parent,
           codestring new_address, vector<OctreePoint*>& new_points);
    ~Octree();

    void addPoints(const double *new_points, const float *new_normals,
                   int num_points, OctreeGraph &graph);
    void findPoints(PointIter new_begin, const PointIter new_end,
                    vector<OctreePoint *> &new_points, bool adding);
    Octree* searchUp(codestring minCode, codestring maxCode);

    void print(ostream &out) const;

    OctreePoint *findPoint(const double *location);

private:
    OctreePoint *findAddress(codestring query_address);
};

/*
 * =====================================================================================
 *       Struct:  basic_nullbuff
 *  Description:  /dev/null replacement
 * =====================================================================================
 */
template<typename Ch, typename Traits = std::char_traits<Ch> >
struct basic_nullbuf : std::basic_streambuf<Ch, Traits>
{
    typedef std::basic_streambuf<Ch, Traits> base_type;
    typedef typename base_type::int_type int_type;
    typedef typename base_type::traits_type traits_type;

    virtual int_type overflow(int_type c)
    {
        return traits_type::not_eof(c);
    }
};

// convenient typedefs
typedef basic_nullbuf<char> nullbuf;
typedef basic_nullbuf<wchar_t> wnullbuf;

// buffers and streams
// in some .h
extern std::ostream cnull;
extern std::wostream wcnull;


#endif
