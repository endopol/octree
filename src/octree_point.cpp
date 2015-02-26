/*
 * =====================================================================================
 *
 *       filename:  Octree_point.cpp
 *
 *    description:  point-history archive for the Octree class
 *
 *        version:  1.0
 *        created:  09/18/2013 03:58:35 pm
 *       revision:  none
 *       compiler:  gcc
 *
 *         author:  joshua hernandez (jah), endopol@gmail.com
 *   organization:  ucla vision lab (vision.cs.ucla.edu)
 *
 * =====================================================================================
 */
#include "octree.h"
#include <iomanip>
#include <assert.h>
#include <cmath>

#define pi 3.14159265

/* #####   constructors   ########################################################### */
OctreePoint::OctreePoint() {
    index = -1;
    address = 0;

    num_points = 0;

    home = NULL;
    depth = 0;
}

OctreePoint::OctreePoint(codestring new_address) {
    index = -1;
    address = new_address;

    num_points = 0;

    home = NULL;
    depth = 0;
}

OctreePoint::OctreePoint(const PointIter begin, const PointIter end, Octree *new_home) {

    /* position */
    home = new_home;
    address = new_home->address;
    index = new_home->index;
    depth = new_home->depth;

    for (int i = 0; i < NDIM; i++) {
        location[i] = 0;
        nom_location[i] = (home->limits[2 * i] + home->limits[2 * i + 1]) / 2;
        color[i] = 0;
    }


    num_points = 0;

    add(begin, end);
}

OctreePoint::~OctreePoint() {
    home = NULL;
}

/* #####   mutators   ############################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       class:  OctreePoint
 *      method:  add(const PointIter, const PointIter, Octree&)
 * description:  add the given range of codedpoints to this point
 *--------------------------------------------------------------------------------------
 */
