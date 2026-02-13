//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGBase.h"
#include "CSGUtils.h"
#include "JsonIO.h"

namespace CSG
{

CSGBase::CSGBase()
  : _surface_list(CSGSurfaceList()),
    _cell_list(CSGCellList()),
    _universe_list(CSGUniverseList()),
    _lattice_list(CSGLatticeList())
{
}

CSGBase::CSGBase(const CSGBase & other_base)
  : _surface_list(other_base.getSurfaceList()),
    _cell_list(CSGCellList()),
    _universe_list(CSGUniverseList()),
    _lattice_list(CSGLatticeList())
{
  // Iterate through all cell references from the other CSGBase instance and
  // create new CSGCell pointers based on these references. This is done
  // recursively to properly handle cells with universe fills
  for (const auto & [name, cell] : other_base.getCellList().getCellListMap())
    addCellToList(*cell);

  // Link all cells in other_base root universe to current root universe
  for (auto & root_cell : other_base.getRootUniverse().getAllCells())
  {
    const auto & list_cell = _cell_list.getCell(root_cell.get().getName());
    addCellToUniverse(getRootUniverse(), list_cell);
  }

  // Iterate through all universe references from the other CSGBase instance and
  // create new CSGUniverse pointers based on these references. This is done in case
  // any universe exist in the universe list that are not connected to the cell list.
  for (const auto & [name, univ] : other_base.getUniverseList().getUniverseListMap())
    addUniverseToList(*univ);

  // Iterate through all lattice references from the other CSGBase instance and
  // create new CSGLattice pointers based on these references.
  for (const auto & [name, lattice] : other_base.getLatticeList().getLatticeListMap())
    addLatticeToList(*lattice);
}

CSGBase::~CSGBase() {}

const CSGCell &
CSGBase::addCellToList(const CSGCell & cell)
{
  // If cell has already been created, we just return a reference to it
  const auto name = cell.getName();
  if (_cell_list.hasCell(name))
    return _cell_list.getCell(name);

  // Otherwise if the cell has material or void cell, we can create it directly
  const auto fill_type = cell.getFillType();
  const auto region = cell.getRegion();
  if (fill_type == "VOID")
    return _cell_list.addVoidCell(name, region);
  else if (fill_type == "CSG_MATERIAL")
  {
    const auto mat_name = cell.getFillMaterial();
    return _cell_list.addMaterialCell(name, mat_name, region);
  }
  else if (fill_type == "LATTICE")
  {
    // add lattice recursively to capture all linked universes in the lattice
    const CSGLattice & lattice = addLatticeToList(cell.getFillLattice());
    return _cell_list.addLatticeCell(name, lattice, region);
  }
  // Otherwise if the cell has a universe fill, we need to recursively define
  // all linked universes and cells first before defining this cell
  else if (fill_type == "UNIVERSE")
  {
    const auto & univ = addUniverseToList(cell.getFillUniverse());
    return _cell_list.addUniverseCell(name, univ, region);
  }
  else
    mooseError("Cell " + name + " has unrecognized fill type " + fill_type);
}

const CSGUniverse &
CSGBase::addUniverseToList(const CSGUniverse & univ)
{
  // If universe has already been created, we just return a reference to it
  const auto name = univ.getName();
  if (_universe_list.hasUniverse(name))
    return _universe_list.getUniverse(name);

  // Otherwise we create a new universe based on its associated cells.
  // addCellToList is called recursively in case associated cells have not
  // been added to the cell list yet.
  const auto univ_cells = univ.getAllCells();
  std::vector<std::reference_wrapper<const CSGCell>> current_univ_cells;
  for (const auto & univ_cell : univ_cells)
    current_univ_cells.push_back(addCellToList(univ_cell));
  return createUniverse(name, current_univ_cells);
}

const CSGLattice &
CSGBase::addLatticeToList(const CSGLattice & lattice)
{
  // If lattice has already been created, we just return a reference to it
  const auto name = lattice.getName();
  if (_lattice_list.hasLattice(name))
    return _lattice_list.getLattice(name);

  // Clone the lattice (associated universes need to be transferred and set)
  auto cloned_lattice = lattice.clone();

  // If lattice has associated universes, we need to add them to this CSGBase instance as well.
  // addUniverseToList is called recursively in case associated universes have not been added to
  // the universe list yet.
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> current_univ_map;
  for (const auto & univ_list : lattice.getUniverses())
  {
    std::vector<std::reference_wrapper<const CSGUniverse>> current_univ_list;
    for (const auto & univ_ref : univ_list)
      current_univ_list.push_back(addUniverseToList(univ_ref.get()));
    current_univ_map.push_back(current_univ_list);
  }

  // Set universes only if lattice has universes defined
  if (current_univ_map.size() > 0)
    cloned_lattice->setUniverses(current_univ_map);

  // Update reference to outer universe if it exists
  if (lattice.getOuterType() == "UNIVERSE")
  {
    const auto & outer_univ_ref = addUniverseToList(lattice.getOuterUniverse());
    cloned_lattice->updateOuter(outer_univ_ref);
  }

  // Use addLattice to add the cloned lattice
  return addLattice(std::move(cloned_lattice));
}

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
  if (add_to_univ && (&fill_univ == add_to_univ))
    mooseError("Cell " + name +
               " cannot be filled with the same universe to which it is being added.");

