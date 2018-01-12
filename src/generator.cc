#include "generator.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <uuid/uuid.h>
#include <yaml-cpp/yaml.h>
#include <curl/curl.h>
#include "configuration.h"
#include "parser.h"
#include "installer.h"
#include "runtime_env.h"
#include "util.h"

#define RUNTIME_DIR ".reg"
#define TMP_DIR "/tmp/"

namespace reg {

int Generator::generate(const std::string& ret, const std::string& version, RuntimeEnv *re) {

  std::string tmp_name = TMP_DIR + ret + version + ".tar.gz";
  reg::Download("https://github.com/reg-ret/" + ret + "/archive/" + version + "-x86_64.tar.gz", tmp_name);
  reg::Extract(tmp_name.c_str());
  //reg::Dld();
  
  return 0;
  /**
   * use uuid as runtime env id
   **/
  uuid_t uuid;
  char str[36];
  uuid_generate(uuid);
  uuid_unparse(uuid, str);
  std::string uuidstr(str);

  re->set_id(uuidstr);

  std::cout << re->id() << std::endl;

  std::string id(re->id());
  /*
  mkdir(".reg");
  mkdir(".reg/lower");
  mkdir(".reg/upper");
  mkdir(".reg/work");
  mkdir(".reg/mount");

  chdir("/var/lib/reg/rets");
  mkdir(ret.c_str());
  chdir(ret.c_str());
  */
  /**
   * make fs
   **
  mkdir(("/var/lib/reg/runtimes/" + id).c_str(), 0755);
  mkdir(("/var/lib/reg/runtimes/" + id + "/lower").c_str(), 0755);
  mkdir(("/var/lib/reg/runtimes/" + id + "/upper").c_str(), 0755);
  mkdir(("/var/lib/reg/runtimes/" + id + "/work").c_str(), 0755);
  mkdir(("/var/lib/reg/runtimes/" + id + "/mount").c_str(), 0755);
  mkdir(("/var/lib/reg/runtimes/" + id + "/src").c_str(), 0755);
  */
  //system(("cp -rf .git /var/lib/reg/runtimes/" + id + "/src").c_str());

  //chdir(("/var/lib/reg/runtimes/" + id + "/src").c_str());
  //system("git checkout -f master");

  /**
   * install libs and bins
   **/
  //reg::Parser::Parse("reg.yaml", config);

  chdir(("/var/lib/reg/runtimes/" + id).c_str());
  mkdir("./lower/bin", 0755);
  mkdir("./lower/lib", 0755);
  mkdir("./lower/lib64", 0755);

  link("/lib/x86_64-linux-gnu/ld-2.19.so", "./lower/lib64/ld-linux-x86-64.so.2");

  std::map<std::string, std::string>::iterator it;
  //it = config->bins().begin();

  //std::cout << config->bins().size() << std::endl;

  YAML::Node config = YAML::LoadFile("src/reg.yaml");
  re->set_cmd(config["cmd"].as<std::string>());

  YAML::Node bins = config["bins"];
  for (YAML::const_iterator itx = bins.begin(); itx != bins.end(); ++itx) {
    re->InstallBin(itx->first.as<std::string>(), itx->second.as<std::string>());
  }
  
  /*
  while (it != config->bins().end()) { std::cout << "fk" << std::endl;
  //for (; it != config->bins().end(); it++) { std::cout<<"fk1"<<std::endl;
    re->InstallBin(it->first, it->second);
    it++;
  }
  */
  return 0;
}
}
