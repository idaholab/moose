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
  // _surface_list was copy-constructed above, cloning all surfaces (including surface EngUnits)
  // via CSGSurface::clone(). Any CSGSurfaceEngUnits need to also be added to the engineering units
  // list for tracking.
  for (auto & [name, surf_ptr] : _surface_list.getSurfaceListMap())
    if (auto * eng_unit = dynamic_cast<CSGSurfaceEngUnit *>(surf_ptr.get()))
      _eng_unit_list.addEngUnit(*eng_unit);

  // Iterate through all cell references from the other CSGBase instance and create new CSGCell
  // pointers based on these references. This is done recursively to properly handle cells with
  // universe fills. Engineering units are added first so that they can recurse properly and not
  // cause errors in an attempt to add them as plain objects.
  for (const auto & [name, cell] : other_base.getCellList().getCellListMap())
    if (const auto * eng_unit = isCellEngUnit(*cell))
      addEngUnit(eng_unit->clone());
  for (const auto & [name, cell] : other_base.getCellList().getCellListMap())
    if (!isCellEngUnit(*cell))
      addCellToList(*cell);

  // Link all cells in other_base root universe to current root universe
  for (auto & root_cell : other_base.getRootUniverse().getAllCells())
  {
    const auto & list_cell = _cell_list.getCell(root_cell.get().getName());
    addCellToUniverse(getRootUniverse(), list_cell);
  }

  // Iterate through all universe references from the other CSGBase instance and create new
  // CSGUniverse pointers based on these references. This is done in case any universe exist in the
  // universe list that are not connected to the cell list. Engineering units are added first so
  // that they can recurse properly and not cause errors in an attempt to add them as plain objects.
  for (const auto & [name, univ] : other_base.getUniverseList().getUniverseListMap())
    if (const auto * eng_unit = isUniverseEngUnit(*univ))
      addEngUnit(eng_unit->clone());
  for (const auto & [name, univ] : other_base.getUniverseList().getUniverseListMap())
    if (!isUniverseEngUnit(*univ))
      addUniverseToList(*univ);

  // Iterate through all lattice references from the other CSGBase instance and
  // create new CSGLattice pointers based on these references.
  for (const auto & [name, lattice] : other_base.getLatticeList().getLatticeListMap())
    addLatticeToList(*lattice);
}

CSGBase::~CSGBase() {}

const CSGSurface &
CSGBase::addSurface(std::unique_ptr<CSGSurface> surf)
{
  if (isSurfaceEngUnit(*surf))
    mooseError("Surface '",
               surf->getName(),
               "' is a CSGSurfaceEngUnit and must be added via addEngUnit(), not addSurface().");
  return _surface_list.addSurface(std::move(surf));
}

void
CSGBase::prepareSurfaceDeletion(const CSGSurface & surface) const
{
  for (const auto & cell_ref : _cell_list.getAllCells())
  {
    const auto & cell = cell_ref.get();
    for (const auto & region_surf : cell.getRegion().getSurfaces())
      if (region_surf.get() == surface)
        mooseError("Cannot delete surface with name ",
                   surface.getName(),
                   " as it is used in region definition of cell with name ",
                   cell.getName());
  }
}

void
CSGBase::deleteSurface(const CSGSurface & surface)
{
  if (!checkSurfaceInBase(surface))
    mooseError("Surface with name ",
               surface.getName(),
               " cannot be deleted as it is different from the surface of the same name in the "
               "CSGBase instance.");

  prepareSurfaceDeletion(surface);
  if (const auto * eng_unit = isSurfaceEngUnit(surface))
    _eng_unit_list.removeEngUnit(*eng_unit);
  _surface_list.getSurfaceListMap().erase(surface.getName());
}

