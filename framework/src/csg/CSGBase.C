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

CSGBase::CSGBase()
  : _surface_list(CSGSurfaceList()), _cell_list(CSGCellList()), _universe_list(CSGUniverseList())
{
}

CSGBase::~CSGBase() {}

std::shared_ptr<CSGCell>
CSGBase::createCell(const std::string name, const std::string mat_name, const CSGRegion & region)
{
  auto cell = _cell_list.addMaterialCell(name, mat_name, region);
  getRootUniverse()->addCell(cell);
  return cell;
}

std::shared_ptr<CSGCell>
CSGBase::createCell(const std::string name, const CSGRegion & region)
{
  auto cell = _cell_list.addVoidCell(name, region);
  getRootUniverse()->addCell(cell);
  return cell;
}

std::shared_ptr<CSGCell>
CSGBase::createCell(const std::string name, const CSGUniverse & univ, const CSGRegion & region)
{
  auto cell = _cell_list.addUniverseCell(name, univ, region);
  getRootUniverse()->addCell(cell);
  return cell;
}

void
CSGBase::joinSurfaceLists(CSGSurfaceList & surf_list)
{
  auto all_surfs = surf_list.getAllSurfaces();
  for (auto s : all_surfs)
    _surface_list.addSurface(s);
}

void
CSGBase::joinCellList(CSGCellList & cell_list)
{
  auto all_cells = cell_list.getAllCells();
  for (auto c : all_cells)
    _cell_list.addCell(c);
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list)
{
  auto all_univs = univ_list.getAllUniverses();
  for (auto u : all_univs)
  {
    if (u.second->isRoot())
    {
      // add existing cells to current root instead of creating new universe
      auto root = getRootUniverse();
      auto all_cells = u.second->getAllCells();
      for (auto cell : all_cells)
      {
        root->addCell(cell);
      }
    }
    else
      _universe_list.addUniverse(u);
  }
}

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

  // Print out cell information
  auto all_cells = getAllCells();
  for (const auto & c_pair : all_cells)
  {
    const auto cell_ptr = c_pair.second;
    const auto cell_name = cell_ptr->getName();
    const auto cell_region = cell_ptr->getRegionAsString();
    const auto cell_filltype = cell_ptr->getFillTypeString();
    const auto fill_name = cell_ptr->getFillName();
    csg_json["CELLS"][cell_name]["FILLTYPE"] = cell_filltype;
    csg_json["CELLS"][cell_name]["REGION"] = cell_region;
    csg_json["CELLS"][cell_name]["FILL"] = fill_name;
  }

  // Print out universe information
  auto all_univs = getAllUniverses();
  for (const auto & u_pair : all_univs)
  {
    const auto univ_ptr = u_pair.second;
    const auto univ_name = univ_ptr->getName();
    const auto univ_cells = univ_ptr->getAllCells();
    csg_json["UNIVERSES"][univ_name]["CELLS"] = {};
    for (const auto & c : univ_cells)
      csg_json["UNIVERSES"][univ_name]["CELLS"].push_back(c->getName());
    if (univ_ptr->isRoot())
      csg_json["UNIVERSES"][univ_name]["ROOT"] = univ_ptr->isRoot();
  }

  return csg_json;
}
} // namespace CSG
