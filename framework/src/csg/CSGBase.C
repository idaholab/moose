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

const CSGCell &
CSGBase::createCell(const std::string & name,
                    const std::string & mat_name,
                    const CSGRegion & region,
                    const CSGUniverse * add_to_univ)
{
  checkRegionSurfaces(region);
  auto & cell = _cell_list.addMaterialCell(name, mat_name, region);
  if (add_to_univ)
    addCellToUniverse(*add_to_univ, cell);
  else
    addCellToUniverse(getRootUniverse(), cell);
  return cell;
}

const CSGCell &
CSGBase::createCell(const std::string & name,
                    const CSGRegion & region,
                    const CSGUniverse * add_to_univ)
{
  checkRegionSurfaces(region);
  auto & cell = _cell_list.addVoidCell(name, region);
  if (add_to_univ)
    addCellToUniverse(*add_to_univ, cell);
  else
    addCellToUniverse(getRootUniverse(), cell);
  return cell;
}

const CSGCell &
CSGBase::createCell(const std::string & name,
                    const CSGUniverse & fill_univ,
                    const CSGRegion & region,
                    const CSGUniverse * add_to_univ)
{
  checkRegionSurfaces(region);
  if (add_to_univ && (fill_univ == *add_to_univ))
    mooseError("Cell " + name +
               " cannot be filled with the same universe to which it is being added.");

  auto & cell = _cell_list.addUniverseCell(name, fill_univ, region);
  if (add_to_univ)
    addCellToUniverse(*add_to_univ, cell);
  else
    addCellToUniverse(getRootUniverse(), cell);
  return cell;
}

void
CSGBase::updateCellRegion(const CSGCell & cell, const CSGRegion & region)
{
  checkRegionSurfaces(region);
  if (!checkCellInBase(cell))
    mooseError("The region of cell with name " + cell.getName() +
               " is being updated that is different " +
               "from the cell of the same name in the CSGBase instance.");
  auto & list_cell = _cell_list.getCell(cell.getName());
  list_cell.updateRegion(region);
}

const CSGUniverse &
CSGBase::createUniverse(const std::string & name,
                        std::vector<std::reference_wrapper<const CSGCell>> & cells)
{
  auto & univ = _universe_list.addUniverse(name);
  addCellsToUniverse(univ, cells); // performs a check that cells are a part of this base
  return univ;
}

void
CSGBase::addCellToUniverse(const CSGUniverse & universe, const CSGCell & cell)
{
  // make sure cell is a part of this CSGBase instance
  if (!checkCellInBase(cell))
    mooseError("A cell named " + cell.getName() + " is being added to universe " +
               universe.getName() +
               " that is different from the cell of the same name in the CSGBase instance.");
  auto & univ = _universe_list.getUniverse(universe.getName());
  if (univ != universe)
    mooseError("Cells are being added to a universe named " + universe.getName() +
               " that is different " +
               "from the universe of the same name in the CSGBase instance.");
  univ.addCell(cell);
}

void
CSGBase::addCellsToUniverse(const CSGUniverse & universe,
                            std::vector<std::reference_wrapper<const CSGCell>> & cells)
{
  for (auto & c : cells)
    addCellToUniverse(universe, c);
}

void
CSGBase::removeCellFromUniverse(const CSGUniverse & universe, const CSGCell & cell)
{
  // make sure cell is a part of this CSGBase instance
  if (!checkCellInBase(cell))
    mooseError("A cell named " + cell.getName() + " is being removed from universe " +
               universe.getName() +
               " that is different from the cell of the same name in the CSGBase instance.");
  auto & univ = _universe_list.getUniverse(universe.getName());
  if (univ != universe)
    mooseError("Cells are being removed from a universe named " + universe.getName() +
               " that is different " +
               "from the universe of the same name in the CSGBase instance.");
  univ.removeCell(cell.getName());
}

void
CSGBase::removeCellsFromUniverse(const CSGUniverse & universe,
                                 std::vector<std::reference_wrapper<const CSGCell>> & cells)
{
  for (auto & c : cells)
    removeCellFromUniverse(universe, c);
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> & base)
{
  joinSurfaceList(base->getSurfaceList());
  joinCellList(base->getCellList());
  joinUniverseList(base->getUniverseList());
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> & base, std::string & new_root_name_join)
{
  joinSurfaceList(base->getSurfaceList());
  joinCellList(base->getCellList());
  joinUniverseList(base->getUniverseList(), new_root_name_join);
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> & base,
                       std::string & new_root_name_base,
                       std::string & new_root_name_join)
{
  joinSurfaceList(base->getSurfaceList());
  joinCellList(base->getCellList());
  joinUniverseList(base->getUniverseList(), new_root_name_base, new_root_name_join);
}

void
CSGBase::joinSurfaceList(CSGSurfaceList & surf_list)
{
  // TODO: check if surface is a duplicate (by definition) and skip
  // adding if duplicate; must update references to the surface in cell
  // region definitions.
  auto & surf_list_map = surf_list.getSurfaceListMap();
  for (auto & s : surf_list_map)
    _surface_list.addSurface(s.second);
}

