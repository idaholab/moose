//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGBase.h"

namespace CSG
{

CSGBase::CSGBase() : _surface_list(CSGSurfaceList()), _cell_list(CSGCellList()), _universe_list(CSGUniverseList()) {}

CSGBase::~CSGBase() {}

nlohmann::json
CSGBase::generateOutput() const
{
  nlohmann::json csg_json;

  csg_json["SURFACES"] = {};
  csg_json["CELLS"] = {};
  csg_json["UNIVERSES"] = {};

  // get all surfaces information
  auto all_surfs = getAllSurfaces();
  for (const auto & s : all_surfs)
  {
    auto surf_obj = s.second;
    auto coeffs = surf_obj->getCoeffs();
    csg_json["SURFACES"][s.first] = {{"TYPE", surf_obj->getSurfaceTypeString()},
                                     {"BOUNDARY", surf_obj->getBoundaryTypeString()},
                                     {"COEFFICIENTS", {}}};
    for (const auto & c : coeffs)
        csg_json["SURFACES"][s.first]["COEFFICIENTS"][c.first] = c.second;
  }

  // Print out cell information. For now we are assuming there is a single
  // root universe in the CSGBase object
  auto root_univ = getRootUniverse();
  auto root_univ_name = root_univ->getName();;
  auto all_cells = root_univ->getAllCells();
  for (const auto & cell_ptr : all_cells)
  {
    const auto cell_name = cell_ptr->getName();
    const auto cell_region = cell_ptr->getRegionAsString();
    const auto cell_filltype = cell_ptr->getFillTypeString();
    const auto fill_name = cell_ptr ->getFillName();
    csg_json["CELLS"][cell_name]["FILLTYPE"] = cell_filltype;
    csg_json["CELLS"][cell_name]["REGION"] = cell_region;
    csg_json["CELLS"][cell_name]["FILL"] = fill_name;
  }

  return csg_json;
}
} // namespace CSG
