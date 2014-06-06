#ifndef EXODIFF_UTIL_H
#define EXODIFF_UTIL_H

#define TOPTR(x) (x.empty() ? NULL : &x[0])

char **get_name_array(int size, int length);
void free_name_array(char **names, int size);
#endif