  auto & cell = _cell_list.addUniverseCell(name, fill_univ, region);
  if (add_to_univ)
    addCellToUniverse(*add_to_univ, cell);
  else
    addCellToUniverse(getRootUniverse(), cell);
  return cell;
}

const CSGCell &
CSGBase::createCell(const std::string & name,
                    const CSGLattice & fill_lattice,
                    const CSGRegion & region,
                    const CSGUniverse * add_to_univ)
{
  checkRegionSurfaces(region);

  // check that cell is not being added to a universe that exists in the lattice itself
  if (add_to_univ)
    for (auto univ_list : fill_lattice.getUniverses())
      for (const auto & univ_ref : univ_list)
      {
        const CSGUniverse & univ_in_lattice = univ_ref.get();
        if (&univ_in_lattice == add_to_univ)
          mooseError("Cell " + name +
                     " cannot be filled with a lattice containing the same universe to which it is "
                     "being added.");
      }

  auto & cell = _cell_list.addLatticeCell(name, fill_lattice, region);
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
  // make sure universe is a part of this CSGBase instance
  if (!checkUniverseInBase(universe))
    mooseError("Cells are being added to a universe named " + universe.getName() +
               " that is different " +
               "from the universe of the same name in the CSGBase instance.");
  auto & univ = _universe_list.getUniverse(universe.getName());
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
  // make sure universe is a part of this CSGBase instance
  if (!checkUniverseInBase(universe))
    mooseError("Cells are being removed from a universe named " + universe.getName() +
               " that is different " +
               "from the universe of the same name in the CSGBase instance.");
  auto & univ = _universe_list.getUniverse(universe.getName());
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
CSGBase::setLatticeOuter(const CSGLattice & lattice, const std::string & outer_name)
{
  auto name = lattice.getName();
  if (!checkLatticeInBase(lattice))
    mooseError("Cannot set outer for lattice " + name +
               ". Lattice is different from the lattice of the same name in the "
               "CSGBase instance.");
  _lattice_list.getLattice(name).updateOuter(outer_name);
}

void
CSGBase::setLatticeOuter(const CSGLattice & lattice, const CSGUniverse & outer_univ)
{
  auto name = lattice.getName();
  if (!checkLatticeInBase(lattice))
    mooseError("Cannot set outer universe for lattice " + name +
               ". Lattice is different from the lattice of the same name in the "
               "CSGBase instance.");
  if (!checkUniverseInBase(outer_univ))
    mooseError("Cannot set outer universe for lattice " + name + ". Outer universe " +
               outer_univ.getName() + " is not in the CSGBase instance.");
  _lattice_list.getLattice(name).updateOuter(outer_univ);
}

void
CSGBase::resetLatticeOuter(const CSGLattice & lattice)
{
  auto name = lattice.getName();
  if (!checkLatticeInBase(lattice))
    mooseError("Cannot reset outer for lattice " + name +
               ". Lattice is different from the lattice of the same name in the "
               "CSGBase instance.");
  _lattice_list.getLattice(name).resetOuter();
}

void
CSGBase::setUniverseAtLatticeIndex(const CSGLattice & lattice,
                                   const CSGUniverse & universe,
                                   std::pair<int, int> index)
{
  auto name = lattice.getName();
  if (!checkLatticeInBase(lattice))
    mooseError("Cannot set universe at index for lattice " + name +
               ". Lattice is different from the lattice of the same name in the "
               "CSGBase instance.");
  if (!checkUniverseInBase(universe))
    mooseError("Cannot add universe " + universe.getName() + " to lattice " + lattice.getName() +
               ". Universe is not in the CSGBase instance.");
  _lattice_list.getLattice(name).setUniverseAtIndex(universe, index);
}

void
CSGBase::setLatticeUniverses(
    const CSGLattice & lattice,
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> & universes)
{
  auto name = lattice.getName();
  if (!checkLatticeInBase(lattice))
    mooseError("Cannot set universes for lattice " + name +
               ". Lattice is different from the lattice of the same name in the "
               "CSGBase instance.");
  // make sure all universes are a part of this base instance
  for (auto univ_list : universes)
    for (const CSGUniverse & univ : univ_list)
      if (!checkUniverseInBase(univ))
        mooseError("Cannot set universes for lattice " + name + ". Universe " + univ.getName() +
                   " is not in the CSGBase instance.");
  _lattice_list.getLattice(name).setUniverses(universes);
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> base)
{
  joinSurfaceList(base->getSurfaceList());
  joinCellList(base->getCellList());
  joinLatticeList(base->getLatticeList());
  joinUniverseList(base->getUniverseList());
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> base, std::string & new_root_name_join)
{
  joinSurfaceList(base->getSurfaceList());
  joinCellList(base->getCellList());
  joinLatticeList(base->getLatticeList());
  joinUniverseList(base->getUniverseList(), new_root_name_join);
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> base,
                       const std::string & new_root_name_base,
                       const std::string & new_root_name_join)
{
  joinSurfaceList(base->getSurfaceList());
  joinCellList(base->getCellList());
  joinLatticeList(base->getLatticeList());
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
    _surface_list.addSurface(std::move(s.second));
}

void
CSGBase::joinCellList(CSGCellList & cell_list)
{
  auto & cell_list_map = cell_list.getCellListMap();
  for (auto & c : cell_list_map)
    _cell_list.addCell(std::move(c.second));
}

void
CSGBase::joinLatticeList(CSGLatticeList & lattice_list)
{
  auto & lat_list_map = lattice_list.getLatticeListMap();
  for (auto & lat : lat_list_map)
    _lattice_list.addLattice(std::move(lat.second));
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
      _universe_list.addUniverse(std::move(u.second));
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list, const std::string & new_root_name_incoming)
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
      _universe_list.addUniverse(std::move(u.second));
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list,
                          const std::string & new_root_name_base,
                          const std::string & new_root_name_incoming)
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
      _universe_list.addUniverse(std::move(u.second));
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
    if (&s != &list_surf)
      mooseError("Region is being set with a surface named " + sname +
                 " that is different from the surface of the same name in the CSGBase instance.");
  }
}

