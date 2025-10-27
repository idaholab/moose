//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurfaceList.h"
#include "CSGRegion.h"
#include "CSGCellList.h"
#include "CSGUniverseList.h"
#include "CSGLatticeList.h"
#include "nlohmann/json.h"

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

namespace CSG
{

/**
 * CSGBase creates an internal representation of a Constructive Solid Geometry (CSG)
 * model.
 */
class CSGBase
{
public:
  /**
   * Default constructor
   */
  CSGBase();

  /**
   * Copy constructor
   */
  CSGBase(const CSGBase & other_base);

  /**
   * Destructor
   */
  ~CSGBase();

  /// Create a deep copy of this CSGBase instance
  std::unique_ptr<CSGBase> clone() const { return std::make_unique<CSGBase>(*this); }

  /**
   * @brief add a unique surface pointer to this base instance
   *
   * @param surf pointer to surface to add
   *
   * @return reference to CSGSurface that was added
   */
  const CSGSurface & addSurface(std::unique_ptr<CSGSurface> surf)
  {
    return _surface_list.addSurface(std::move(surf));
  }

  /**
   * @brief Get all surface objects
   *
   * @return list of references to all CSGSurface objects in CSGBase
   */
  std::vector<std::reference_wrapper<const CSGSurface>> getAllSurfaces() const
  {
    return _surface_list.getAllSurfaces();
  }

  /**
   * @brief Get a Surface object by name
   *
   * @param name surface name
   * @return reference to CSGSurface object
   */
  const CSGSurface & getSurfaceByName(const std::string & name) const
  {
    return _surface_list.getSurface(name);
  }

  /**
   * @brief rename the specified surface
   *
   * @param surface CSGSurface to rename
   * @param name new name
   */
  void renameSurface(const CSGSurface & surface, const std::string & name)
  {
    _surface_list.renameSurface(surface, name);
  }

  /**
   * @brief Create a Material Cell object
   *
   * @param name unique cell name
   * @param mat_name material name
   * @param region cell region
   * @param add_to_univ (optional) universe to which this cell will be added (default is root
   * universe)
   * @return reference to CSGCell that is created
   */
  const CSGCell & createCell(const std::string & name,
                             const std::string & mat_name,
                             const CSGRegion & region,
                             const CSGUniverse * add_to_univ = nullptr);

  /**
   * @brief Create a Void Cell object
   *
   * @param name unique cell name
   * @param region cell region
   * @param add_to_univ (optional) universe to which this cell will be added (default is root
   * universe)
   * @return reference to CSGCell that is created
   */
  const CSGCell & createCell(const std::string & name,
                             const CSGRegion & region,
                             const CSGUniverse * add_to_univ = nullptr);

  /**
   * @brief Create a Universe Cell object
   *
   * @param name unique cell name
   * @param fill_univ universe that will fill the cell
   * @param region cell region
   * @param add_to_univ (optional) universe to which this cell will be added (default is root
   * universe)
   * @return reference to cell that is created
   */
  const CSGCell & createCell(const std::string & name,
                             const CSGUniverse & fill_univ,
                             const CSGRegion & region,
                             const CSGUniverse * add_to_univ = nullptr);

  /**
   * @brief Get all cell objects
   *
   * @return list of references to all CSGCell objects in CSGBase
   */
  std::vector<std::reference_wrapper<const CSGCell>> getAllCells() const
  {
    return _cell_list.getAllCells();
  }

  /**
   * @brief Get a Cell object by name
   *
   * @param name cell name
   * @return reference to CSGCell object
   */
  const CSGCell & getCellByName(const std::string & name) const { return _cell_list.getCell(name); }

  /**
   * @brief rename the specified cell
   *
   * @param cell reference to CSGCell to rename
   * @param name new name
   */
  void renameCell(const CSGCell & cell, const std::string & name)
  {
    _cell_list.renameCell(cell, name);
  }

  /**
   * @brief change the region of the specified cell
   *
   * @param cell cell to update the region for
   * @param region new region to assign to cell
   */
  void updateCellRegion(const CSGCell & cell, const CSGRegion & region);

  /**
   * @brief Get the Root Universe object
   *
   * @return reference to root CSGUniverse
   */
  const CSGUniverse & getRootUniverse() const { return _universe_list.getRoot(); }

