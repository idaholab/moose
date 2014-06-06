#include "util.h"

#include <cstring>
#include "exodusII.h"

char **get_name_array(int size, int length) {
  char **names = NULL;
  if (size > 0) {
    names = new char* [size];
    for (int i=0; i < size; i++) {
      names[i] = new char [length+1];
      std::memset(names[i], '\0', length+1);
    }
  }
  return names;
}

void free_name_array(char **names, int size) {
  for (int i=0; i < size; i++) {
    delete [] names[i];
  }
  delete [] names;
  names = NULL;
}
