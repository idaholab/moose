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

std::shared_ptr<CSGUniverse>
CSGBase::createRootUniverse(const std::string name)
{
  if (_root_universe)
    mooseError("Root universe for this mesh has already been created.");
  _root_universe = std::make_shared<CSGUniverse>(name);
  return _root_universe;
}

std::shared_ptr<CSGUniverse>
CSGBase::getRootUniverse() const
{
  if (!_root_universe)
    mooseError("Cannot retrieve root universe before it is initialized.");
  return _root_universe;
}

CSGBase::CSGBase() : _surface_list(CSGSurfaceList()) {}

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
  for (const auto & c : all_cells)
  {
    const auto cell_ptr = c.second;
    const auto cell_name = cell_ptr->getName();
    const auto cell_region = cell_ptr->getRegionAsString();
    const auto cell_filltype = cell_ptr->getFillTypeString();
    csg_json["CELLS"][cell_name]["FILLTYPE"] = cell_filltype;
    csg_json["CELLS"][cell_name]["REGION"] = cell_region;
    if (cell_filltype == "MATERIAL")
    {
      const auto matcell_ptr = std::static_pointer_cast<CSGMaterialCell>(cell_ptr);
      const auto cell_fill = matcell_ptr->getFill();
      csg_json["CELLS"][cell_name]["FILL"] = cell_fill;
    }
  }

  return csg_json;
}
} // namespace CSG