  /**
   * @brief rename the root universe for this instance (default is ROOT_UNIVERSE)
   *
   * @param name new name for the root universe
   */
  void renameRootUniverse(const std::string & name)
  {
    _universe_list.renameUniverse(_universe_list.getRoot(), name);
  }

  /**
   * @brief rename the specified universe
   *
   * @param universe reference to CSGUniverse to rename
   * @param name new name
   */
  void renameUniverse(const CSGUniverse & universe, const std::string & name)
  {
    _universe_list.renameUniverse(universe, name);
  }

  /**
   * @brief Create an empty Universe object
   *
   * @param name unique universe name
   * @return reference to CSGUniverse that is created
   */
  const CSGUniverse & createUniverse(const std::string & name)
  {
    return _universe_list.addUniverse(name);
  }

  /**
   * @brief Create a Universe object from list of cells
   *
   * @param name unique universe name
   * @param cells list of cells to add to universe
   * @return reference to CSGUniverse that is created
   */
  const CSGUniverse & createUniverse(const std::string & name,
                                     std::vector<std::reference_wrapper<const CSGCell>> & cells);

  /**
   * @brief Add a cell to an existing universe
   *
   * @param universe universe to which to add the cell
   * @param cell cell to add
   */
  void addCellToUniverse(const CSGUniverse & universe, const CSGCell & cell);

  /**
   * @brief Add a list of cells to an existing universe
   *
   * @param universe universe to which to add the cells
   * @param cells list of references to cells to add
   */
  void addCellsToUniverse(const CSGUniverse & universe,
                          std::vector<std::reference_wrapper<const CSGCell>> & cells);

  /**
   * @brief Remove a cell from an existing universe
   *
   * @param universe universe from which to remove the cell
   * @param cell cell to remove
   */
  void removeCellFromUniverse(const CSGUniverse & universe, const CSGCell & cell);

  /**
   * @brief Remove a list of cells from an existing universe
   *
   * @param universe universe from which to remove the cells
   * @param cells list of references to cells to remove
   */
  void removeCellsFromUniverse(const CSGUniverse & universe,
                               std::vector<std::reference_wrapper<const CSGCell>> & cells);

  /**
   * @brief Get all universe objects
   *
   * @return list of references to CSGUniverse objects in this CSGBase instance
   */
  std::vector<std::reference_wrapper<const CSGUniverse>> getAllUniverses() const
  {
    return _universe_list.getAllUniverses();
  }

  /**
   * @brief Get a universe object by name
   *
   * @param name universe name
   * @return reference to CSGUniverse object
   */
  const CSGUniverse & getUniverseByName(const std::string & name)
  {
    return _universe_list.getUniverse(name);
  }

  /**
   * @brief Create a Cartesian Lattice from the provided universes
   *
   * @param name unique name identifier for the lattice
   * @param pitch flat-to-flat size of the lattice elements
   * @param universes list of list of universes that define the layout of the lattice
   * @return reference to new Cartesian lattice
   */
  const CSGLattice & createCartesianLattice(
      const std::string & name,
      const Real pitch,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> universes);

  /**
   * @brief Create an empty Cartesian lattice. NOTE: this will not create an empty universe mapping;
   * this must be done by using setLatticeUniverses.
   *
   * @param name unique name identifier for the lattice
   * @param nx0 number of lattice elements in the first dimension (rows)
   * @param nx1 number of lattice elements in the second dimension (columns)
   * @param pitch flat-to-flat size of the lattice elements
   * @return reference to new Cartesian lattice
   */
  const CSGLattice &
  createCartesianLattice(const std::string & name, const int nx0, const int nx1, const Real pitch)
  {
    return _lattice_list.addCartesianLattice(name, nx0, nx1, pitch);
  }

  /**
   * @brief add the universe to the lattice at the specified location
   *
   * @param lattice lattice to update
   * @param universe universe to set at the location
   * @param index index of the lattice element (int, int)
   */
  void addUniverseToLattice(const CSGLattice & lattice,
                            const CSGUniverse & universe,
                            std::pair<int, int> index);