bool
CSGBase::checkCellInBase(const CSGCell & cell) const
{
  auto name = cell.getName();
  // if no cell by this name exists, an error will be produced by getCell
  auto & list_cell = _cell_list.getCell(name);
  // return whether that the cell in the list is the same as the cell provided (in memory)
  return &cell == &list_cell;
}

bool
CSGBase::checkUniverseInBase(const CSGUniverse & universe) const
{
  auto name = universe.getName();
  // if no universe by this name exists, an error will be produced by getUniverse
  auto & list_univ = _universe_list.getUniverse(name);
  // return whether that the universe in the list is the same as the universe provided (in memory)
  return &universe == &list_univ;
}

bool
CSGBase::checkLatticeInBase(const CSGLattice & lattice) const
{
  auto name = lattice.getName();
  // if no lattice by this name exists, an error will be produced by getLattice
  auto & list_lattice = _lattice_list.getLattice(name);
  // return whether that the lattice in the list is the same as the lattice provided (in memory)
  return &lattice == &list_lattice;
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
    else if (cell.getFillType() == "LATTICE")
    {
      const auto & lattice = cell.getFillLattice();
      for (const auto & univ_list : lattice.getUniverses())
        for (const auto & univ_ref : univ_list)
        {
          const CSGUniverse & lattice_univ = univ_ref.get();
          getLinkedUniverses(lattice_univ, linked_universe_names);
        }

      if (lattice.getOuterType() == "UNIVERSE")
      {
        const CSGUniverse & outer_univ = lattice.getOuterUniverse();
        getLinkedUniverses(outer_univ, linked_universe_names);
      }
    }
}

