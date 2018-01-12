#include "installer.h"
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include "yaml-cpp/yaml.h"


using std::cout;
using std::endl;
using std::string;
using std::fstream;
using std::ios;

namespace reg {

Installer::Installer() {
  cout << "inst";
}

int Installer::InstallBin(std::string name, std::string version) {
  
  string oldName = "/var/lib/dygre/bins/" + name + "/" + version + "/" + name;
  string newName = "./lower/bin/" + name;

  int result;
  result = link(oldName.c_str(), newName.c_str());
  if (result != 0) {
    cout << "create bin link error." << endl;
    return -1;
  }

  YAML::Node config = YAML::LoadFile(("/var/lib/dygre/bins/" + name + "/" + version + "/" + "dygre.yaml").c_str());
  YAML::Node binlibs = config["libs"];
  for (YAML::const_iterator itx = binlibs.begin(); itx != binlibs.end(); ++itx) {
    oldName = "/var/lib/dygre/libs/" + (itx)->first.as<string>() + "/" + itx->first.as<string>() + ".so." + itx->second.as<string>();
    newName = "./lib/" + itx->first.as<string>() + ".so." + itx->second.as<string>();

    fstream _file;
    _file.open(newName, ios::in);
    if (!_file) {
      result = link(oldName.c_str(), newName.c_str());
      if (result != 0) {
        cout << "create link error." << endl;
        return -1;
      }
    }
  }

  return 0;
  
  //fstream binfile;
  //  binfile.open(REG_BINS_DIR + name, ios::in);
}

/*void Installer::InstallPackage(std::string name, std::string version) {

  }*/

}
