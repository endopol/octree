/*
 * =====================================================================================
 *
 *       Filename:  coded_point.cpp
 *
 *    Description:  Point wrapper for the Octree class
 *
 *        Version:  1.0
 *        Created:  09/26/2013 06:47:10 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Joshua Hernandez (jah), endopol@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */
#include "octree.h"
#include <iomanip>

/* #####   Constructors   ########################################################### */

CodedPoint::CodedPoint(codestring new_code)
{	
    code = new_code;
}

CodedPoint::CodedPoint(const long* new_int_location, int max_depth){
	code = locationToCode(new_int_location, max_depth);
}

CodedPoint::CodedPoint(const double *new_location, const float *new_normal, const double *limits, int max_depth)
{
	good_point=true;

    for (int j = 0; j < NDIM; j++)
    {
    	good_point = good_point && ((location[j]>=limits[2*j])&&(location[j]<=limits[2*j+1]));
    	location[j] = new_location[j];
        double dx = (limits[2 * j + 1] - limits[2 * j]) / (1 << max_depth);
        int_location[j] = floor((location[j] - limits[2 * j]) / dx);    	
        normal[j] = new_normal[j];
    }
    code = locationToCode(int_location, max_depth);
}

CodedPoint::CodedPoint(const double *new_location, const double *limits, int max_depth)
{
	good_point=true;
    for (int j = 0; j < NDIM; j++)
    {
    	good_point = good_point && ((location[j]>=limits[2*j])&&(location[j]<=limits[2*j+1]));
    	location[j] = new_location[j];
        double dx = (limits[2 * j + 1] - limits[2 * j]) / (1 << max_depth);
        int_location[j] = floor((location[j] - limits[2 * j]) / dx); 
        normal[j] = 0;
    }
    code = locationToCode(int_location, max_depth);
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CodedPoint
 *      Method:  CodedPoint()
 * Description:  Construct a default point
 *--------------------------------------------------------------------------------------
 */
CodedPoint::CodedPoint() { code = 0; }

codestring CodedPoint::get_code() { return code; }

/* #####   I/O   #################################################################### */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  void printBinary(T, ostream&)
 *  Description:  Print the binary expansion of a given int
 * =====================================================================================
 */
template<typename T>
void printBinary(T n, ostream &out)
{
    const int PRINT_MAX = 16, SHIFT_MAX = 64, BITS_PER_BYTE = 8;

    /* may need to bitshift large integers in parts */
    int num_bits = sizeof(n) * BITS_PER_BYTE;
    int string_length = min(PRINT_MAX, num_bits);
    T out_bit = 1;
    for (int position = 1; position <= string_length; position += SHIFT_MAX)
    {
        out_bit <<= min(SHIFT_MAX, string_length - position);
    }

    if (n / out_bit > 1)
        out << "...";
    for (int i = 0; i < string_length; i++)
    {
        out << ((out_bit & n) > 0);
        out_bit >>= 1;
    }
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  ostream& operator<<(ostream&, const OctreePoint&)
 *  Description:  Print details of a point
 * =====================================================================================
 */
ostream &operator<<(ostream &out, const CodedPoint &p)
{
    out << "CodedPoint @ (";
    for (int i = 0; i < (NDIM - 1); i++)
        cout << setw(3) << p.location[i] << ", ";
    cout << setw(3) << p.location[NDIM - 1] << "): ";
    cout << "code=" << setw(4) << p.code << " (";

    /* write code in binary format */
    printBinary(p.code, out);
    out << ").";

    return out;
}

/* #####   Helper Functions   ####################################################### */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  void codeToLocation(codestring, int* int)
 *  Description:  Generate coordinates from a given codestring
 * =====================================================================================
 */
void codeToLocation(codestring code, long *location, int max_depth)
{
    for (int i = 0; i < NDIM; i++)
        location[i] = 0;

    codestring new_code = code;
    int out_bit = 1;
    for (int i = 0; i < max_depth; i++)
    {
        for (int j = 0; j < NDIM; j++)
        {
            location[j] |= out_bit * (new_code & 1);
            new_code >>= 1;
        }
        out_bit <<= 1;
    }
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  codestring locationToCode(const int*, int)
 *  Description:  Construct a codestring from location coordinates
 * =====================================================================================
 */
codestring locationToCode(const long *location, int max_depth)
{
    int new_location[NDIM];
    for (int i = 0; i < NDIM; i++)
        new_location[i] = location[i];

    /* Thread the location bits into a single codestring */
    codestring code = 0;
    codestring out_bit = 1;
    for (int i = 0; i < max_depth; i++)
    {
        for (int j = 0; j < NDIM; j++)
        {
            code |= out_bit * (new_location[j] & 1);

            out_bit <<= 1;
            new_location[j] >>= 1;
        }
    }

    return code;
}



/* ===  FUNCTION  ======================================================================
 *         Name:  codestring locationToCodeDestructive(int*, int)
 *  Description:  Construct a codestring from location coordinates, destroying the input
 * =====================================================================================
 */
codestring locationToCodeDestructive(long *location, int max_depth)
{

    /* Thread the location bits into a single codestring */
    codestring code = 0;
    codestring out_bit = 1;
    for (int i = 0; i < max_depth; i++)
    {
        for (int j = 0; j < NDIM; j++)
        {
            code |= out_bit * (location[j] & 1);

            out_bit <<= 1;
            location[j] >>= 1;
        }
    }

    return code;
}

/* #####   Binary Operators   ####################################################### */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  bool operator<(const OctreePoint&, const Octreepoint&)
 *  Description:  Compares two OctreePoints by their code, then their frame
 * =====================================================================================
 */
bool operator<(const CodedPoint &p1, const CodedPoint &p2)
{
    return (p1.code < p2.code); // || (p1.code==p2.code && p1.frame<p2.frame));
}