  /**
   * @brief Set provided universes as the layout of the lattice.
   *
   * @param lattice lattice to add universes to
   * @param universes list of list of universes in the proper layout for the lattice type and
   * dimensions
   */
  void setLatticeUniverses(
      const CSGLattice & lattice,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> & universes);

  /**
   * @brief rename the lattice
   *
   * @param lattice lattice to rename
   * @param name new name
   */
  void renameLattice(const CSGLattice & lattice, const std::string & name)
  {
    _lattice_list.renameLattice(lattice, name);
  }

  /**
   * @brief change a dimension of the lattice
   *
   * @param lattice lattice to update
   * @param dim_name name of the dimension to change
   * @param dim_value new value for the dimension
   */
  void
  updateLatticeDimension(CSGLattice & lattice, const std::string & dim_name, std::any dim_value)
  {
    lattice.updateDimension(dim_name, dim_value);
  }

  /**
   * @brief Get all lattice objects
   *
   * @return list of references to CSGLattice objects in this CSGBase instance
   */
  std::vector<std::reference_wrapper<const CSGLattice>> getAllLattices() const
  {
    return _lattice_list.getAllLattices();
  }

  /**
   * @brief Get a lattice object by name
   *
   * @param name lattice name
   * @return reference to CSGLattice object
   */
  const CSGLattice & getLatticeByName(const std::string & name)
  {
    return _lattice_list.getLattice(name);
  }

  /**
   * @brief Join another CSGBase object to this one. The cells of the root universe
   * of the incoming CSGBase will be added to the existing root universe of this
   * CSGBase.
   *
   * @param base pointer to a different CSGBase object
   */
  void joinOtherBase(std::unique_ptr<CSGBase> base);

  /**
   * @brief Join another CSGBase object to this one. For the incoming CSGBase object,
   * the root universe is added to this CSGBase object as a new non-root universe with
   * the specified new name.
   * Note: this newly created universe will not be connected to the root universe
   * of this CSGBase object by default.
   *
   * @param base pointer to a different CSGBase object
   * @param new_root_name_join new name for the universe generated from the incoming root universe
   */
  void joinOtherBase(std::unique_ptr<CSGBase> base, std::string & new_root_name_join);

  /**
   * @brief Join another CSGBase object to this one. The root universe for the incoming CSGBase
   * object is added to this CSGBase object as a non-root universe with a new name. The root
   * universe of this CSGBase object will be renamed and designated as non-root.
   * Note: upon completion of this join method, the root universe of this CSGBase
   * object will be empty. Neither of the new non-root universes will be connected to the
   * new root universe by default.
   *
   * @param base pointer to a different CSGBase object
   * @param new_root_name_base new name for universe generated from this root universe
   * @param new_root_name_join new name for the universe generated from the incoming root universe
   */
  void joinOtherBase(std::unique_ptr<CSGBase> base,
                     const std::string & new_root_name_base,
                     const std::string & new_root_name_join);

  /**
   * @brief generate the JSON representation output for the CSG object
   *
   */
  nlohmann::json generateOutput() const;

  /// Operator overload for checking if two CSGBase objects are equal
  bool operator==(const CSGBase & other) const;

  /// Operator overload for checking if two CSGBase objects are not equal
  bool operator!=(const CSGBase & other) const;

private:
  /**
   * @brief Get a Surface object by name.
   *
   * Note:  This is a private method that returns a non-const reference. For the public method that
   * returns a const reference, use `getSurfaceByName`
   *
   * @param name surface name
   * @return reference to CSGSurface object
   */
  CSGSurface & getSurface(const std::string & name) { return _surface_list.getSurface(name); }

  /// Check universes linked to root universe match universes defined in _universe_list
  void checkUniverseLinking() const;

  /**
   * @brief Recursive method to retrieve all universes linked to current universe
   *
   * @param univ Reference to universe under consideration
   * @param linked_universe_name List of universe names linked to current universe
   */
  void getLinkedUniverses(const CSGUniverse & univ,
                          std::vector<std::string> & linked_universe_names) const;

  /**
   * @brief Get a const reference to the CSGSurfaceList object
   *
   * @return CSGSurfaceList
   */
  const CSGSurfaceList & getSurfaceList() const { return _surface_list; }

