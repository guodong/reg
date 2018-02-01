#ifndef REG_UTIL_H_
#define REG_UTIL_H_

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace reg {

template<typename Out>
void Split(const std::string &s, char delim, Out result);
std::vector<std::string> Split(const std::string &s, char delim);

//vector<string> Split(string s, string seperator);
//void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c);

/**
 * parse string to name and version
 * eg. python@2.7 -> python 2.7
 * eg. python -> python latest
 **/
//void StringToNameVersion(const std::string& str, std::string& name, std::string& version);

int Download(const std::string &url, const std::string &dst);
void Extract(const char *filename);
class Util {
 public:
  static bool FileExists(const std::string &file);
};

namespace util {
std::string ArchString();
}
}

#endif
