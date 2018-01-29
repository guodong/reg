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

    TCLAP::UnlabeledValueArg<std::string> action_("action", "reg actions", true, "", "generate, remove, start, stop, install, uninstall", cmd);

    TCLAP::SwitchArg parallel_("p", "parallel", "execute command simultaneously", false);
    cmd.add(parallel_);

    TCLAP::ValueArg<std::string> envroot_("d", "envdir", "env root dir", false, ".env", "string");

    TCLAP::UnlabeledMultiArg<std::string> params_("params", "reg params", false, "string");
    cmd.add(params_);

    cmd.parse(argc, argv);

    std::vector<std::string> params = params_.getValue();

    // eg. sudo reg generate python
    if (action_.getValue() == "generate") {
      if (params.size() == 0) {
        std::cout << "no templates specified!" << std::endl;
      }
      
      int res;
      reg::Generator gen;
      gen.set_envroot(envdir_.getValue());

      reg::Env *env = new Env();

      env.set_root(envroot_.getValue());
      
      
      reg::Env *env = gen.Generate(params);
      if (!env) {
        std::cout << "gen err" << std::endl;
        return -1;
      }

    } else if (action_.getValue() == "start") {
      if (params.size() == 0) {
        std::cout << "no commands specified!" << std::endl;
      }
      
      reg::RuntimeEnv* re = new reg::RuntimeEnv();
      re->set_cmds(params);
      re->set_parallel(parallel_.getValue());
      re->Start();
    }
    
    
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
  }

  return 0;

  
}