const CSGCell &
CSGBase::addCellToList(const CSGCell & cell)
{
  // If cell has already been created, we just return a reference to it
  const auto name = cell.getName();
  if (_cell_list.hasCell(name))
    return _cell_list.getCell(name);

  // Engineering unit cells must be registered via addEngUnit(), not addCellToList()
  if (isCellEngUnit(cell))
    mooseError("Cell '",
               name,
               "' is a CSGCellEngUnit and must be added via addEngUnit(), not addCellToList().");

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

  // Engineering unit universes must be registered via addEngUnit(), not addUniverseToList()
  if (isUniverseEngUnit(univ))
    mooseError("Universe '",
               name,
               "' is a CSGUniverseEngUnit and must be added via addEngUnit(), not "
               "addUniverseToList().");

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

void
CSGBase::deleteLattice(const CSGLattice & lattice)
{
  if (!checkLatticeInBase(lattice))
    mooseError("Lattice with name ",
               lattice.getName(),
               " cannot be deleted as it is different from the lattice of the same name in the "
               "CSGBase instance.");

  // Check if lattice is used as fill in existing cells
  for (const auto & cell_ref : _cell_list.getAllCells())
  {
    const auto & cell = cell_ref.get();
    if ((cell.getFillType() == "LATTICE") && (cell.getFillLattice() == lattice))
      mooseError("Cannot delete lattice with name ",
                 lattice.getName(),
                 " as it is used as the fill of cell with name ",
                 cell.getName());
  }

  _lattice_list.getLatticeListMap().erase(lattice.getName());
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
CSGBase::prepareCellDeletion(const CSGCell & cell)
{
  for (const auto & univ_ref : _universe_list.getAllUniverses())
  {
    const auto & univ = univ_ref.get();
    for (const auto & univ_cell : univ.getAllCells())
      if (cell == univ_cell.get() && univ != getRootUniverse())
      {
        mooseWarning("Removing cell ",
                     cell.getName(),
                     " from universe with name ",
                     univ.getName(),
                     " before cell deletion.");
        _universe_list.getUniverse(univ.getName()).removeCell(cell.getName());
      }
  }
}

void
CSGBase::deleteCell(const CSGCell & cell)
{
  if (!checkCellInBase(cell))
    mooseError("Cell with name ",
               cell.getName(),
               " cannot be deleted as it is different from the cell of the same name in the "
               "CSGBase instance.");

  prepareCellDeletion(cell);
  if (const auto * eng_unit = isCellEngUnit(cell))
    _eng_unit_list.removeEngUnit(*eng_unit);
  _cell_list.getCellListMap().erase(cell.getName());
}

void
CSGBase::updateCellRegion(const CSGCell & cell, const CSGRegion & region)
{
  // cannot update region for a cell that is actually an engineering unit
  if (isCellEngUnit(cell))
    mooseError("Region cannot be updated for cell '" + cell.getName() +
               "' because it is a CSGCellEngUnit.");

  checkRegionSurfaces(region);
  if (!checkCellInBase(cell))
    mooseError("The region of cell with name " + cell.getName() +
               " that is being updated is different " +
               "from the cell of the same name in the CSGBase instance.");
  auto & list_cell = _cell_list.getCell(cell.getName());
  list_cell.updateRegion(region);
}

void
CSGBase::resetCellFill(const CSGCell & cell)
{
  // cannot update region for a cell that is actually an engineering unit
  if (isCellEngUnit(cell))
    mooseError("Fill cannot be reset for cell '" + cell.getName() +
               "' because it is a CSGCellEngUnit.");

  if (!checkCellInBase(cell))
    mooseError("The fill of cell with name " + cell.getName() +
               " that is being updated is different " +
               "from the cell of the same name in the CSGBase instance.");
  auto & list_cell = _cell_list.getCell(cell.getName());
  list_cell.resetCellFill();
}

void
CSGBase::updateCellFill(const CSGCell & cell, const std::string & mat_name)
{
  // cannot update region for a cell that is actually an engineering unit
  if (isCellEngUnit(cell))
    mooseError("Fill cannot be updated for cell '" + cell.getName() +
               "' because it is a CSGCellEngUnit.");

  if (!checkCellInBase(cell))
    mooseError("The region of cell with name " + cell.getName() +
               " that is being updated is different " +
               "from the cell of the same name in the CSGBase instance.");
  auto & list_cell = _cell_list.getCell(cell.getName());
  list_cell.updateCellFill(mat_name);
}

void
CSGBase::updateCellFill(const CSGCell & cell, const CSGUniverse * univ)
{
  // cannot update region for a cell that is actually an engineering unit
  if (isCellEngUnit(cell))
    mooseError("Fill cannot be updated for cell '" + cell.getName() +
               "' because it is a CSGCellEngUnit.");

  if (!checkUniverseInBase(*univ))
    mooseError("Universe with name ",
               univ->getName(),
               " is being used as a cell fill that is different from the universe of the same name "
               "in the CSGBase instance.");
  if (!checkCellInBase(cell))
    mooseError("The fill of cell with name " + cell.getName() +
               " that is being updated is different " +
               "from the cell of the same name in the CSGBase instance.");
  auto & list_cell = _cell_list.getCell(cell.getName());
  list_cell.updateCellFill(univ);
}

void
CSGBase::updateCellFill(const CSGCell & cell, const CSGLattice * lattice)
{
  // cannot update region for a cell that is actually an engineering unit
  if (isCellEngUnit(cell))
    mooseError("Fill cannot be updated for cell '" + cell.getName() +
               "' because it is a CSGCellEngUnit.");

  if (!checkLatticeInBase(*lattice))
    mooseError("Lattice with name ",
               lattice->getName(),
               " is being used as a cell fill that is different from the lattice of the same name "
               "in the CSGBase instance.");
  if (!checkCellInBase(cell))
    mooseError("The fill of cell with name " + cell.getName() +
               " that is being updated is different " +
               "from the cell of the same name in the CSGBase instance.");
  auto & list_cell = _cell_list.getCell(cell.getName());
  list_cell.updateCellFill(lattice);
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
CSGBase::prepareUniverseDeletion(const CSGUniverse & univ) const
{
  if (univ == getRootUniverse())
    mooseError("Cannot delete root universe from CSGBase instance");

  // Check if universe is used in any existing lattices
  for (const auto & lat : _lattice_list.getAllLattices())
  {
    for (const auto & lat_univ : lat.get().getUniqueUniverses())
      if (univ == lat_univ.get())
        mooseError("Cannot delete universe with name ",
                   univ.getName(),
                   " as it is used in lattice with name ",
                   lat.get().getName());
    if ((lat.get().getOuterType() == "UNIVERSE") && (lat.get().getOuterUniverse() == univ))
      mooseError("Cannot delete universe with name ",
                 univ.getName(),
                 " as it is used as the outer universe of lattice with name ",
                 lat.get().getName());
  }

  // Check if universe is used as fill in existing cells
  for (const auto & cell_ref : _cell_list.getAllCells())
  {
    const auto & cell = cell_ref.get();
    if ((cell.getFillType() == "UNIVERSE") && (cell.getFillUniverse() == univ))
      mooseError("Cannot delete universe with name ",
                 univ.getName(),
                 " as it is used as the fill of cell with name ",
                 cell.getName());
  }
}

void
CSGBase::deleteUniverse(const CSGUniverse & univ)
{
  if (!checkUniverseInBase(univ))
    mooseError("Universe with name ",
               univ.getName(),
               " cannot be deleted as it is different from the universe of the same name in the "
               "CSGBase instance.");

  prepareUniverseDeletion(univ);
  if (const auto * eng_unit = isUniverseEngUnit(univ))
    _eng_unit_list.removeEngUnit(*eng_unit);
  _universe_list.getUniverseListMap().erase(univ.getName());
}

void
CSGBase::addCellToUniverse(const CSGUniverse & universe, const CSGCell & cell)
{
  // if universe is actually engineering unit, cannot add cells
  if (isUniverseEngUnit(universe))
    mooseError("Universe '" + universe.getName() +
               "' cannot add cells because it is a CSGUniverseEngUnit.");

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
  univ.removeCell(
      cell.getName()); // this will produce error that cell is not found in the case that the
                       // universe is actually an engineering unit, so we don't need to check that.
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
CSGBase::addTransformation(const CSGObjectVariant & csg_object,
                           TransformationType type,
                           const std::tuple<Real, Real, Real> & values)
{
  // Use std::visit to handle each type in the variant
  std::visit(
      [&](const auto & obj)
      {
        using T = std::decay_t<decltype(obj.get())>;

        // Handle each CSG object type differently because each needs to check that it exists in
        // this base instance
        if constexpr (std::is_same_v<T, CSGCell>)
        {
          const CSGCell & cell = obj.get();
          if (!checkCellInBase(cell))
            mooseError("Cannot apply transformation to cell ",
                       cell.getName(),
                       " that is not in this CSGBase instance.");

          // Get non-const reference and apply transformation
          CSGCell & mutable_cell = _cell_list.getCell(cell.getName());
          mooseAssert(mutable_cell == cell, "Mutable cell does not match const cell passed in.");
          mutable_cell.addTransformation(type, values);
        }
        else if constexpr (std::is_same_v<T, CSGSurface>)
        {
          const CSGSurface & surface = obj.get();
          if (!checkSurfaceInBase(surface))
            mooseError("Cannot apply transformation to surface ",
                       surface.getName(),
                       " that is not in this CSGBase instance.");

          // Get non-const reference and apply transformation
          CSGSurface & mutable_surface = _surface_list.getSurface(surface.getName());
          mooseAssert(mutable_surface == surface,
                      "Mutable surface does not match const surface passed in.");
          mutable_surface.addTransformation(type, values);
        }
        else if constexpr (std::is_same_v<T, CSGUniverse>)
        {
          const CSGUniverse & universe = obj.get();
          if (!checkUniverseInBase(universe))
            mooseError("Cannot apply transformation to universe ",
                       universe.getName(),
                       " that is not in this CSGBase instance.");

          // Get non-const reference and apply transformation
          CSGUniverse & mutable_universe = _universe_list.getUniverse(universe.getName());
          mooseAssert(mutable_universe == universe,
                      "Mutable universe does not match const universe passed in.");
          mutable_universe.addTransformation(type, values);
        }
        else if constexpr (std::is_same_v<T, CSGLattice>)
        {
          const CSGLattice & lattice = obj.get();
          if (!checkLatticeInBase(lattice))
            mooseError("Cannot apply transformation to lattice ",
                       lattice.getName(),
                       " that is not in this CSGBase instance.");

          // Get non-const reference and apply transformation
          CSGLattice & mutable_lattice = _lattice_list.getLattice(lattice.getName());
          mooseAssert(mutable_lattice == lattice,
                      "Mutable lattice does not match const lattice passed in.");
          mutable_lattice.addTransformation(type, values);
        }
        else if constexpr (std::is_same_v<T, CSGRegion>)
        {
          // iterate on the surfaces of the region and apply the transformation to those surfaces
          const CSGRegion & region = obj.get();
          const auto surfaces = region.getSurfaces();
          for (const CSGSurface & surface : surfaces)
          {
            if (!checkSurfaceInBase(surface))
              mooseError("Cannot apply transformation to region with surface ",
                         surface.getName(),
                         " that is not in this CSGBase instance.");
            addTransformation(surface, type, values);
          }
        }
        else if constexpr (std::is_same_v<T, CSGEngUnit>)
        {
          const CSGEngUnit & eng_unit = obj.get();
          if (!checkEngUnitInBase(eng_unit))
            mooseError("Cannot apply transformation to engineering unit ",
                       eng_unit.getName(),
                       " that is not in this CSGBase instance.");

          CSGEngUnit & mutable_eng = _eng_unit_list.getEngUnit(eng_unit.getName());
          if (auto * s = dynamic_cast<CSGSurfaceEngUnit *>(&mutable_eng))
            s->addTransformation(type, values);
          else if (auto * c = dynamic_cast<CSGCellEngUnit *>(&mutable_eng))
            c->addTransformation(type, values);
          else if (auto * u = dynamic_cast<CSGUniverseEngUnit *>(&mutable_eng))
            u->addTransformation(type, values);
          else
            mooseError("Engineering unit '",
                       eng_unit.getName(),
                       "' has an unrecognized type for transformation.");
        }
        else
          mooseError("Transformation not implemented for this object type: ", typeid(T).name());
      },
      csg_object);
}

void
CSGBase::applyAxisRotation(const CSGObjectVariant & csg_object,
                           RotationAxisType axis,
                           const Real angle)
{
  // convert to the Euler angles (phi, theta, psi) based on axis
  Real phi = 0.0;
  Real theta = 0.0;
  Real psi = 0.0;

  switch (axis)
  {
    case RotationAxisType::X:
      theta = angle;
      break;
    case RotationAxisType::Y:
      phi = 90.0;
      theta = angle;
      psi = -90.0;
      break;
    case RotationAxisType::Z:
      phi = angle;
      break;
    default:
      mooseError("Invalid axis type provided for axis rotation.");
  }

  addTransformation(csg_object, TransformationType::ROTATION, std::make_tuple(phi, theta, psi));
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> base, const bool ignore_identical_components)
{
  // If we are ignoring identical incoming CSG components, we need to update any references
  // stored by these components to point to the references of the pre-existing CSGBase object
  if (ignore_identical_components)
    updateIncomingCSGReferences(*base);
  joinSurfaceList(base->getSurfaceList(), ignore_identical_components);
  joinCellList(base->getCellList(), ignore_identical_components);
  joinLatticeList(base->getLatticeList(), ignore_identical_components);
  joinUniverseList(base->getUniverseList(), ignore_identical_components);
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> base,
                       const bool ignore_identical_components,
                       const std::string & new_root_name_join)
{
  // If we are ignoring identical incoming CSG components, we need to update any references
  // stored by these components to point to the references of the pre-existing CSGBase object
  if (ignore_identical_components)
    updateIncomingCSGReferences(*base);
  joinSurfaceList(base->getSurfaceList(), ignore_identical_components);
  joinCellList(base->getCellList(), ignore_identical_components);
  joinLatticeList(base->getLatticeList(), ignore_identical_components);
  joinUniverseList(base->getUniverseList(), ignore_identical_components, new_root_name_join);
}

void
CSGBase::joinOtherBase(std::unique_ptr<CSGBase> base,
                       const bool ignore_identical_components,
                       const std::string & new_root_name_base,
                       const std::string & new_root_name_join)
{
  // If we are ignoring identical incoming CSG components, we need to update any references
  // stored by these components to point to the references of the pre-existing CSGBase object
  if (ignore_identical_components)
    updateIncomingCSGReferences(*base);
  joinSurfaceList(base->getSurfaceList(), ignore_identical_components);
  joinCellList(base->getCellList(), ignore_identical_components);
  joinLatticeList(base->getLatticeList(), ignore_identical_components);
  joinUniverseList(
      base->getUniverseList(), ignore_identical_components, new_root_name_base, new_root_name_join);
}

void
CSGBase::updateIncomingCSGReferences(CSGBase & incoming_base)
{
  // Iterate through all incoming surfaces and track which ones have names already
  // defined within this CSGSurfaceList object
  std::map<std::string, std::reference_wrapper<const CSGSurface>> identical_surface_refs;
  auto & surf_list_map = incoming_base.getSurfaceList().getSurfaceListMap();
  for (const auto & [surf_name, surf_ptr] : surf_list_map)
    if (hasSurface(surf_name))
      identical_surface_refs.insert({surf_name, getSurfaceByName(surf_name)});

  // Iterate through all incoming cells and track which ones have names already
  // defined within this CSGCellList object
  std::map<std::string, std::reference_wrapper<const CSGCell>> identical_cell_refs;
  auto & cell_list_map = incoming_base.getCellList().getCellListMap();
  for (const auto & [cell_name, cell_ptr] : cell_list_map)
    if (hasCell(cell_name))
      identical_cell_refs.insert({cell_name, getCellByName(cell_name)});

  // Iterate through all incoming universes and track which ones have names already
  // defined within this CSGUniverseList object
  std::map<std::string, std::reference_wrapper<const CSGUniverse>> identical_universe_refs;
  auto & universe_list_map = incoming_base.getUniverseList().getUniverseListMap();
  for (const auto & [univ_name, univ_ptr] : universe_list_map)
    if (hasUniverse(univ_name))
      identical_universe_refs.insert({univ_name, getUniverseByName(univ_name)});

  // Iterate through all incoming lattices and track which ones have names already
  // defined within this CSGLatticeList object
  std::map<std::string, std::reference_wrapper<const CSGLattice>> identical_lattice_refs;
  auto & lattice_list_map = incoming_base.getLatticeList().getLatticeListMap();
  for (const auto & [lat_name, lat_ptr] : lattice_list_map)
    if (hasLattice(lat_name))
      identical_lattice_refs.insert({lat_name, getLatticeByName(lat_name)});

  // Update all surface, cell, universe, and lattice references of incoming base to those of this
  // base
  if (!identical_surface_refs.empty())
    replaceSurfaceRefsByName(identical_surface_refs, incoming_base);

  if (!identical_cell_refs.empty())
    replaceCellRefsByName(identical_cell_refs, incoming_base);

  if (!identical_universe_refs.empty())
    replaceUniverseRefsByName(identical_universe_refs, incoming_base);

  if (!identical_lattice_refs.empty())
    replaceLatticeRefsByName(identical_lattice_refs, incoming_base);
}

void
CSGBase::replaceSurfaceRefsByName(
    std::map<std::string, std::reference_wrapper<const CSGSurface>> & identical_surface_refs,
    CSGBase & base)
{
  // Update surface references of cell regions to those of this base
  for (auto & [cell_name, cell_ptr] : base.getCellList().getCellListMap())
    cell_ptr->updateCellRegionSurfaces(identical_surface_refs);
}

void
CSGBase::replaceCellRefsByName(
    std::map<std::string, std::reference_wrapper<const CSGCell>> & identical_cell_refs,
    CSGBase & base)
{
  // Update cell references of universes to those of this base
  for (auto & [univ_name, univ_ptr] : base.getUniverseList().getUniverseListMap())
    for (auto & [cell_name, cell_ref] : identical_cell_refs)
      if (univ_ptr->hasCell(cell_name))
      {
        univ_ptr->removeCell(cell_name);
        univ_ptr->addCell(cell_ref);
      }
}

void
CSGBase::replaceUniverseRefsByName(
    std::map<std::string, std::reference_wrapper<const CSGUniverse>> & identical_universe_refs,
    CSGBase & base)
{
  // Update universe references of cells to those of this base
  for (auto & [cell_name, cell_ptr] : base.getCellList().getCellListMap())
  {
    const auto fill_type = cell_ptr->getFillType();
    const auto fill_name = cell_ptr->getFillName();
    if ((fill_type == "UNIVERSE") &&
        (identical_universe_refs.find(fill_name) != identical_universe_refs.end()))
    {
      const CSGUniverse * univ_ptr = &identical_universe_refs.at(fill_name).get();
      cell_ptr->updateCellFill(univ_ptr);
    }
  }

  // Update universe references of lattices to those of this base
  for (auto & [lat_name, lat_ptr] : base.getLatticeList().getLatticeListMap())
    for (auto & [univ_name, univ_ref] : identical_universe_refs)
    {
      // Check if universe belongs to lattice
      if (lat_ptr->hasUniverse(univ_name))
      {
        // If so, replace all instances of this universe in the lattice
        const auto univ_indices = lat_ptr->getUniverseIndices(univ_name);
        for (const auto & index : univ_indices)
          lat_ptr->setUniverseAtIndex(univ_ref, index);
      }
      // Check if universe belongs to lattice outer
      if ((lat_ptr->getOuterType() == "UNIVERSE") &&
          (lat_ptr->getOuterUniverse().getName() == univ_name))
        lat_ptr->updateOuter(univ_ref);
    }
}

void
CSGBase::replaceLatticeRefsByName(
    std::map<std::string, std::reference_wrapper<const CSGLattice>> & identical_lattice_refs,
    CSGBase & base)
{
  // Update lattice references of cells to those of this base
  for (auto & [cell_name, cell_ptr] : base.getCellList().getCellListMap())
  {
    const auto fill_type = cell_ptr->getFillType();
    const auto fill_name = cell_ptr->getFillName();
    if ((fill_type == "LATTICE") &&
        (identical_lattice_refs.find(fill_name) != identical_lattice_refs.end()))
    {
      const CSGLattice * lat_ptr = &identical_lattice_refs.at(fill_name).get();
      cell_ptr->updateCellFill(lat_ptr);
    }
  }
}

void
CSGBase::joinSurfaceList(CSGSurfaceList & surf_list, const bool ignore_identical_surfaces)
{
  auto & surf_list_map = surf_list.getSurfaceListMap();
  for (auto & s : surf_list_map)
    _surface_list.addSurface(std::move(s.second), ignore_identical_surfaces);
}

void
CSGBase::joinCellList(CSGCellList & cell_list, const bool ignore_identical_cells)
{
  auto & cell_list_map = cell_list.getCellListMap();
  for (auto & c : cell_list_map)
    _cell_list.addCell(std::move(c.second), ignore_identical_cells);
}

void
CSGBase::joinLatticeList(CSGLatticeList & lattice_list, const bool ignore_identical_lattices)
{
  auto & lat_list_map = lattice_list.getLatticeListMap();
  for (auto & lat : lat_list_map)
    _lattice_list.addLattice(std::move(lat.second), ignore_identical_lattices);
}

void
CSGBase::joinEngUnitList(CSGEngUnitList & eng_unit_list)
{
  // Objects are already owned by the type lists (moved via joinSurfaceList/joinCellList/
  // joinUniverseList). Copy the raw pointers into our index — they still point to the
  // objects now living in our type lists.
  for (auto * ptr : eng_unit_list._eng_units)
    _eng_unit_list.addEngUnit(*ptr);
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list, const bool ignore_identical_universes)
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
      _universe_list.addUniverse(std::move(u.second), ignore_identical_universes);
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list,
                          const bool ignore_identical_universes,
                          const std::string & new_root_name_incoming)
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
      _universe_list.addUniverse(std::move(u.second), ignore_identical_universes);
  }
}

void
CSGBase::joinUniverseList(CSGUniverseList & univ_list,
                          const bool ignore_identical_universes,
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
      _universe_list.addUniverse(std::move(u.second), ignore_identical_universes);
  }
}