void OctreePoint::add(const PointIter begin, const PointIter end) {

    int old_num_points = num_points;

    for (PointIter curr = begin; curr != end; curr++)
        num_points++;


    if (old_num_points < ENOUGH_POINTS) {
        for (int i = 0; i < NDIM; i++) {
            location[i] *= old_num_points;
            normal[i] *= old_num_points;
        }

        for (PointIter curr = begin; curr != end && old_num_points < ENOUGH_POINTS; curr++) {
            for (int i = 0; i < NDIM; i++) {
                location[i] += curr->location[i];
                normal[i] += curr->normal[i];
            }
            old_num_points++;
        }
        for (int i = 0; i < NDIM; i++) {
            location[i] /= old_num_points;
            normal[i] /= old_num_points;
        }
    }
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  OctreePoint
 *      Method:  void findNeighbors()
 * Description:  Find existing neighbors and link to them
 *--------------------------------------------------------------------------------------
 */
void OctreePoint::findNeighbors(OctreeGraph &graph) {
    //if(((long)home)>0xffffffff)
    //    return;

    // 1. Build list of addresses
    neighbors.resize(0);
    PointBuffer pb;
    long loc_max = 1l << depth;

    // cout << "int_location: ";
    // for (int j = 0; j < NDIM; j++) cout << home->int_location[j] << " ";
    // cout << endl;

    for (int i = 0; i < NNEI; i++) {
        if (i == NNEI / 2) continue;

        int offset = i;
        bool good_neighbor = true;
        long new_int_location[NDIM];
        for (int j = 0; j < NDIM; j++) {
            int offset_j = (offset % DIAM) - FOOT;
            new_int_location[j] = home->int_location[j] + offset_j;

            /* Check whether this neighbor lies in the volume */
            if (new_int_location[j] < 0 || new_int_location[j] >= loc_max) {
                good_neighbor = false;
                break;
            }
            offset = offset / DIAM;
        }
        // cout << "\ti=" << i << ", new_int_location: ";
        // for (int k = 0; k < NDIM; k++) cout << new_int_location[k] << " ";
        // cout << endl;

        if (good_neighbor)
            pb.emplace_back(new_int_location, home->max_depth);
    }

    if (!pb.empty()) {
        // 2. Sort the tree
        sort(pb.begin(), pb.end());

        // 3. Move up the tree
        Octree *root = home->searchUp(pb.front().code, pb.back().code);

        // 4. Fill neighbor vector
        root->findPoints(pb.begin(), pb.end(), neighbors, false);
    }

    sort(neighbors.begin(), neighbors.end());
    for (unsigned int i = 0; i < neighbors.size(); i++)
        graph.addEdge(this, neighbors[i]);
}


/*
 *--------------------------------------------------------------------------------------
 *       Class:  OctreePoint
 *      Method:  computeNormal(const double*)
 * Description:  Computes normal vector based on neighboring points' locations
 *--------------------------------------------------------------------------------------
 */
const double SIGMA_STEP = 1;
void OctreePoint::computeNormal() {
    cout.setf(ios::fixed, ios::floatfield);

    double min_ratio = 1;
    for (int k = 1; k <= (COVAR_SIGMA / SIGMA_STEP); k++) {
        double sigma = k * SIGMA_STEP / ((double)(1 << depth));
        // Covariance matrix
        double l_mat[NDIM][NDIM];
        computeCovariance(l_mat, sigma);

        double v1[NDIM] = {1, 0, 0};
        double e1 = lanczos(l_mat, v1, 10);

        for (int i = 0; i < NDIM; i++)
            l_mat[i][i] -= e1;

        double v2[NDIM] = {0, 0, 1};
        double e2 = lanczos(l_mat, v2, 10);

        double ratio = fabs(e2 / e1);
        if (ratio < min_ratio) {
            min_ratio = ratio;
            scale(v2, 1 / sqrt(norm2(v2)));
            copyTo(v2, normal);
        }
    }

    // cout << "\t" << normal[0] << " " << normal[1] << " " << normal[2] << endl << endl;
}

/*
 *      Method:  OctreePoint :: computeCovariance(double[NDIM][NDIM])
 * Description:
 *--------------------------------------------------------------------------------------
 */
void OctreePoint::computeCovariance(double cov[NDIM][NDIM], double sigma) {
    for (int i = 0; i < NDIM; i++)
        for (int j = 0; j < NDIM; j++)
            cov[i][j] = 0;

    double ld[NDIM];
    int nnei = neighbors.size();
    double total_weight = 0;
    for (int k = 0; k < nnei; k++) {
        double *l1 = neighbors[k]->location;

        // Get relative location
        sub(l1, location, ld);
        double weight = gauss(ld, sigma);

        for (int i = 0; i < NDIM; i++)
            for (int j = 0; j < NDIM; j++)
                cov[i][j] += weight * (1 << (depth)) * ld[i] * ld[j];

        total_weight += weight;
    }
    // cout << "COVAR:\n";
    for (int i = 0; i < NDIM; i++) {
        for (int j = 0; j < NDIM; j++) {
            cov[i][j] /= (total_weight * total_weight);
            // cout << cov[i][j] << " ";
        }
        // cout << endl;
    }
    // cout << endl;
}


/* #####   Accesssors   ############################################################# */

codestring OctreePoint::getAddress() const {
    return address;
}

vector<OctreePoint *> OctreePoint::getNeighbors() {
    return neighbors;
}

OctreePoint *OctreePoint::getNeighbor(int i) const {
    return neighbors[i];
}

OctreePoint *searchVector(const vector<OctreePoint *> &v, codestring c) {
    int begin = 0, end = v.size() - 1;

    while (end > begin) {
        int curr = (end + begin) / 2;
        if (c <= v[curr]->address)
            end = curr;
        if (c >= v[curr]->address)
            begin = curr;
    }

    if (c == v[begin]->address)
        return v[begin];
    else return NULL;
}


OctreePoint *OctreePoint::getNeighbor(const int relative_coords[NDIM]) const {
    long new_int_location[NDIM];
    sum(home->int_location, relative_coords, new_int_location);

    codestring c = locationToCode(new_int_location, home->max_depth);
    return searchVector(neighbors, c);
}

int OctreePoint::getIndex() const {
    return index;
}

const double *OctreePoint::getLocation() const {
    return location;
}

const double *OctreePoint::getNomLocation() const {
    return nom_location;
}

const float *OctreePoint::getNormal() const {
    return normal;
}

double &OctreePoint::getLocation(int i) {
    return location[i];
}

double &OctreePoint::getNomLocation(int i) {
    return nom_location[i];
}

int OctreePoint::getDepth() const {
    return depth;
}

/* #####   I/O   #################################################################### */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ostream& operator<<(ostream&, const OctreePoint&)
 *  Description:  Print out point details
 * =====================================================================================
 */
ostream &operator<<(ostream &out, const OctreePoint &p) {
    out << "OctreePoint " << &p << ", index=" << p.index << endl;
    out << "\twith neighbors ";
    for (unsigned int i = 0; i < p.neighbors.size(); i++)
        if (p.neighbors[i] != 0)
            out << p.neighbors[i] << " ";
    out << endl;
    return out;
}


/* #####   HELPER FUNCTION DEFINITIONS   ############################################ */

float point_distance(const OctreePoint *p1, const OctreePoint *p2) {
    return l2Dist(p1->location, p2->location);
}