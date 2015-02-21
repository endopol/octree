/*
 * =====================================================================================
 *
 *       Filename:  octree.cpp
 *
 *    Description:  Efficient data-structure for handling point clouds
 *
 *        Version:  1.0
 *        Created:  09/18/2013 04:12:27 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joshua Hernandez (jah), endopol@gmail.com
 *   Organization:  UCLA Vision Lab (vision.cs.ucla.edu)
 *
 * =====================================================================================
 */
#include "octree.h"
#include <iomanip>
using namespace std;

/* #####   Constructors/Destructors   ############################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Octree
 *      Method:  Octree(const float*, int)
 * Description:  Construct the root of a new Octree
 *--------------------------------------------------------------------------------------
 */
Octree::Octree(const double *new_limits, int new_max_depth)
{

    /* Position on the tree */
    parent = NULL;
    address = 0 ;
    index = 0;
    depth = 0;

    /* Limits */
    max_depth = new_max_depth;
    depth_bit = ((codestring) 1) << (NDIM * (max_depth - 1));

    for (int i = 0; i < 2 * NDIM; i++)
        limits[i] = new_limits[i];

    /* p d */
    num_descendants = 0;
    children = new Octree*[NDIV];
    for (int i = 0; i < NDIV; i++)
        children[i] = NULL;

    /* Data */
    data = NULL;
}


/*
 *--------------------------------------------------------------------------------------
 *       Class:  Octree
 *      Method:  Octree(const PointIter, const PointIter, Octree*, codestring,
 *                  vector<OctreePoint*>&)
 * Description:  Construct non-root tree
 *--------------------------------------------------------------------------------------
 */
