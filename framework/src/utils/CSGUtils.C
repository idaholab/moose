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
getInnerRegion(const std::vector<std::reference_wrapper<const CSGSurface>> & surfaces,
               const libMesh::Point & origin)
{
  CSGRegion inner_region;
  for (const auto & surf_ref : surfaces)
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

void
checkValidCSGName(const std::string & name)
{
  // Check if invalid symbol is present in the name. These include whitespaces and symbols
  // for halfspaces and region operators
  const std::regex invalid_symbols(R"([\+\-~|& ])");
  std::smatch matches;

  if (std::regex_search(name, matches, invalid_symbols))
  {
    char matched_char = name[matches.position(0)];
    if (matched_char == ' ')
      mooseError("Detected whitespace in CSG component with name ", name, ". This is not allowed.");
    else
      mooseError("Invalid symbol in CSG component with name ", name, ": ", matched_char);
  }
}
}
