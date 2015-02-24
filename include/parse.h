#ifndef PARSE_H
#define PARSE_H

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <map>

global std::map<std::string, std::string> options;

void parse(std::ifstream & cfgfile)
{
    std::string id, eq, val;

    while(cfgfile >> id >> eq >> val)
    {
      if (id[0] == '#') continue;  // skip comments
      if (eq != "=") throw std::runtime_error("Parse error");

      options[id] = val;
    }
}

#endif // PARSE_H