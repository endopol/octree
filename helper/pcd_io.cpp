#include "octree.h"
#include <fstream>
#include <string>
#include <sstream>
#include <time.h>
#include <string.h>
using namespace std;

bool load_coords(ifstream &f, int num_fields, const int *field_map, double *points, float *normals)
{
    double trash;
    string line;
    getline(f, line);
    istringstream lss(line);

    for (int i = 0; i < num_fields; i++)
        if (field_map[i] >= 0 && field_map[i] < 2 * NDIM)
        {
            if (field_map[i] < NDIM)
            {
                lss >> points[field_map[i]];
                // cout << points[field_map[i]] << " ";
            }
            else
                lss >> normals[field_map[i] - NDIM];
        }
        else lss >> trash;
    // cout << endl;

    return f.good();
}

bool read_points(ifstream &f, int num_pts, int num_fields, const int *field_map, double *points, float *normals)
{
    bool good = true;
    for (int i = 0; i < num_pts; i++)
        good = good && load_coords(f, num_fields, field_map, &points[NDIM * i], &normals[NDIM * i]);
    return good;
}

const char FIELD_NAMES[6][10] = {"x", "y", "z", "nx", "ny", "nz"};
bool read_pcd_header(ifstream &f, int &num_pts, int &num_fields, int *field_map, bool &haveNormals)
{
    int position=-1;
    string line, heading;
    bool inHeader = true;
    double numtest;

    haveNormals = false;
    while (inHeader && f.good())
    {
        position = f.tellg();
        getline(f, line);

        istringstream lss(line);
        lss >> heading;

        // Get number of fields
        if (heading.compare("FIELDS") == 0)
        {
            char fieldname[10];
            int fieldnum = 0;
            while (lss >> fieldname)
            {
                field_map[fieldnum] = -1; // unused
                for (int i = 0; i < 2 * NDIM; i++)
                {
                    if (strcmp(fieldname, FIELD_NAMES[i]) == 0)
                    {
                        field_map[fieldnum] = i;
                        if (i >= NDIM)
                            haveNormals = true;
                    }
                }
                fieldnum++;
            }
            num_fields = fieldnum;
            continue;
        }

        // Get number of points
        if (heading.compare("POINTS") == 0)
        {
            lss >> num_pts;
            continue;
        }

        // Check for end of header
        istringstream hss(heading);
        if (hss >> numtest)
        {
            inHeader = false;
            break;
        }
    }

    f.seekg(position);
    return f.good();
}


bool read_ply_header(ifstream &f, int &num_pts, int &num_fields, int *field_map, bool &haveNormals)
{
    string line, heading, type;
    bool inHeader = true, inVertex = false;
    for (int i = 0; i < 6; i++)
        field_map[i] = -1;

    int fieldnum = 0;

    haveNormals = false;
    while (inHeader && f.good())
    {
        getline(f, line);

        istringstream lss(line);
        lss >> heading;

        // Get number of vertices
        if (heading.compare("element") == 0)
        {
            lss >> type;
            if (type.compare("vertex") == 0)
            {
                inVertex = true;
                lss >> num_pts;
            }
            else inVertex = false;
            continue;
        }

        // Get number of fields
        if ((heading.compare("property") == 0) && inVertex)
        {
            char fieldname[10];
            lss >> type;
            if (lss >> fieldname)
            {
                for (int i = 0; i < 6; i++)
                {
                    if (strcmp(fieldname, FIELD_NAMES[i]) == 0)
                    {
                        field_map[fieldnum] = i;
                        if (i >= 3)
                            haveNormals = true;
                    }
                }
                fieldnum++;
            }
            num_fields = fieldnum;
            continue;
        }

        // Check for end of header
        if (heading.compare("end_header") == 0)
        {
            inHeader = false;
            break;
        }
    }
    return f.good();
}


bool load_points_from_pxx(const char *filename, Octree &tree, OctreeGraph &graph)
{
    int num_points, num_fields = 0, field_map[6];
    double *points;
    float *normals;
    bool haveNormals = false, result = false;

    cout << "\nOpening file " << filename << ".\n";
    ifstream infile;
    infile.open(filename);
    if (infile.fail())
    {
        cout << "ERROR:  File not found.\n";
        return false;
    }

    if (strcmp(&filename[strlen(filename) - 3], "pcd") == 0)
    {
        cout << "Reading PCD header.\n";
        result = read_pcd_header(infile, num_points, num_fields, field_map, haveNormals);
    }
    if (strcmp(&filename[strlen(filename) - 3], "ply") == 0)
    {
        cout << "Reading PLY header.\n";
        result = read_ply_header(infile, num_points, num_fields, field_map, haveNormals);
    }

    if (!result)
    {
        cout << "ERROR:  Could not read header.\n";
        return false;
    }

    points = new double[num_points * NDIM]();
    normals = new float[num_points * NDIM]();

    // Read points from the file
    clock_t t1 = clock();
    cout << "\nReading " << num_points << " points ";
    if (haveNormals) cout << "(with normals):  "; else cout << "(without normals):  ";
    read_points(infile, num_points, num_fields, field_map, points, normals);
    cout << "DONE";
    clock_t t2 = clock();
    cout << " (" << (int)((t2 - t1) * (((double) 1000) / CLOCKS_PER_SEC)) << "ms).\n";

    // Add points to the tree
    clock_t t3 = clock();
    cout << "Building tree (depth=" << tree.max_depth << "):  ";
    if (haveNormals)
        tree.addPoints(points, normals, num_points, graph);
    else
        tree.addPoints(points, NULL, num_points, graph);
    cout << "DONE";
    clock_t t4 = clock();
    cout << " (" << (int)((t4 - t3) * (((double) 1000) / CLOCKS_PER_SEC)) << "ms).\n";
    cout << "New tree has " << graph.getNumVertices() << " leaf-nodes and " << graph.getNumEdges() << " edges.\n";


    delete[] points;
    delete[] normals;
    return true;
}
