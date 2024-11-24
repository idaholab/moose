//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGIntersection.h"

namespace CSG
{

CSGIntersection::CSGIntersection() : _nodes(std::vector<CSGHalfspace>()) {}

CSGIntersection::CSGIntersection(std::vector<CSGHalfspace> & nodes) : _nodes(nodes) {}

std::string
CSGIntersection::toString() const
{
  std::string region_str = "";
  if (_nodes.size() > 0)
  {
    for (const auto i : index_range(_nodes))
      region_str += ((i != 0) ? " & " : "") + _nodes[i].toString();
  }
  else
    region_str = "None";
  return region_str;
}
} // namespace CSG
