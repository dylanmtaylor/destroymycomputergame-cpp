#include "common.h"
#include <dirent.h> //for scanning directory contents

bool fileExists(const char *filename) {
    return (access(filename, F_OK) != -1);
}

std::vector<std::string> getDirectoryContents(std::string path) {
    //This is implemented in a cross-platform compatible way.
    //There is no need to change this. :)
    std::vector<std::string> directory;
    DIR *dir;
    struct dirent *ent;
    printf("Scanning directory \"%s\"...\n",path.c_str());
    dir = opendir(path.c_str());
    if (dir != NULL) {
      // add all the files and directories within directory to the vector
      std::string full_path;
      while ((ent = readdir(dir)) != NULL) {
          full_path = path + "/" + ent->d_name;
          if (strstr(full_path.c_str(),"/..") == NULL && strstr(full_path.c_str(),"/.") == NULL) {
                directory.push_back(full_path);
                println(full_path.c_str());
          };
      }
      closedir(dir);
    }
    return directory;
}
