#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <map>
#include "octree.h"
using namespace std;

// #### VARIABLES DECLARED HERE
vector<string> PLY_NAMES;
double LIMS[6] = {-1,1, -1,1, -1,1};
int DEPTH = 10;

int FOOT = 2, DIAM, NNEI;
double COVAR_SIGMA = 1;

// #### VARIABLES FOR VISUALIZATION
enum edge_enum {NONE, NORMALS, GRAPH};
edge_enum edgetype = NONE;
enum loc_enum {AVERAGE, NOMINAL};
loc_enum loctype = AVERAGE;


// #### INITIALIZATION OF GLOBALS
std::map<std::string, std::string> options;
typedef std::map<std::string, std::string>::iterator optit;
template<typename T>
bool init_var(string varname, T& var);
template<typename T>
bool init_arr(string arrname, T* array, int numel);
template<typename T>
bool init_vec(string vecname, vector<T>& v);

void init_globals(){
	init_var("FOOT", FOOT);
	DIAM = 1+2*FOOT; /* 1+2*FOOT */
	NNEI = (int) pow(DIAM, NDIM);	
	init_var("COVAR_SIGMA", COVAR_SIGMA);

	init_arr("LIMS", LIMS, 6);
	init_var("DEPTH", DEPTH);
	init_vec("PLY_NAMES", PLY_NAMES);
}
void init_viz(){
	int temp = 0;
	init_var("edgetype", temp); edgetype = (edge_enum) temp;
	init_var("loctype", temp); loctype = (loc_enum) temp;
}


// ################## PARSING FUNCTIONS ##################
void dump_stream(istringstream& ss, string& s);
string trim_whitespace(string s);
void parse_stream(std::ifstream & cfgfile)
{    
    std::string line, id, eq, val;

    while(getline(cfgfile, line))
    {    	
    	int eq_index = line.find_first_of("=");
    	id = trim_whitespace(line.substr(0,eq_index-1));
    	if(id[0]=='#') continue;

    	val = trim_whitespace(line.substr(eq_index+1, -1));
    	// cout << id << ": " << val << endl;
      	options[id] = val;
    }

    init_globals();
}
void parse_globals(const char* filename){    
	ifstream infile(filename);
    parse_stream(infile);
    infile.close();
}


string trim_whitespace(string s){
	string ignore_chars = " ;";
	int begin = s.find_first_not_of(ignore_chars),
		end = s.find_last_not_of(ignore_chars);
	if(begin<0) return "";
	// cout << "begin=" << begin << ", end=" << end << endl;
	return s.substr(begin, end-begin+1);
}
void dump_stream(istringstream& ss, string& s){
	string word;
	s = "";
	while(ss >> word)
		s += word + " ";
}



// ################## ARGUMENT PARSING FUNCTIONS ##################
template<typename T>
bool init_vec(string vecname, vector<T>& v){
	// cout << vecname << ": \n";
	optit temp = options.find(vecname);
	if (temp != options.end()) {
		string s = temp->second;
		int begin, end = 0;
		string ignore_chars = " ,()[]{}";
		while (((unsigned int) end) < s.length()) {
			begin = s.find_first_not_of(ignore_chars, end);
			if (begin == -1) return true;

			end = s.find_first_of(ignore_chars, begin);
			istringstream nextnum_in(s.substr(begin, end - begin));
			T next_num;
			nextnum_in >> next_num;
			// cout << next_num << endl;
			v.push_back(next_num);
		}
		return true;
	}
	else return false;
}

template<typename T>
bool init_arr(string arrname, T* array, int numel){
	// cout << arrname << ": \n";
	optit temp = options.find(arrname);
	if (temp != options.end()) {
		string s = temp->second;
		int begin, end = 0, index = 0;
		string ignore_chars = " ,()[]{}";
		while (((unsigned int) end) < s.length() && index < numel) {
			begin = s.find_first_not_of(ignore_chars, end);
			if (begin == -1) return true;

			end = s.find_first_of(ignore_chars, begin);
			istringstream nextnum_in(s.substr(begin, end - begin));
			nextnum_in >> array[index];
			// cout << array[index] << endl;
			index++;
		}
		return true;
	}
	else return false;
}

template<typename T>
bool init_var(string varname, T& var){
	return init_arr(varname, &var, 1);
}

#endif // GLOBALS_H