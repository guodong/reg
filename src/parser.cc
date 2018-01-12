#include "parser.h"
#include <iostream>
#include "yaml-cpp/yaml.h"
#include "configuration.h"
#include "util.h"

using std::string;

namespace reg {

void Parser::Parse(string filename, Configuration* configuration) {
  /*
  YAML::Node config = YAML::LoadFile(filename);
  YAML::Node ybins = config["bins"];

  std::string bname;
  std::string version;
  const char div = '@';
  std::string sep = "@";
  
  for (YAML::const_iterator it = ybins.begin(); it != ybins.end(); it++) {
    std::string binname = it->as<std::string>();
        std::cout << binname;
    std::size_t found = binname.find(div);
    if (found == std::string::npos) {
      version = "latest";
    } else {
      //std::vector<std::string> parts;
      //reg::SplitString(binname, parts, "@");
      //std::cout << parts.at(1);
      std::vector<std::string> parts = Split(binname, sep);
      std::cout << parts.at(1);
    
      configuration->AddBin(parts.at(0), parts.at(1));
      std::cout << "addbin" << std::endl;
    }
  }
  */
}
}
