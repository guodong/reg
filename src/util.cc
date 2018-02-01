#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <cmath>
#include <sstream>
#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>

using std::string;
using std::vector;

namespace reg {

template<typename Out>
void Split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<std::string> Split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    Split(s, delim, std::back_inserter(elems));
    return elems;
}

bool Util::FileExists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

namespace util {
std::string ArchString() {
  std::string arch = "x86_64";
  return arch;
}
}

}
