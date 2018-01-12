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

/*
std::vector<std::string> Split(std::string str,std::string sep) {
  char* cstr=const_cast<char*>(str.c_str());
  char* current;
  std::vector<std::string> arr;
  current=strtok(cstr,sep.c_str());
  while(current!=NULL){
    arr.push_back(current);
    current=strtok(NULL,sep.c_str());
  }
  return arr;
}

void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c) {
  std::string::size_type pos1, pos2;
  pos2 = s.find(c);
  pos1 = 0;
  while(std::string::npos != pos2)
  {
    v.push_back(s.substr(pos1, pos2-pos1));
 
    pos1 = pos2 + c.size();
    pos2 = s.find(c, pos1);
  }
  if(pos1 != s.length())
    v.push_back(s.substr(pos1));
}

void StringToNameVersion(const std::string& str, std::string& name, std::string& version) {
  std::vector<std::string> arr = Split(str, "@");
  if (arr.size() == 1) {
    name = str;
    version = "latest";
  } else {
    name = arr.at(0);
    version = arr.at(1);
  }
}
*/
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {  
  size_t written = fwrite(ptr, size, nmemb, stream);  
  return written;  
} 

int Download(const std::string &url, const std::string &dst) {
  CURL *curl;
  CURLcode res;
  FILE *fp;

  res = curl_global_init(CURL_GLOBAL_ALL);
  if (CURLE_OK != res) {
    curl_global_cleanup();
    return -1;
  }

  curl = curl_easy_init();
  if (curl) {
    fp = fopen(dst.c_str(), "wb");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cout << "curl error " << res << std::endl;
    }

    fclose(fp);

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  return 0;
    
}

static int copy_data(struct archive *ar, struct archive *aw) {
  int r;
  const void *buff;
  size_t size;
  int64_t offset;

  for (;;) {
    r = archive_read_data_block(ar, &buff, &size, &offset);
    if (r == ARCHIVE_EOF)
      return (ARCHIVE_OK);
    if (r < ARCHIVE_OK)
      return (r);
    r = archive_write_data_block(aw, buff, size, offset);
    if (r < ARCHIVE_OK) {
      fprintf(stderr, "%s\n", archive_error_string(aw));
      return (r);
    }
  }
}


void Extract(const char *filename) {
  struct archive *a;
  struct archive *ext;
  struct archive_entry *entry;
  int flags;
  int r;

  /* Select which attributes we want to restore. */
  flags = ARCHIVE_EXTRACT_TIME;
  flags |= ARCHIVE_EXTRACT_PERM;
  flags |= ARCHIVE_EXTRACT_ACL;
  flags |= ARCHIVE_EXTRACT_FFLAGS;

  a = archive_read_new();
  archive_read_support_format_all(a);
  archive_read_support_compression_all(a);
  ext = archive_write_disk_new();
  archive_write_disk_set_options(ext, flags);
  archive_write_disk_set_standard_lookup(ext);
  if ((r = archive_read_open_filename(a, filename, 10240)))
    exit(1);
  for (;;) {
    r = archive_read_next_header(a, &entry);
    if (r == ARCHIVE_EOF)
      break;
    if (r < ARCHIVE_OK)
      fprintf(stderr, "%s\n", archive_error_string(a));
    if (r < ARCHIVE_WARN)
      exit(1);
    r = archive_write_header(ext, entry);
    if (r < ARCHIVE_OK)
      fprintf(stderr, "%s\n", archive_error_string(ext));
    else if (archive_entry_size(entry) > 0) {
      r = copy_data(a, ext);
      if (r < ARCHIVE_OK)
        fprintf(stderr, "%s\n", archive_error_string(ext));
      if (r < ARCHIVE_WARN)
        exit(1);
    }
    r = archive_write_finish_entry(ext);
    if (r < ARCHIVE_OK)
      fprintf(stderr, "%s\n", archive_error_string(ext));
    if (r < ARCHIVE_WARN)
      exit(1);
  }
  archive_read_close(a);
  archive_read_free(a);
  archive_write_close(ext);
  archive_write_free(ext);
  exit(0);
}


}
