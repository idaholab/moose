//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "CSGUtils.h"

using namespace CSG;

namespace CSGUtils
{

CSGRegion
getInnerRegion(const std::vector<std::reference_wrapper<const CSGSurface>> & radial_surfaces,
               const libMesh::Point & origin)
{
  CSGRegion inner_region;
  for (const auto & surf_ref : radial_surfaces)
  {
    const auto & surf = surf_ref.get();
    const auto direction = surf.getHalfspaceFromPoint(origin);
    auto halfspace = (direction == CSGSurface::Halfspace::POSITIVE) ? +surf : -surf;
    inner_region = (inner_region.getRegionType() == CSGRegion::RegionType::EMPTY)
                       ? halfspace
                       : inner_region & halfspace;
  }
  return inner_region;
}
}
