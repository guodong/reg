#include "generator.h"
#include <sys/stat.h>
#include <sys/mount.h>
#include <ftw.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
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

std::string nowdir;
std::string dest;

inline bool FileExists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

int static Gendirs() {
  int result;
  result = mkdir(".reg", 0755);
  result = mkdir(".reg/lower", 0755);
  result = mkdir(".reg/upper", 0755);
  result = mkdir(".reg/work", 0755);
  result = mkdir(".reg/mount", 0755);
  mkdir(".reg/lower/bin", 0755);
  mkdir(".reg/lower/lib", 0755);
  mkdir(".reg/lower/lib64", 0755);
  system("mkdir -p .reg/lower/usr/lib");
  //mkdir(".reg/lower/usr/lib", 0755);
 
  return 0;
}


static int linkitem(const char *fpath, const struct stat *sb,
    int tflag, struct FTW *ftwbuf)
{

  if (tflag == FTW_D) {

    system(("mkdir -p " + nowdir + "/.reg/lower/" + dest + "/" + fpath).c_str());

  } else {

    link(fpath, (nowdir + "/.reg/lower/" + dest + "/" + fpath).c_str());
  }
  return 0;           
}

// 1. Download ret from github
// 2. Parse reg.yaml if exist
// 3. link
// 4. Install dependencies
int InstallEnv(Env *env) {

  int result;

  std::string dst = "/var/lib/envhub/envs/" + name + "/" + version + "/";

  if (!Util::FileExists(dst)) {
    result = system(("mkdir -p " + dst).c_str());

    if (result != 0) {
      std::cout << "make env repo dir error" << std::endl;
      return -1;
    }
  }
  
  if (!FileExists(dst + ".git")) {
    std::string giturl = "https://github.com/envhub/" + name + ".git";
  
    // Clone exact version branch with depth=1
    // eg. git clone --depth=1 -b 3.5.2-x86_64 https://github.com/reg-ret/python.git /var/lib/reg/ret/python/3.5.2
    result = system(("git clone --depth=1 -b " + version + "-" + arch + " " + giturl + " " + dst).c_str());
    if (result != 0) {
      std::cout << "git clone error" << std::endl;
      return -1;
    }
  }

  if (FileExists(dst + "reg.yaml")) { // bin template
    YAML::Node config = YAML::LoadFile(dst + "reg.yaml");

    YAML::Node files = config["files"];

    for (YAML::const_iterator ifile = files.begin(); ifile != files.end(); ++ifile) {
      struct stat sb;
      stat((dst + ifile->first.as<std::string>()).c_str(), &sb);
      bool isdir = S_ISDIR(sb.st_mode);
      if (isdir) {
        dest = ifile->second.as<std::string>();
        
        char _nowdir[100];
        getcwd(_nowdir, sizeof(_nowdir));
        nowdir = _nowdir;
        chdir((dst + ifile->first.as<std::string>()).c_str());
        nftw(".", linkitem, 20, 0);
        chdir(_nowdir);

      } else {
        link((dst + ifile->first.as<std::string>()).c_str(), (".reg/lower" + ifile->second.as<std::string>()).c_str());
      }
    }
    
    YAML::Node deps = config["deps"];
    std::string depn, depv;
    for (YAML::const_iterator it = deps.begin(); it != deps.end(); ++it) {
      std::vector<std::string> retv = reg::Split(it->as<std::string>(), '@');
      depn = retv[0];
      if (retv.size() == 1) {
        depv = "latest";
      } else {
        depv = retv[1];
      }

      // Recursivelly install dependencies
      InstallEnv(depn, depv, arch);
    }
  } else { // lib template
    //link((dst + name + ".so." + version).c_str(), (".reg/lower/lib/" + name + ".so." + version).c_str());
  }

  return 0;
}

Generator::Generator() {
  env_ = new Env();
}

int Generator::Init() {
  // generate dirs to store layers
  int result;

  if (!Util::FileExists("/var/lib/envhub")) {
    result = mkdir("/var/lib/envhub", 0755);
    if (result != 0) {
      goto mkdirerr;
    }
  }

  if (!Util::FileExists("/var/lib/envhub/envs")) {
    result = mkdir("/var/lib/envhub/envs", 0755);
    if (result != 0) {
      goto mkdirerr;
    }
  }
  
  return 0;

 mkdirerr:
  std::cout << "mkdir err" << std::endl;
  return -1;
}