void
CSGBase::joinCellList(CSGCellList & cell_list)
{
  auto & cell_list_map = cell_list.getCellListMap();
  for (auto & c : cell_list_map)
    _cell_list.addCell(c.second);
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list)
{
  // case 1: incoming root is joined into existing root; no new universes are created
  auto & univ_list_map = univ_list.getUniverseListMap();
  auto & root = getRootUniverse(); // this root universe
  for (auto & u : univ_list_map)
  {
    if (u.second->isRoot())
    {
      // add existing cells to current root instead of creating new universe
      auto all_cells = u.second->getAllCells();
      for (auto & cell : all_cells)
        addCellToUniverse(root, cell);
    }
    else // unique non-root universe to add to list
      _universe_list.addUniverse(u.second);
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list, std::string & new_root_name_incoming)
{
  // case 2: incoming root is turned into new universe and existing root remains root

  // add incoming universes to current Base
  auto & all_univs = univ_list.getUniverseListMap();
  for (auto & u : all_univs)
  {
    if (u.second->isRoot())
    {
      // create new universe from incoming root universe
      auto all_cells = u.second->getAllCells();
      createUniverse(new_root_name_incoming, all_cells);
    }
    else // unique non-root universe to add to list
      _universe_list.addUniverse(u.second);
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list,
                          std::string & new_root_name_base,
                          std::string & new_root_name_incoming)
{
  // case 3: each root universe becomes a new universe and a new root is created

  // make a new universe from the existing root universe
  auto & root = getRootUniverse();
  auto root_cells = root.getAllCells();
  createUniverse(new_root_name_base, root_cells);
  removeCellsFromUniverse(root, root_cells);

  // add incoming universes to current Base
  auto & all_univs = univ_list.getUniverseListMap();
  for (auto & u : all_univs)
  {
    if (u.second->isRoot())
    {
      // create new universe from incoming root universe
      auto all_cells = u.second->getAllCells();
      createUniverse(new_root_name_incoming, all_cells);
    }
    else // unique non-root universe to add to list
      _universe_list.addUniverse(u.second);
  }
}

void
CSGBase::checkRegionSurfaces(const CSGRegion & region) const
{
  auto & surfs = region.getSurfaces();
  for (const CSGSurface & s : surfs)
  {
    auto sname = s.getName();
    // if there is no surface by this name at all, there will be an error from getSurface
    const auto & list_surf = _surface_list.getSurface(s.getName());
    // if there is a surface by the same name, check that it is actually the surface being used
    // (ie same surface points to same location in memory)
    if (s != list_surf)
      mooseError("Region is being set with a surface named " + sname +
                 " that is different from the surface of the same name in the CSGBase instance.");
  }
}

bool
CSGBase::checkCellInBase(const CSGCell & cell) const
{
  auto name = cell.getName();
  // if no cell by this name exists, an error will be produced by getCell
  auto list_cell = _cell_list.getCell(name);
  // return whether that the cell in the list is the same as the cell provided
  return cell == list_cell;
}

void
CSGBase::checkUniverseLinking() const
{
  std::vector<std::string> linked_universe_names;

  // Recursively figure out which universe names are linked to root universe
  getLinkedUniverses(getRootUniverse(), linked_universe_names);

  // Iterate through all universes in universe list and check that they exist in universes linked
  // to root universe list
  for (const CSGUniverse & univ : getAllUniverses())
    if (std::find(linked_universe_names.begin(), linked_universe_names.end(), univ.getName()) ==
        linked_universe_names.end())
      mooseWarning("Universe with name ", univ.getName(), " is not linked to root universe.");
}

void
CSGBase::getLinkedUniverses(const CSGUniverse & univ,
                            std::vector<std::string> & linked_universe_names) const
{
  linked_universe_names.push_back(univ.getName());
  const auto & univ_cells = univ.getAllCells();
  for (const CSGCell & cell : univ_cells)
    if (cell.getFillType() == "UNIVERSE")
      getLinkedUniverses(cell.getFillUniverse(), linked_universe_names);
}

nlohmann::json
CSGBase::generateOutput() const
{
  // Check that orphaned universes do not exist in universe list of CSGBase object
  checkUniverseLinking();

  nlohmann::json csg_json;

  csg_json["SURFACES"] = {};
  csg_json["CELLS"] = {};
  csg_json["UNIVERSES"] = {};

  // get all surfaces information
  auto all_surfs = getAllSurfaces();
  for (const CSGSurface & s : all_surfs)
  {
    const auto & surf_name = s.getName();
    const auto & coeffs = s.getCoeffs();
    csg_json["SURFACES"][surf_name] = {{"TYPE", s.getSurfaceType()}, {"COEFFICIENTS", {}}};
    for (const auto & c : coeffs)
      csg_json["SURFACES"][surf_name]["COEFFICIENTS"][c.first] = c.second;
  }

  // Print out cell information
  auto all_cells = getAllCells();
  for (const CSGCell & c : all_cells)
  {
    const auto & cell_name = c.getName();
    const auto & cell_region = c.getRegionAsString();
    const auto & cell_filltype = c.getFillType();
    const auto & fill_name = c.getFillName();
    csg_json["CELLS"][cell_name]["FILLTYPE"] = cell_filltype;
    csg_json["CELLS"][cell_name]["REGION"] = cell_region;
    csg_json["CELLS"][cell_name]["FILL"] = fill_name;
  }

  // Print out universe information
  auto all_univs = getAllUniverses();
  for (const CSGUniverse & u : all_univs)
  {
    const auto & univ_name = u.getName();
    const auto & univ_cells = u.getAllCells();
    csg_json["UNIVERSES"][univ_name]["CELLS"] = {};
    for (const CSGCell & c : univ_cells)
      csg_json["UNIVERSES"][univ_name]["CELLS"].push_back(c.getName());
    if (u.isRoot())
      csg_json["UNIVERSES"][univ_name]["ROOT"] = u.isRoot();
  }

  return csg_json;
}

} // namespace CSG