Octree::Octree(const PointIter new_begin, const PointIter new_end, Octree *new_parent,
               codestring new_address, vector<OctreePoint*>& new_points)
{

    /* Position on the tree */
    parent = new_parent;
    max_depth = parent->max_depth;
    address = new_address;    
    codeToLocation(address, int_location, max_depth);

    depth = parent->depth + 1;
    depth_bit = (parent->depth_bit) >> NDIM;
    index = new_points.size();

    /* Limits */
    codestring test_bit = parent->depth_bit;
    for (int i = 0; i < NDIM; i++)
    {
        int half_bit = (bool) (test_bit & address);
        double width = parent->limits[2 * i + 1] - parent->limits[2 * i];
        limits[2 * i] = parent->limits[2 * i] + half_bit * width / 2;
        limits[2 * i + 1] = limits[2 * i] + width / 2;
        test_bit <<= 1;
    }

    /* Children */
    num_descendants = 0;
    // Leaf nodes
    if (depth == max_depth)
        children = NULL;
    // Non-leaf nodes
    else
    {
        children = new Octree*[NDIV];
        for (int i = 0; i < NDIV; i++)
            children[i] = NULL;
    }

    /* data */
    data = NULL;

    findPoints(new_begin, new_end, new_points, true);
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Octree
 *      Method:  ~Octree()
 * Description:  Recursively destroy a tree
 *--------------------------------------------------------------------------------------
 */
Octree::~Octree()
{
    if (children != NULL)
    {
        for (int i = 0; i < NDIV; i++)
        {
            if (children[i] != NULL)
                delete children[i];
            children[i] = NULL;
        }
        children = NULL;
        delete[] children;
    }

    if (data != NULL)
    {
        delete data;
    }
}

/* #####   Initializers   ########################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Octree
 *      Method:  void addPoints(const double*, const double*)
 * Description:  Add new points to the tree
 *--------------------------------------------------------------------------------------
 */
void Octree::addPoints(const double *new_points, const float *new_normals,
                       int num_points, OctreeGraph &graph)
{
    //Find num valid points to add
    int numValid = 0;

    /* 1. Create a vector of CodedPoints */
    vector<CodedPoint> new_codes;
    new_codes.reserve(num_points);
    for (int i = 0; i < num_points; i++)
    {
        const float* next_normal = (new_normals==NULL)?NULL:(&new_normals[i*NDIM]);
        const double* next_point = &new_points[i*NDIM];
        
        CodedPoint new_point;
        if(new_normals != NULL)
            new_point = CodedPoint(next_point, next_normal,  limits, max_depth);
        else 
            new_point = CodedPoint(next_point, limits, max_depth);

        if(new_point.good_point){
            numValid++;
            new_codes.push_back(new_point);
        }
    }
    cout << "Added " << numValid << " / " << num_points << " good points ";
    //for(int i=0; i<new_codes.size(); i++)
    //    cout << new_codes[i] << endl;

    /* 2. Sort the vector */
    sort(new_codes.begin(), new_codes.end());


    /* 3. Add points */
    findPoints(new_codes.begin(), new_codes.end(), graph.getVertices(), true);

    graph.computeEdges();
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Octree
 *      Method:  void addPoints(PointIter, const PointIter, OctreeGraph&)
 * Description:  Recursive function adds points from a sorted segment of a vector
 *--------------------------------------------------------------------------------------
 */
void Octree::findPoints(PointIter begin, const PointIter end, 
    vector<OctreePoint*>& new_points, bool adding){

    /* Display tree as it is being built * (DISABLED)
    cout << "Level=" << depth << ", bit=" << depth_bit << ":\n";
    */

    /* If at the bottom level, then store and go up */
    if (depth == max_depth)
    {
        /* Display tree as it is being built * (DISABLED)
        cout << "Adding coded points ";
        for(PointIter curr = begin; curr!=end; curr++)
            cout << *curr << " ";
        cout << " at location " << address << endl;
        */

        if(adding){
            if (data == NULL)
            {
                data = new OctreePoint(begin, end, this);
                new_points.push_back(data);
            }
            else
                data->add(begin, end);
        }
        else{
            if(data != NULL)
                new_points.push_back(data);
        }

        return;
    }

    int old_count = new_points.size();
    codestring new_address = address;
    PointIter new_end = begin;
    for (int i = 0; i < NDIV; i++)
    {
        new_address += depth_bit;

        /* Grab the section defined by the new address */
        while ((new_end != end) && new_end->code < new_address)
            new_end++;

        /* If there are points there, then file 'em */
        if (new_end != begin)
        {

            /* Display tree as it is being built * (DISABLED)
            for(int j=0; j<depth; j++)
                cout << "  ";
            cout << "Codes " << begin->code << "-" << (new_end-1)->code << " ";
            cout << "stored in " << i << ":" << new_address-depth_bit
                 << "-" << new_address-1 << ".\n";
            */

            if (children[i] == NULL){
                if(adding)
                    children[i] = new Octree(begin, new_end, this, 
                        new_address - depth_bit, new_points);
            }
            else
                children[i]->findPoints(begin, new_end, new_points, adding);
        }
        /* set up for the next child */
        begin = new_end;
    }
    /* Take credit for all the new nodes created in this thread */
    if(adding)
        num_descendants += (new_points.size() - old_count);
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Octree
 *      Method:  Octree* findAddress(codestring)
 * Description:  Find the tree with a given address
 *--------------------------------------------------------------------------------------
 */
Octree* Octree::searchUp(codestring minCode, codestring maxCode){
    /* If the address is below this one, then descend */
    if (minCode >= address && maxCode < address + (depth_bit << NDIM))
        return this;
    else
        return parent->searchUp(minCode, maxCode);
}

OctreePoint* Octree::findPoint(const double *location)
{
    CodedPoint new_point(location, limits, max_depth);
    return findAddress(new_point.get_code());
}

OctreePoint* Octree::findAddress(codestring address){    
    CodedPoint new_point(address);
    PointBuffer pb; pb.push_back(new_point);
    vector<OctreePoint*> pv;

    Octree* root = searchUp(address, address);
    root->findPoints(pb.begin(), pb.end(), pv, false);

    if (pv.empty())
        return NULL;
    else
        return pv[0];
}

/* #####   I/O   #################################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  Octree
 *      Method:  const void print(ostream&)
 * Description:  Recursively print out a tree
 *--------------------------------------------------------------------------------------
 */
void Octree::print(ostream &out) const
{
    for (int i = 0; i < depth - 1; i++)
        out << "| ";
    if (depth > 0)
        out << "+-";

    out << "Octree " << depth << "-" << address;
    if (parent == NULL)
        out << " (root). ";
    else
        out << ", son of " << parent->depth << "-" << parent->address << ". ";

    out << setprecision(3);
    out << "Limits: {";
    for (int i = 0; i < (NDIM - 1); i++)
        out << "[" << limits[2 * i] << ", " << limits[2 * i + 1] << "], ";
    out << "[" << limits[2 * (NDIM - 1)] << ", " << limits[2 * (NDIM - 1) + 1] << "]}. ";

    //out << "DB=" << depth_bit << " ";

    /* Display data */
    out << endl;
    for (int i = 0; i < depth; i++)
        out << "  ";
    if (data != NULL)
        out << *data;
    out << endl;

    if (children != NULL)
        for (int i = 0; i < NDIV; i++)
            if (children[i] != NULL)
                children[i]->print(out);
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ostream& operator<<(ostream& out, Octree &tree)
 *  Description:  Print a tree to stream
 * =====================================================================================
 */
ostream &operator<<(ostream &out, Octree &tree)
{

    // cnull stuff
    nullbuf null_obj;
    wnullbuf wnull_obj;
    std::ostream cnull(&null_obj);
    std::wostream wcnull(&wnull_obj);

    tree.print(cout);
    cout << "\nTotal tree nodes: " << tree.num_descendants << ".\n";

    return out;
}
