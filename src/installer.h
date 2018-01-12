#ifndef REG_INSTALLER_H_
#define REG_INSTALLER_H_

#define REG_BINS_DIR "/var/lib/reg/bins/"
#define REG_LIBS_DIR "/var/lib/reg/libs/"

#include <string>

class Installer;

namespace reg {
class Installer {
 public:
  Installer();
  ~Installer();
  
  void ParseConfig(std::string);
  static int InstallBin(std::string name, std::string version);



};
}

#endif
