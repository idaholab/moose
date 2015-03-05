#ifndef EFAFUNCS_H
#define EFAFUNCS_H

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <algorithm>

template <typename T>
static bool deleteFromMap(std::map<unsigned int, T*> &theMap, T* elemToDelete)
{
  bool didIt = false;
  typename std::map<unsigned int, T*>::iterator i=theMap.find(elemToDelete->id());
  if (i != theMap.end())
  {
    delete i->second;
    theMap.erase(i);
    elemToDelete=NULL;
    didIt = true;
  }
  return didIt;
}

template <typename T>
static unsigned int getNewID(std::map<unsigned int, T*> &theMap)
{
  typename std::map<unsigned int, T*>::reverse_iterator last_elem = theMap.rbegin();
  unsigned int new_elem_id = 0;
  if (last_elem != theMap.rend())
    new_elem_id=last_elem->first+1;
  return new_elem_id;
}

template <class T>
static unsigned int num_common_elems(std::set<T> &v1, std::set<T> &v2)
{
  std::vector<T> common_elems;
  std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                        std::inserter(common_elems, common_elems.end()));
  return common_elems.size();
}

template <class T>
static unsigned int num_common_elems(std::set<T> &v1, std::vector<T> &v2)
{
  std::vector<T> common_elems;
  std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                        std::inserter(common_elems, common_elems.end()));
  return common_elems.size();
}

#endif
