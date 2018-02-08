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
#include <dirent.h>     
#include <yaml-cpp/yaml.h>
#include <curl/curl.h>
#include "configuration.h"
#include "parser.h"
#include "installer.h"
#include "runtime_env.h"
#include "util.h"
#include "env.h"

#define TMP_DIR "/tmp/"

namespace reg {

std::string nowdir;
std::string dest;

inline bool FileExists (const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

int Generator::Gendirs() {
  int result;
  if (mkdir(envroot_.c_str(), 0755) != 0) {
    goto err;
  }
  if (mkdir((envroot_ + "/upper").c_str(), 0755) != 0) {
    goto err;
  }
  if (mkdir((envroot_ + "/work").c_str(), 0755) != 0) {
    goto err;
  }
  if (mkdir((envroot_ + "/mount").c_str(), 0755) != 0) {
    goto err;
  }

  return 0;

err:
  std::cout << "err to create envroot" << std::endl;
  return -1;
}

int Generator::Deldirs(){
 
 if (access(envroot_.c_str(), 0) == 0)  
  {  
      //cout<<dir<<" exists"<<endl;  
      //cout<<"now delete it"<<endl;
    if(rmdir((envroot_+"/upper").c_str())!=0){
      goto err;
    }
    if(rmdir((envroot_+"/work").c_str())!=0){
      goto err;
    }
    if(rmdir((envroot_+"/mount").c_str())!=0){
      goto err;
    }
      return 0;
  } else{
    std::cout << "no envroot dir to delete" << std::endl;
    return -1;
  }
  err:
  std::cout << "err to remove envroot dir" << std::endl;
  return -1;

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

std::vector<std::string> Generator::GetDependencyDirs(Env *env) {
  std::vector<std::string> overlayfs_lowers;
  //std::cout << (env->dependencies().at(0))->name();

  for (int i = 0; i < env->dependencies().size(); i++) {
    overlayfs_lowers.push_back(env->dependencies().at(i)->source_dir());
    
    if (env->dependencies().at(i)->dependencies().size() != 0) {
      std::vector<std::string> dep_dirs = GetDependencyDirs(env->dependencies().at(i));
      overlayfs_lowers.insert(overlayfs_lowers.end(), dep_dirs.begin(), dep_dirs.end());
    }
  }

  // why iterator doesn't work with segment fault
  /*for (std::vector<Env*>::iterator it = env->dependencies().begin(); it != env->dependencies().end(); ++it) {std::cout<<(*it);
    if (!((*it)->dependencies().empty())) {
      //std::vector<std::string> dep_dirs = GetDependencyDirs(*it);
      //overlayfs_lowers.insert(overlayfs_lowers.end(), dep_dirs.begin(), dep_dirs.end());
    }

    //overlayfs_lowers.push_back(env->source_dir());
  }
  */
  return overlayfs_lowers;
}

int Generator::MakeFs(Env *env) {
  std::vector<std::string> overlayfs_lowers = GetDependencyDirs(env);

  std::stringstream overlayfs_lowers_ss;
  for (int i = 0; i < overlayfs_lowers.size(); ++i) {
    if (i != 0) {
      overlayfs_lowers_ss << ":";
    }
    overlayfs_lowers_ss << overlayfs_lowers.at(i);
  }

  std::string overlayfs_lower_str = overlayfs_lowers_ss.str();

  std::string args = "lowerdir=" + overlayfs_lower_str + ",upperdir=" + envroot_ + "/upper,workdir=" + envroot_ + "/work";
  std::cout << args;
  int result = mount(NULL, (envroot_ + "/mount").c_str(), "overlay", 0, args.c_str());
  if (result != 0) {
    std::cout << "mount error." << std::endl;
    return -1;
  }
  
  for (std::vector<std::string>::iterator it = overlayfs_lowers.begin(); it != overlayfs_lowers.end(); ++it) {
    std::cout << *it << std::endl;
  }

  return 0;
}

Env* Generator::Generate(const std::vector<std::string> rets) {
  Init();
  
  Env *env = new Env();
  int result;
  for (unsigned i = 0; i < rets.size(); i++) {
    Env *dep_env = FetchEnv(rets.at(i));
    if (!dep_env) {
      return NULL;
    }

    env->AddDependency(dep_env);
  }

  Gendirs();

  MakeFs(env);

  // remove unused .git and rtenv.yaml file in mount dir
  system(("rm -rf " + envroot_ + "/mount/.git " + envroot_ + "/mount/rtenv.yaml").c_str());
  
  return env;
}

Env* Generator::Generate(const std::string &env_name) {
  std::vector<std::string> deps;
  deps.push_back(env_name);

  return Generate(deps);
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

  int result; // = InstallEnv(env, "x86_64");

  std::string dst = "/var/lib/envhub/envs/" + name + "/" + version + "/";

  if (!Util::FileExists(dst)) {
    result = system(("mkdir -p " + dst).c_str());

    if (result != 0) {
      std::cout << "make env repo dir error" << std::endl;
      return NULL;
    }
  }

  // if not downloaded
  if (!FileExists(dst + ".git")) {
    std::string giturl = "https://github.com/envhub/" + name + ".git";
  
    // Clone exact version branch with depth=1
    // eg. git clone --depth=1 -b 3.5.2-x86_64 https://github.com/reg-ret/python.git /var/lib/envhub/envs/python/3.5.2
    result = system(("git clone --depth=1 -b " + version + "-" + reg::util::ArchString() + " " + giturl + " " + dst).c_str());
    if (result != 0) {
      std::cout << "git clone error" << std::endl;
      return NULL;
    }
  }

  if (FileExists(dst + "rtenv.yaml")) { // bin template
    YAML::Node config = YAML::LoadFile(dst + "rtenv.yaml");
    YAML::Node deps = config["dependencies"];
    std::string depn, depv;
    for (YAML::const_iterator it = deps.begin(); it != deps.end(); ++it) {
      // Recursivelly install dependencies
      Env *dep_env = FetchEnv(it->as<std::string>());
      env->AddDependency(dep_env);
    }
  }
  
  return env;
}

}