  /**
   * @brief Get a non-const reference to the CSGSurfaceList object
   *
   * @return CSGSurfaceList
   */
  CSGSurfaceList & getSurfaceList() { return _surface_list; }

  /**
   * @brief Get a const reference to the CSGCellList object
   *
   * @return CSGCellList
   */
  const CSGCellList & getCellList() const { return _cell_list; }

  /**
   * @brief Get a non-const reference to the CSGCellList object
   *
   * @return CSGCellList
   */
  CSGCellList & getCellList() { return _cell_list; }

  /**
   * @brief Get a const reference to the CSGUniverseList object
   *
   * @return CSGUniverseList
   */
  const CSGUniverseList & getUniverseList() const { return _universe_list; }

  /**
   * @brief Get a non-const reference to the CSGUniverseList object
   *
   * @return CSGUniverseList
   */
  CSGUniverseList & getUniverseList() { return _universe_list; }

  /**
   * @brief join a separate CSGSurfaceList object to this one
   *
   * @param surf_list CSGSurfaceList from a separate CSGBase object
   */
  void joinSurfaceList(CSGSurfaceList & surf_list);

  /**
   * @brief join a separate CSGCellList object to this one
   *
   * @param cell_list CSGCellList from a separate CSGBase object
   */
  void joinCellList(CSGCellList & cell_list);

  /**
   * @brief join a separate CSGUniverseList object to this one;
   * root universes from univ_list will be combined into this root universe
   *
   * @param univ_list CSGUniverseList from a separate CSGBase object
   */
  void joinUniverseList(CSGUniverseList & univ_list);

  /**
   * @brief join a separate CSGUniverseList object to this one;
   * the incoming root universe will be moved to a new universe of the new
   * name specified.
   *
   * @param univ_list CSGUniverseList from a separate CSGBase object
   * @param new_root_name_incoming new name for the universe generated from the incoming root
   * universe
   */
  void joinUniverseList(CSGUniverseList & univ_list, const std::string & new_root_name_incoming);

  /**
   * @brief join a separate CSGUniverseList object to this one;
   * both this root universe and the incoming root universe will be
   * maintained as separate universes of the specified names.
   * Note: upon completion of this join method, the root universe will be empty.
   *
   * @param univ_list CSGUniverseList from a separate CSGBase object
   * @param new_root_name_base new name for universe generated from this root universe
   * @param new_root_name_incoming new name for the universe generated from the incoming root
   * universe
   */
  void joinUniverseList(CSGUniverseList & univ_list,
                        const std::string & new_root_name_base,
                        const std::string & new_root_name_incoming);

  // check that surfaces used in this region are a part of this CSGBase instance
  void checkRegionSurfaces(const CSGRegion & region) const;

  // check that cell being accessed is a part of this CSGBase instance
  bool checkCellInBase(const CSGCell & cell) const;

  // check that universe being accessed is a part of this CSGBase instance
  bool checkUniverseInBase(const CSGUniverse & universe) const;

  /**
   * @brief Add a new cell to the cell list based on a cell reference.
   * This method is called by the copy constructor of CSGBase
   *
   * @param cell reference to CSGCell that should be added to cell list
   */
  const CSGCell & addCellToList(const CSGCell & cell);

  /**
   * @brief Add a new universe to the universe list based on a universe reference.
   * This method is called by the copy constructor of CSGBase
   *
   * @param univ reference to CSGUniverse that should be added to universe list
   */
  const CSGUniverse & addUniverseToList(const CSGUniverse & univ);

  /// List of surfaces associated with CSG object
  CSGSurfaceList _surface_list;

  /// List of cells associated with CSG object
  CSGCellList _cell_list;

  /// List of universes associated with CSG object
  CSGUniverseList _universe_list;

  /// List of lattices associated with CSG object
  CSGLatticeList _lattice_list;

#ifdef MOOSE_UNIT_TEST
  /// Friends for unit testing
  ///@{
  FRIEND_TEST(CSGBaseTest, testCheckRegionSurfaces);
  FRIEND_TEST(CSGBaseTest, testAddGetSurface);
  FRIEND_TEST(CSGBaseTest, testUniverseLinking);
  ///@}
#endif
};
} // namespace CSG