void
CSGBase::checkRegionSurfaces(const CSGRegion & region) const
{
  const auto surfs = region.getSurfaces();
  for (const CSGSurface & s : surfs)
  {
    if (!checkSurfaceInBase(s))
      mooseError("Region is being set with a surface named " + s.getName() +
                 " that is different from the surface of the same name in the CSGBase instance.");
  }
}

bool
CSGBase::checkSurfaceInBase(const CSGSurface & surface) const
{
  auto name = surface.getName();
  // if no surface by this name exists, an error will be produced by getSurface
  auto & list_surf = _surface_list.getSurface(name);
  // return whether the surface in the list is the same object as the one provided
  return &surface == &list_surf;
}

bool
CSGBase::checkCellInBase(const CSGCell & cell) const
{
  auto name = cell.getName();
  // if no cell by this name exists, an error will be produced by getCell
  auto & list_cell = _cell_list.getCell(name);
  // return whether the cell in the list is the same object as the one provided
  return &cell == &list_cell;
}

bool
CSGBase::checkUniverseInBase(const CSGUniverse & universe) const
{
  auto name = universe.getName();
  // if no universe by this name exists, an error will be produced by getUniverse
  auto & list_univ = _universe_list.getUniverse(name);
  // return whether the universe in the list is the same object as the one provided
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

bool
CSGBase::checkEngUnitInBase(const CSGEngUnit & unit) const
{
  const auto & name = unit.getName();
  // if no engineering unit by this name exists, an error will be produced by getEngUnit
  const auto & list_unit = _eng_unit_list.getEngUnit(name);
  // compare CSGEngUnit subobject addresses
  return &unit == &list_unit;
}

void
CSGBase::renameEngUnit(const CSGEngUnit & unit, const std::string & name)
{
  // Rename in the owning type list (updates the object's name and the type list map key).
  // The EngUnit index stores raw pointers; because the object's name is updated in-place,
  // no index update is needed.
  if (const auto * surf = dynamic_cast<const CSGSurfaceEngUnit *>(&unit))
    renameSurface(static_cast<const CSGSurface &>(*surf), name);
  else if (const auto * cell = dynamic_cast<const CSGCellEngUnit *>(&unit))
    renameCell(static_cast<const CSGCell &>(*cell), name);
  else if (const auto * univ = dynamic_cast<const CSGUniverseEngUnit *>(&unit))
    renameUniverse(static_cast<const CSGUniverse &>(*univ), name);
  else
    mooseError(
        "Engineering unit '", unit.getName(), "' has an unrecognized type and cannot be renamed.");
}

void
CSGBase::renameSurface(const CSGSurface & surface, const std::string & name)
{
  // if surface is actually an engineering unit, we have to also check that no other units have the
  // same name already
  if (isSurfaceEngUnit(surface))
    if (_eng_unit_list.hasEngUnit(name))
      mooseError("Cannot rename surface " + surface.getName() + " to " + name + ". " +
                 surface.getName() + " is an engineering unit and a unit with name " + name +
                 " already exists.");

  _surface_list.renameSurface(surface, name);
}

void
CSGBase::renameCell(const CSGCell & cell, const std::string & name)
{
  // if cell is actually an engineering unit, we have to also check that no other units have the
  // same name already
  if (isCellEngUnit(cell))
    if (_eng_unit_list.hasEngUnit(name))
      mooseError("Cannot rename cell " + cell.getName() + " to " + name + ". " + cell.getName() +
                 " is an engineering unit and a unit with name " + name + " already exists.");

  _cell_list.renameCell(cell, name);
}

void
CSGBase::renameUniverse(const CSGUniverse & universe, const std::string & name)
{
  // if universe is actually an engineering unit, we have to also check that no other units have the
  // same name already
  if (isUniverseEngUnit(universe))
    if (_eng_unit_list.hasEngUnit(name))
      mooseError("Cannot rename universe " + universe.getName() + " to " + name + ". " +
                 universe.getName() + " is an engineering unit and a unit with name " + name +
                 " already exists.");

  _universe_list.renameUniverse(universe, name);
}

void
CSGBase::checkUniverseLinking() const
{
  std::vector<std::string> linked_universe_names;
  std::vector<std::string> linked_cell_names;

  // Recursively figure out which universe names are linked to root universe
  getLinkedUniverses(getRootUniverse(), linked_universe_names, linked_cell_names);

  // Iterate through combined list of universes and universe engineering units and check that they
  // exist in universes linked to root universe list.
  // (CSGUniverseEngUnit is derived from CSGUniverse, so they share the same base interface)
  auto all_univs = getAllUniverses();
  for (const CSGUniverseEngUnit & eu : getAllUniverseEngUnits())
    all_univs.emplace_back(static_cast<const CSGUniverse &>(eu));

  for (const CSGUniverse & univ : all_univs)
    if (std::find(linked_universe_names.begin(), linked_universe_names.end(), univ.getName()) ==
        linked_universe_names.end())
      mooseWarning("Universe with name ", univ.getName(), " is not linked to root universe.");

  // Iterate through all cells in cell list and check that they exist in cells linked
  // to root universe
  for (const CSGCell & cell : getAllCells())
    if (std::find(linked_cell_names.begin(), linked_cell_names.end(), cell.getName()) ==
        linked_cell_names.end())
      mooseWarning("Cell with name ", cell.getName(), " is not linked to root universe.");
}

void
CSGBase::getLinkedUniverses(const CSGUniverse & univ,
                            std::vector<std::string> & linked_universe_names,
                            std::vector<std::string> & linked_cell_names) const
{
  linked_universe_names.push_back(univ.getName());
  const auto & univ_cells = univ.getAllCells();
  for (const CSGCell & cell : univ_cells)
  {
    linked_cell_names.push_back(cell.getName());
    if (cell.getFillType() == "UNIVERSE")
      getLinkedUniverses(cell.getFillUniverse(), linked_universe_names, linked_cell_names);
    else if (cell.getFillType() == "LATTICE")
    {
      const auto & lattice = cell.getFillLattice();
      for (const auto & univ_list : lattice.getUniverses())
        for (const auto & univ_ref : univ_list)
        {
          const CSGUniverse & lattice_univ = univ_ref.get();
          getLinkedUniverses(lattice_univ, linked_universe_names, linked_cell_names);
        }

      if (lattice.getOuterType() == "UNIVERSE")
      {
        const CSGUniverse & outer_univ = lattice.getOuterUniverse();
        getLinkedUniverses(outer_univ, linked_universe_names, linked_cell_names);
      }
    }
  }
}

void
CSGBase::deleteEngUnit(const CSGEngUnit & unit)
{
  if (!checkEngUnitInBase(unit))
    mooseError("Engineering unit with name ",
               unit.getName(),
               " cannot be deleted as it is different from the engineering unit of the same name "
               "in the CSGBase instance.");

  // Delegate to the typed delete method — it handles EngUnit index cleanup and type list erasure
  if (const auto * surf_unit = dynamic_cast<const CSGSurfaceEngUnit *>(&unit))
    deleteSurface(static_cast<const CSGSurface &>(*surf_unit));
  else if (const auto * cell_unit = dynamic_cast<const CSGCellEngUnit *>(&unit))
    deleteCell(static_cast<const CSGCell &>(*cell_unit));
  else if (const auto * univ_unit = dynamic_cast<const CSGUniverseEngUnit *>(&unit))
    deleteUniverse(static_cast<const CSGUniverse &>(*univ_unit));
  else
    mooseError(
        "Engineering unit '", unit.getName(), "' has an unrecognized type and cannot be deleted.");
}

CSGRegion
CSGBase::expandEngUnit(const CSGSurfaceEngUnit & unit)
{
  // Get mutable reference from the owning surface list — expandUnit() is non-const
  auto & mutable_unit = static_cast<CSGSurfaceEngUnit &>(_surface_list.getSurface(unit.getName()));

  // Derived class creates the CSGSurface object(s) and adds them to CSGBase
  mutable_unit.expandUnit(*this);

  // Derived class provides the expanded region formed by the expanded surfaces
  CSGRegion exp_region = mutable_unit.getExpandedRegion();

  // Propagate any stored transformations from the EngUnit to all new expanded surfaces
  const auto & trans = static_cast<const CSGSurface &>(mutable_unit).getTransformations();
  if (!trans.empty())
    for (const auto & surf_ref : exp_region.getSurfaces())
    {
      CSGSurface & mutable_surf = _surface_list.getSurface(surf_ref.get().getName());
      for (const auto & [trans_type, values] : trans)
        mutable_surf.addTransformation(trans_type, values);
    }

  // Replace every CSGSurfaceEngUnit reference in regions of CSGCells with the expanded sub-region
  replaceSurfaceRefsWithRegion(static_cast<const CSGSurface &>(mutable_unit), exp_region);

  // Remove the EngUnit (destroyed here — no more references to it after
  // replaceSurfaceRefsWithRegion)
  deleteEngUnit(unit);
  return exp_region;
}

const CSGCell &
CSGBase::expandEngUnit(const CSGCellEngUnit & unit)
{
  // Get mutable reference from the owning cell list — expandUnit() is non-const
  auto & mutable_unit = static_cast<CSGCellEngUnit &>(_cell_list.getCell(unit.getName()));

  // Record whether the original unit lives in root before expansion — createCell() inside
  // expandUnit() might create a new cell which defaults to being added to root. If the original
  // cell unit was not a part of root, then the new cell should not be a part of root either.
  const bool unit_in_root = getRootUniverse().hasCell(unit.getName());

  // Derived class creates the CSGCell object and any other necessary objects and adds them to
  // CSGBase, storing the result internally for retrieval via getExpandedCell()
  mutable_unit.expandUnit(*this);
  const CSGCell & exp_cell = mutable_unit.getExpandedCell();

  // Propagate any stored transformations from the EngUnit to the expanded cell
  const auto & trans = static_cast<const CSGCell &>(mutable_unit).getTransformations();
  if (!trans.empty())
  {
    CSGCell & mutable_cell = _cell_list.getCell(exp_cell.getName());
    for (const auto & [trans_type, values] : trans)
      mutable_cell.addTransformation(trans_type, values);
  }

  // Replace all references to the CSGCellEngUnit in universes with the new expanded CSGCell
  replaceCellRefs(static_cast<const CSGCell &>(mutable_unit), exp_cell);

  // createCell() inside expandUnit() could add the new cell to root by default, even if the
  // original unit was not a part of root. If the original unit was not in root, remove the expanded
  // cell from root to match the original ownership.
  if (!unit_in_root && getRootUniverse().hasCell(exp_cell.getName()))
    removeCellFromUniverse(getRootUniverse(), exp_cell);

  // Remove the EngUnit (destroyed here — no more references to it after replaceCellRefs)
  deleteEngUnit(unit);
  return exp_cell;
}

const CSGUniverse &
CSGBase::expandEngUnit(const CSGUniverseEngUnit & unit)
{
  // Get mutable reference from the owning universe list — expandUnit() is non-const
  auto & mutable_unit =
      static_cast<CSGUniverseEngUnit &>(_universe_list.getUniverse(unit.getName()));

  // Derived class creates the CSGUniverse object and any other necessary objects and adds them to
  // CSGBase, storing the result internally for retrieval via getExpandedUniverse()
  mutable_unit.expandUnit(*this);
  const CSGUniverse & exp_univ = mutable_unit.getExpandedUniverse();

  // Propagate any stored transformations from the EngUnit to the new expanded universe
  const auto & trans = static_cast<const CSGUniverse &>(mutable_unit).getTransformations();
  if (!trans.empty())
  {
    CSGUniverse & mutable_univ = _universe_list.getUniverse(exp_univ.getName());
    for (const auto & [trans_type, values] : trans)
      mutable_univ.addTransformation(trans_type, values);
  }

  // Replace references in cell fills, lattice maps and outers, and the root universe
  replaceUniverseRefs(static_cast<const CSGUniverse &>(mutable_unit), exp_univ);

  // Remove the EngUnit (destroyed here — no more references to it after replaceUniverseRefs)
  deleteEngUnit(unit);
  return exp_univ;
}

void
CSGBase::replaceUniverseRefs(const CSGUniverse & old_univ, const CSGUniverse & new_univ)
{
  // 1. Cell fills
  for (const auto & cell_ref : getAllCells())
  {
    const CSGCell & cell = cell_ref.get();
    if (cell.getFillType() == "UNIVERSE" && cell.getFillUniverse() == old_univ)
      updateCellFill(cell, &new_univ);
  }

  // 2. Lattice universe maps and outer fills
  for (const auto & lat_ref : getAllLattices())
  {
    const CSGLattice & lat = lat_ref.get();
    if (lat.getOuterType() == "UNIVERSE" && lat.getOuterUniverse() == old_univ)
      setLatticeOuter(lat, new_univ);

    auto lat_map = lat.getUniverses();
    for (std::size_t row = 0; row < lat_map.size(); ++row)
      for (std::size_t col = 0; col < lat_map[row].size(); ++col)
        if (lat_map[row][col].get() == old_univ)
          setUniverseAtLatticeIndex(lat, new_univ, {static_cast<int>(row), static_cast<int>(col)});
  }

  // 3. Root universe pointer in universe list
  if (getRootUniverse() == old_univ)
    _universe_list._root_universe = &new_univ;
}

void
CSGBase::replaceCellRefs(const CSGCell & old_cell, const CSGCell & new_cell)
{
  for (const auto & univ_ref : getAllUniverses())
  {
    const CSGUniverse & univ = univ_ref.get();
    for (const auto & cell_ref : univ.getAllCells())
      if (&cell_ref.get() == &old_cell)
      {
        removeCellFromUniverse(univ, old_cell);
        addCellToUniverse(univ, new_cell);
      }
  }
}

void
CSGBase::replaceSurfaceRefsWithRegion(const CSGSurface & old_surf, const CSGRegion & sub_region)
{
  for (const auto & cell_ref : getAllCells())
  {
    const CSGCell & cell = cell_ref.get();
    CSGRegion new_region = cell.getRegion();
    new_region.replaceWithSubRegion(old_surf, sub_region);
    updateCellRegion(cell, new_region);
  }
}

void
CSGBase::expandAllEngUnits()
{
  // Snapshot raw pointers before expanding — expandEngUnit destroys each unit after expansion
  // so iterating live references would dangle.
  std::vector<const CSGSurfaceEngUnit *> surfs;
  std::vector<const CSGCellEngUnit *> cells;
  std::vector<const CSGUniverseEngUnit *> univs;
  for (const auto & u : getAllSurfaceEngUnits())
    surfs.push_back(&u.get());
  for (const auto & u : getAllCellEngUnits())
    cells.push_back(&u.get());
  for (const auto & u : getAllUniverseEngUnits())
    univs.push_back(&u.get());

  for (const auto * s : surfs)
    expandEngUnit(*s);
  for (const auto * c : cells)
    expandEngUnit(*c);
  for (const auto * u : univs)
    expandEngUnit(*u);
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
    if (isSurfaceEngUnit(s))
      continue; // engineering units are written in a separate section
    const auto & surf_name = s.getName();
    const auto & coeffs = s.getCoeffs();
    csg_json["surfaces"][surf_name] = {{"type", s.getSurfaceType()}, {"coefficients", {}}};
    for (const auto & c : coeffs)
      csg_json["surfaces"][surf_name]["coefficients"][c.first] = c.second;
    // include any information about transformations if present
    if (s.getTransformations().size() > 0)
      csg_json["surfaces"][surf_name]["transformations"] = s.getTransformationsAsStrings();
  }

  // Print out cell information
  auto all_cells = getAllCells();
  for (const CSGCell & c : all_cells)
  {
    if (isCellEngUnit(c))
      continue; // engineering units are written in a separate section
    const auto & cell_name = c.getName();
    const auto & cell_region_infix = c.getRegion().toInfixJSON();
    const auto & cell_region_postfix = c.getRegion().toPostfixStringList();
    const auto & cell_filltype = c.getFillType();
    const auto & fill_name = c.getFillName();
    csg_json["cells"][cell_name]["filltype"] = cell_filltype;
    csg_json["cells"][cell_name]["region_infix"] = cell_region_infix;
    csg_json["cells"][cell_name]["region_postfix"] = cell_region_postfix;
    csg_json["cells"][cell_name]["fill"] = fill_name;
    // include any information about transformations if present
    if (c.getTransformations().size())
      csg_json["cells"][cell_name]["transformations"] = c.getTransformationsAsStrings();
  }

  // Print out universe information
  auto all_univs = getAllUniverses();
  for (const CSGUniverse & u : all_univs)
  {
    if (isUniverseEngUnit(u))
      continue; // engineering units are written in a separate section
    const auto & univ_name = u.getName();
    const auto & univ_cells = u.getAllCells();
    csg_json["universes"][univ_name]["cells"] = {};
    for (const CSGCell & c : univ_cells)
      csg_json["universes"][univ_name]["cells"].push_back(c.getName());
    if (u.isRoot())
      csg_json["universes"][univ_name]["root"] = u.isRoot();
    // include any information about transformations if present
    if (u.getTransformations().size())
      csg_json["universes"][univ_name]["transformations"] = u.getTransformationsAsStrings();
  }

  // print out lattice information if lattices exist
  auto all_lats = getAllLattices();
  if (all_lats.size())
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
      // include any information about transformations if present
      if (lat.getTransformations().size())
        csg_json["lattices"][lat_name]["transformations"] = lat.getTransformationsAsStrings();
    }
  }

  // include engineering units if they exist
  auto all_units = getAllEngUnits();
  if (all_units.size())
  {
    csg_json["units"] = {};
    for (const CSGEngUnit & unit : all_units)
    {
      const auto & unit_name = unit.getName();
      csg_json["units"][unit_name] = {};
      // behavior and type
      csg_json["units"][unit_name]["unit_type"] = unit.getUnitType();
      csg_json["units"][unit_name]["behavior"] = unit.getBehavior();
      csg_json["units"][unit_name]["attributes"] = {};
      // any unit-specific attributes
      const auto & unit_attrs = unit.getAttributes();
      for (const auto & attr : unit_attrs)
        csg_json["units"][unit_name]["attributes"][attr.first] = attr.second;
      if (unit.getTransformations().size())
        csg_json["units"][unit_name]["transformations"] = unit.getTransformationsAsStrings();
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
  const auto & eng_unit_list = this->getEngUnitList();
  const auto & other_eng_unit_list = other.getEngUnitList();
  return (surf_list == other_surf_list) && (cell_list == other_cell_list) &&
         (univ_list == other_univ_list) && (lat_list == other_lat_list) &&
         (eng_unit_list == other_eng_unit_list);
}

bool
CSGBase::operator!=(const CSGBase & other) const
{
  return !(*this == other);
}
} // namespace CSG
