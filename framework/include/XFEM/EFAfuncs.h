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

template <class T>
static std::vector<T> get_common_elems(std::set<T> &v1, std::set<T> &v2)
{
  std::vector<T> common_elems;
  std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(),
                        std::inserter(common_elems, common_elems.end()));
  return common_elems;
}

double
static linearQuadShape2D(unsigned int node_id, std::vector<double> &xi_2d)
{
  double node_xi[4][2] = {{-1.0,-1.0},{1.0,-1.0},{1.0,1.0},{-1.0,1.0}};
  return 0.25*(1.0 + node_xi[node_id][0]*xi_2d[0])*(1.0 + node_xi[node_id][1]*xi_2d[1]);
}

double
static linearTrigShape2D(unsigned int node_id, std::vector<double> &xi_2d)
{
  std::vector<double> area_xi(3,0.0);
  area_xi[0] = xi_2d[0];
  area_xi[1] = xi_2d[1];
  area_xi[2] = 1.0 - xi_2d[0] - xi_2d[1];
  return area_xi[node_id];
}

double
static linearHexShape3D(unsigned int node_id, std::vector<double> &xi_3d)
{
  double node_xi[8][3] = {{-1.0,-1.0,-1.0},{1.0,-1.0,-1.0},{1.0,1.0,-1.0},{-1.0,1.0,-1.0},
                          {-1.0,-1.0, 1.0},{1.0,-1.0, 1.0},{1.0,1.0, 1.0},{-1.0,1.0, 1.0}};
  return 0.125*(1.0 + node_xi[node_id][0]*xi_3d[0])*(1.0 + node_xi[node_id][1]*xi_3d[1])*(1.0 + node_xi[node_id][2]*xi_3d[2]);
}

double
static linearTetShape3D(unsigned int node_id, std::vector<double> &xi_3d)
{
  std::vector<double> vol_xi(4,0.0);
  for (unsigned int i = 0; i < 3; ++i)
    vol_xi[i] = xi_3d[i];
  vol_xi[3] = 1.0 - xi_3d[0] - xi_3d[1] - xi_3d[2];
  return vol_xi[node_id];
}

#endif