nlohmann::json
CSGBase::generateOutput() const
{
  // Check that orphaned universes do not exist in universe list of CSGBase object
  checkUniverseLinking();

  nlohmann::json csg_json;

  csg_json["surfaces"] = {};
  csg_json["cells"] = {};
  csg_json["universes"] = {};

  // get all surfaces information
  auto all_surfs = getAllSurfaces();
  for (const CSGSurface & s : all_surfs)
  {
    const auto & surf_name = s.getName();
    const auto & coeffs = s.getCoeffs();
    csg_json["surfaces"][surf_name] = {{"type", s.getSurfaceType()}, {"coefficients", {}}};
    for (const auto & c : coeffs)
      csg_json["surfaces"][surf_name]["coefficients"][c.first] = c.second;
  }

  // Print out cell information
  auto all_cells = getAllCells();
  for (const CSGCell & c : all_cells)
  {
    const auto & cell_name = c.getName();
    const auto & cell_region_infix = c.getRegion().toInfixJSON();
    const auto & cell_region_postfix = c.getRegion().toPostfixStringList();
    const auto & cell_filltype = c.getFillType();
    const auto & fill_name = c.getFillName();
    csg_json["cells"][cell_name]["filltype"] = cell_filltype;
    csg_json["cells"][cell_name]["region_infix"] = cell_region_infix;
    csg_json["cells"][cell_name]["region_postfix"] = cell_region_postfix;
    csg_json["cells"][cell_name]["fill"] = fill_name;
  }

  // Print out universe information
  auto all_univs = getAllUniverses();
  for (const CSGUniverse & u : all_univs)
  {
    const auto & univ_name = u.getName();
    const auto & univ_cells = u.getAllCells();
    csg_json["universes"][univ_name]["cells"] = {};
    for (const CSGCell & c : univ_cells)
      csg_json["universes"][univ_name]["cells"].push_back(c.getName());
    if (u.isRoot())
      csg_json["universes"][univ_name]["root"] = u.isRoot();
  }

  // print out lattice information if lattices exist
  auto all_lats = getAllLattices();
  if (all_lats.size() > 0)
  {
    csg_json["lattices"] = {};
    for (const CSGLattice & lat : all_lats)
    {
      const auto & lat_name = lat.getName();
      csg_json["lattices"][lat_name] = {};
      csg_json["lattices"][lat_name]["type"] = lat.getType();
      const auto & outer_type = lat.getOuterType();
      csg_json["lattices"][lat_name]["outertype"] = outer_type;
      if (outer_type == "UNIVERSE")
        csg_json["lattices"][lat_name]["outer"] = lat.getOuterUniverse().getName();
      else if (outer_type == "CSG_MATERIAL")
        csg_json["lattices"][lat_name]["outer"] = lat.getOuterMaterial();
      // write out any additional attributes
      csg_json["lattices"][lat_name]["attributes"] = {};
      const auto & lat_attrs = lat.getAttributes();
      for (const auto & attr : lat_attrs)
        csg_json["lattices"][lat_name]["attributes"][attr.first] = attr.second;
      // write the map of universe names: list of lists
      csg_json["lattices"][lat_name]["universes"] = lat.getUniverseNameMap();
    }
  }

  return csg_json;
}

bool
CSGBase::operator==(const CSGBase & other) const
{
  const auto & surf_list = this->getSurfaceList();
  const auto & other_surf_list = other.getSurfaceList();
  const auto & cell_list = this->getCellList();
  const auto & other_cell_list = other.getCellList();
  const auto & univ_list = this->getUniverseList();
  const auto & other_univ_list = other.getUniverseList();
  const auto & lat_list = this->getLatticeList();
  const auto & other_lat_list = other.getLatticeList();
  return (surf_list == other_surf_list) && (cell_list == other_cell_list) &&
         (univ_list == other_univ_list) && (lat_list == other_lat_list);
}

bool
CSGBase::operator!=(const CSGBase & other) const
{
  return !(*this == other);
}

} // namespace CSG
