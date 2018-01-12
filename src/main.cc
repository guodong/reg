#include <iostream>
#include <cstring>
#include <string>
#include "tclap/CmdLine.h"
#include "configuration.h"
#include "parser.h"
#include "installer.h"
#include "util.h"
#include "generator.h"
#include "runtime_env.h"


int main(int argc, char* argv[]) {

  try {

    TCLAP::CmdLine cmd("REG - Runtime Environment Generator", ' ', "0.0.1");

    TCLAP::UnlabeledValueArg<std::string> command("command", "reg commands", true, "", "install, run, stop, start", cmd);

    TCLAP::ValueArg<std::string> cmdArg("c", "cmdarg", "command to execute", false, "/bin/bash", "string");

    TCLAP::ValueArg<std::string> retArg("t", "retarg", "ret", true, "", "string");

    cmd.add(cmdArg);
    cmd.add(retArg);

    cmd.parse(argc, argv);

    std::cout << command.getValue();

    std::string cmdarg = cmdArg.getValue();
    std::string retarg = retArg.getValue();
    std::cout << retarg;

    std::vector<std::string> retv = reg::Split(retarg, '@');

    std::string ret = retv[0];
    std::string version;

    if (retv.size() == 1) {
      version = "latest";
    } else {
      // ret releases name format is name-v(x.x.x)-x86[_64]
      version = "v" + retv[1];
    }
    

    if (command.getValue() == "run") {
      
      int res;
      reg::RuntimeEnv *re = new reg::RuntimeEnv();
      
      reg::Generator gen;
      res = gen.generate(ret, version, re);
      if (res != 0) {
        std::cout << "gen err" << std::endl;
        return -1;
      }

      //re->Start();
    }
    
    
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }

  return 0;

  
}