Env* Generator::Generate(const std::vector<std::string> rets) {
  Env *env = new Env();
  int result;
  for (unsigned i = 0; i < rets.size(); i++) {
    result = Generate(rets.at(i));
    if (result != 0) {
      return NULL;
    }
  }
  
  return env;
}

Env* Generator::Generate(const std::string &ret) {
  Env *env = new Env();
  
  int result;
  result = Init();
  if (result != 0) {
    return -1;
  }
  
  Gendirs();

  Env *dep_env = FetchEnv(ret);
  env->AddDependency(dep_env);
  
  return env;
}

Env* Generator::FetchEnv(const std::string &env_name) {
  Env *env = new Env();
  std::vector<std::string> retv = reg::Split(env_name, '@');

  std::string name = retv[0];
  std::string version;

  
  if (retv.size() == 1) {
    version = "latest";
  } else {
    // ret releases name format is $version-x86[_64]
    version = retv[1];
  }

  env->set_name(name);
  env->set_version(version);

  /*  if (!Util::FileExists(envroot() + "/lower/lib64/ld-linux-x86-64.so.2")) {
    result = link("/lib/x86_64-linux-gnu/ld-2.23.so", (envroot() + "/lower/lib64/ld-linux-x86-64.so.2").c_str());
    if (result != 0) {
      std::cout << "link ld err " << result << std::endl;
      return -1;
    }
  }
  */

  int result; // = InstallEnv(env, "x86_64");

  std::string dst = "/var/lib/envhub/envs/" + name + "/" + version + "/";

  if (!Util::FileExists(dst)) {
    result = system(("mkdir -p " + dst).c_str());

    if (result != 0) {
      std::cout << "make env repo dir error" << std::endl;
      return -1;
    }
  }
  
  if (!FileExists(dst + ".git")) {
    std::string giturl = "https://github.com/envhub/" + name + ".git";
  
    // Clone exact version branch with depth=1
    // eg. git clone --depth=1 -b 3.5.2-x86_64 https://github.com/reg-ret/python.git /var/lib/reg/ret/python/3.5.2
    result = system(("git clone --depth=1 -b " + version + "-" + arch + " " + giturl + " " + dst).c_str());
    if (result != 0) {
      std::cout << "git clone error" << std::endl;
      return -1;
    }
  }

  if (FileExists(dst + "reg.yaml")) { // bin template
    YAML::Node config = YAML::LoadFile(dst + "reg.yaml");

    YAML::Node files = config["files"];

    for (YAML::const_iterator ifile = files.begin(); ifile != files.end(); ++ifile) {
      struct stat sb;
      stat((dst + ifile->first.as<std::string>()).c_str(), &sb);
      bool isdir = S_ISDIR(sb.st_mode);
      if (isdir) {
        dest = ifile->second.as<std::string>();
        
        char _nowdir[100];
        getcwd(_nowdir, sizeof(_nowdir));
        nowdir = _nowdir;
        chdir((dst + ifile->first.as<std::string>()).c_str());
        nftw(".", linkitem, 20, 0);
        chdir(_nowdir);

      } else {
        link((dst + ifile->first.as<std::string>()).c_str(), (".reg/lower" + ifile->second.as<std::string>()).c_str());
      }
    }
    
    YAML::Node deps = config["deps"];
    std::string depn, depv;
    for (YAML::const_iterator it = deps.begin(); it != deps.end(); ++it) {
      std::vector<std::string> retv = reg::Split(it->as<std::string>(), '@');
      depn = retv[0];
      if (retv.size() == 1) {
        depv = "latest";
      } else {
        depv = retv[1];
      }

      // Recursivelly install dependencies
      InstallEnv(depn, depv, arch);
    }
  } else { // lib template
    //link((dst + name + ".so." + version).c_str(), (".reg/lower/lib/" + name + ".so." + version).c_str());
  }


  if (result != 0) {
    std::cout << "install env error" << std::endl;
    return NULL;
  }
  
  return env;
}

}
