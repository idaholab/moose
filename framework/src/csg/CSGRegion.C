//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGRegion.h"

namespace CSG
{

CSGRegion::CSGRegion(std::shared_ptr<CSGSurface> surf, const CSGSurface::Direction direction, const CSGRegion::Operation region_op)
{
  if (region_op != CSGRegion::Operation::HALFSPACE)
    mooseError("This region constructor requires the region operator to be of type halfspace");
  _region_str = ((direction == CSGSurface::Direction::positive) ? "+" : "-") + surf->getName();
}

CSGRegion::CSGRegion(std::vector<const CSGRegion> & regions, const CSGRegion::Operation region_op)
{
  if (region_op == CSGRegion::Operation::COMPLEMENT)
  {
    if (regions.size() != 1)
      mooseError("Only a single region should be passed when constructing a region defined as a complement.");
    _region_str = "~(" + regions[0].toString() + ")";
  }
  else
  {
    if (regions.size() < 2)
      mooseError("Regions defined as unions or intersections must be comprised of at least 2 sub-regions");

    std::string op = (region_op == CSGRegion::Operation::UNION) ? " | " : " & ";
    _region_str = "(";
    for (const auto i : index_range(regions))
      _region_str += ((i != 0) ? op : "") + regions[i].toString();
    _region_str += ")";
  }
}

std::string
CSGRegion::toString() const
{
  return _region_str;
}
} // namespace CSG
