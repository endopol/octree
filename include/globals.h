#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <map>

using namespace std;

// GENERAL VARIABLES
double lims[6] = {-1,1, -1,1, -1,1};
int DEPTH = 10;
int FOOT = 3; /* Radius of footprint */
int DIAM = 1+2*FOOT; /* 1+2*FOOT */
int NNEI = (int) pow(DIAM, NDIM);
double COVAR_SIGMA = 5;

// VARIABLES FOR VISUALIZATION
enum edge_enum {NONE, NORMALS, GRAPH};
edge_enum edgetype = NONE;

enum loc_enum {AVERAGE, NOMINAL};
loc_enum loctype = AVERAGE;




std::map<std::string, std::string> options;
typedef std::map<std::string, std::string>::iterator optit;

void parse_double_array(string s, double* array, int numel);
void try_parse_int(string s, int& var);
void try_parse_double(string s, double& var);

void init_globals(){
	// Dimensional parameters
	try_parse_int("FOOT", FOOT);
	DIAM = 1+2*FOOT; /* 1+2*FOOT */
	NNEI = (int) pow(DIAM, NDIM);	

	try_parse_int("DEPTH", DEPTH);
	try_parse_double("COVAR_SIGMA", COVAR_SIGMA);

	optit lims_i = options.find("lims");
	if(lims_i!=options.end()) parse_double_array(lims_i->second, lims, 6);
}

void init_viz(){
	int temp = 0;
	try_parse_int("edgetype", temp); edgetype = (edge_enum) temp;
	try_parse_int("loctype", temp); loctype = (loc_enum) temp;
}




// HELPER FILES
void dump_stream(istringstream& ss, string& s);
void parse(std::ifstream & cfgfile)
{    
    std::string line, id, eq, val;

    while(getline(cfgfile, line))
    {
    	istringstream ls(line);
    	if(ls.str().length()==0) continue;
    	ls >> id;
      	if (id[0] == '#') continue;  // skip comments
      	if(!(ls >> eq)) continue;
      	if (eq != "=") throw std::runtime_error("Parse error");
      	dump_stream(ls, val);
      	if(val.length()==0) continue;

      	options[id] = val;
    }

    init_globals();
}

void parse_globals(const char* filename){    
	ifstream infile(filename);
    parse(infile);
    infile.close();
}

void dump_stream(istringstream& ss, string& s){
	string word;
	s = "";
	while(ss >> word)
		s += word + " ";
}

void parse_double_array(string s, double* array, int numel){
	int begin, end=0, index = 0;
	string ignore_chars = " ,()[]{}";
	while(((unsigned int) end)<s.length() && index<numel){
		begin=s.find_first_not_of(ignore_chars, end);
		if(begin==-1) return;

		end=s.find_first_of(ignore_chars, begin);
		string nextnum = s.substr(begin, end-begin);
		array[index] = std::stod(nextnum);
		index++;
	}
}

void try_parse_int(string s, int& var){
	optit temp = options.find(s);
	if(temp!=options.end()) var = std::stoi(temp->second);

}

void try_parse_double(string s, double& var){
	optit temp = options.find(s);
	if(temp!=options.end()) var = std::stod(temp->second);

}

#endif // GLOBALS_H