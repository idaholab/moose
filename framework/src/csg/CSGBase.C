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
CSGBase::createCell(const std::string name,
                    const std::string mat_name,
                    const CSGRegion & region,
                    std::shared_ptr<CSGUniverse> add_to_univ)
{
  checkRegionSurfaces(region);
  auto cell = _cell_list.addMaterialCell(name, mat_name, region);
  if (add_to_univ)
    add_to_univ->addCell(cell);
  else
    getRootUniverse()->addCell(cell);
  return cell;
}

std::shared_ptr<CSGCell>
CSGBase::createCell(const std::string name,
                    const CSGRegion & region,
                    const std::shared_ptr<CSGUniverse> add_to_univ)
{
  checkRegionSurfaces(region);
  auto cell = _cell_list.addVoidCell(name, region);
  if (add_to_univ)
    add_to_univ->addCell(cell);
  else
    getRootUniverse()->addCell(cell);
  return cell;
}

std::shared_ptr<CSGCell>
CSGBase::createCell(const std::string name,
                    const std::shared_ptr<CSGUniverse> fill_univ,
                    const CSGRegion & region,
                    const std::shared_ptr<CSGUniverse> add_to_univ)
{
  checkRegionSurfaces(region);
  if (fill_univ == add_to_univ)
    mooseError("Cell " + name +
               " cannot be filled with the same universe to which it is being added.");

  auto cell = _cell_list.addUniverseCell(name, fill_univ, region);
  if (add_to_univ)
    add_to_univ->addCell(cell);
  else
    getRootUniverse()->addCell(cell);
  return cell;
}

void CSGBase::updateCellRegion(const std::shared_ptr<CSGCell> cell, const CSGRegion & region)
{
  checkRegionSurfaces(region);
  cell->updateRegion(region);
}

void
CSGBase::joinSurfaceList(CSGSurfaceList & surf_list)
{
  // TODO: check if surface is a duplicate (by definition) and skip
  // adding if duplicate; must update references to the surface in cell
  // region definitions.
  auto all_surfs = surf_list.getAllSurfaces();
  for (auto s : all_surfs)
    _surface_list.addSurface(s.second);
}

void
CSGBase::joinCellList(CSGCellList & cell_list)
{
  auto all_cells = cell_list.getAllCells();
  for (auto c : all_cells)
    _cell_list.addCell(c.second);
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list)
{
  // case 1: incoming root is joined into existing root; no new universes are created
  auto all_univs = univ_list.getAllUniverses();
  auto root = getRootUniverse(); // this root universe
  for (auto u : all_univs)
  {
    if (u.second->isRoot())
    {
      // add existing cells to current root instead of creating new universe
      auto all_cells = u.second->getAllCells();
      for (auto cell : all_cells)
        root->addCell(cell);
    }
    else // unique non-root universe to add to list
      _universe_list.addUniverse(u.second);
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list, std::string new_root_name_incoming)
{
  // case 2: incoming root is turned into new universe and existing root remains root

  // add incoming universes to current Base
  auto all_univs = univ_list.getAllUniverses();
  for (auto u : all_univs)
  {
    if (u.second->isRoot())
    {
      // create new universe from incoming root universe
      auto all_cells = u.second->getAllCells();
      auto new_incoming_univ = createUniverse(new_root_name_incoming, all_cells);
    }
    else // unique non-root universe to add to list
      _universe_list.addUniverse(u.second);
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list,
                          std::string new_root_name_base,
                          std::string new_root_name_incoming)
{
  // case 3: each root universe becomes a new universe and a new root is created

  // make a new universe from the existing root universe
  auto root = getRootUniverse();
  auto root_cells = root->getAllCells();
  auto new_existing_univ = createUniverse(new_root_name_base, root_cells);
  root->removeAllCells();

  // add incoming universes to current Base
  auto all_univs = univ_list.getAllUniverses();
  for (auto u : all_univs)
  {
    if (u.second->isRoot())
    {
      // create new universe from incoming root universe
      auto all_cells = u.second->getAllCells();
      auto new_incoming_univ = createUniverse(new_root_name_incoming, all_cells);
    }
    else // unique non-root universe to add to list
      _universe_list.addUniverse(u.second);
  }
}

void CSGBase::checkRegionSurfaces(const CSGRegion & region)
{
  auto surfs = region.getSurfaces();
  for (auto s : surfs)
  {
    auto sname = s->getName();
    // if there is no surface by this name at all, there will be an error from getSurface
    auto list_surf = _surface_list.getSurface(s->getName());
    // if there is a surface by the same name, check that it is actually the surface being used
    // (ie same surface points to same location in memory)
    if (s != list_surf)
      mooseError("Region is being set with a surface named " + sname + " that is different from the surface of the same name in the base instance.");
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
