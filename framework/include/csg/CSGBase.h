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
#include "CSGPlane.h"
#include "CSGSphere.h"
#include "CSGXCylinder.h"
#include "CSGYCylinder.h"
#include "CSGZCylinder.h"
#include "CSGRegion.h"
#include "CSGCellList.h"
#include "CSGUniverseList.h"
#include "nlohmann/json.h"

namespace CSG
{

/**
 * CSGBase creates an internal representation of a Constructive Solid Geometry (CSG)
 * mesh based on an existing MooseMesh instance.
 */
class CSGBase
{
public:
  /**
   * Default constructor
   */
  CSGBase();

  /**
   * Destructor
   */
  ~CSGBase();

  /**
   * @brief Create a plane from three points
   *
   * @param name surface name
   * @param p1 point 1
   * @param p2 point 2
   * @param p3 point 3
   * @param boundary (optional) CSGSurface::BoundaryType boundary condition for the surface (default
   * TRANSMISSION)
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createPlaneFromPoints(const std::string name,
                        const Point p1,
                        const Point p2,
                        const Point p3,
                        CSGSurface::BoundaryType boundary = CSGSurface::BoundaryType::TRANSMISSION)
  {
    return _surface_list.addPlaneFromPoints(name, p1, p2, p3, boundary);
  }

  /**
   * @brief Create a plane from coefficients for the equation: ax + by + cz = d
   *
   * @param name surface name
   * @param a coefficient a
   * @param b coefficient b
   * @param c coefficient c
   * @param d coefficient d
   * @param boundary (optional) CSGSurface::BoundaryType boundary condition for the surface (default
   * TRANSMISSION)
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface> createPlaneFromCoefficients(
      const std::string name,
      const Real a,
      const Real b,
      const Real c,
      const Real d,
      CSGSurface::BoundaryType boundary = CSGSurface::BoundaryType::TRANSMISSION)
  {
    return _surface_list.addPlaneFromCoefficients(name, a, b, c, d, boundary);
  }

  /**
   * @brief Create a Sphere At Origin
   *
   * @param name surface name
   * @param r radius
   * @param boundary (optional) CSGSurface::BoundaryType boundary condition for the surface (default
   * TRANSMISSION)
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createSphere(const std::string name,
               const Real r,
               CSGSurface::BoundaryType boundary = CSGSurface::BoundaryType::TRANSMISSION)
  {
    return _surface_list.addSphere(name, Point(0.0, 0.0, 0.0), r, boundary);
  }

  /**
   * @brief Create a Sphere centered at a point
   *
   * @param name surface name
   * @param center a point defining the center
   * @param r radius
   * @param boundary (optional) CSGSurface::BoundaryType boundary condition for the surface (default
   * TRANSMISSION)
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createSphere(const std::string name,
               const Point center,
               const Real r,
               CSGSurface::BoundaryType boundary = CSGSurface::BoundaryType::TRANSMISSION)
  {
    return _surface_list.addSphere(name, center, r, boundary);
  }

  /**
   * @brief Create a Cylinder aligned with an axis (x, y or z) at the point
   * (x0, x1), where x0 and x1 correspond to:
   * x aligned: (y, z)
   * y aligned: (x, z)
   * z aligned: (x, y)
   *
   * @param name surface name
   * @param x0 first coordinate for center
   * @param x1 second coordinate for center
   * @param r radius
   * @param boundary (optional) CSGSurface::BoundaryType boundary condition for the surface (default
   * TRANSMISSION)
   * @return shared pointer to CSGSurface object
   */
  std::shared_ptr<CSGSurface>
  createCylinder(const std::string name,
                 const Real x0,
                 const Real x1,
                 const Real r,
                 const std::string axis,
                 CSGSurface::BoundaryType boundary = CSGSurface::BoundaryType::TRANSMISSION)
  {
    return _surface_list.addCylinder(name, x0, x1, r, axis, boundary);
  }

  /**
   * @brief Get all surface objects
   *
   * @return map of names to CSGSurface objects
   */
  const std::map<std::string, std::shared_ptr<CSGSurface>> & getAllSurfaces() const
  {
    return _surface_list.getAllSurfaces();
  }

  /**
   * @brief Get a Surface object by name
   *
   * @param name surface name
   * @return shared pointer to CSGSurface object
   */
  const std::shared_ptr<CSGSurface> & getSurfaceByName(const std::string name)
  {
    return _surface_list.getSurface(name);
  }

  /**
   * @brief rename the specified surface
   *
   * @param surface CSGSurface to rename
   * @param name new name
   */
  void renameSurface(const std::shared_ptr<CSGSurface> surface, const std::string name)
  {
    _surface_list.renameSurface(surface, name);
  }

  /**
   * @brief change the boundary condition of a surface
   *
   * @param surface CSGSurface to update
   * @param boundary CSGSurface::BoundaryType to set
   */
  void updateSurfaceBoundaryCondition(const std::shared_ptr<CSGSurface> surface,
                                      CSGSurface::BoundaryType boundary)
  {
    surface->setBoundaryType(boundary);
  }

  /**
   * @brief Create a Material Cell object
   *
   * @param name unique cell name
   * @param mat_name material name (TODO: this will eventually be a material object and not just a
   * name)
   * @param region cell region
   * @param add_to_univ (optional) universe to which this cell will be added (default is root
   * universe)
   * @return std::shared_ptr<CSGCell> pointer to CSGCell that is created
   */
  std::shared_ptr<CSGCell> createCell(const std::string name,
                                      const std::string mat_name,
                                      const CSGRegion & region,
                                      const std::shared_ptr<CSGUniverse> add_to_univ = nullptr);

  /**
   * @brief Create a Void Cell object
   *
   * @param name unique cell name
   * @param region cell region
   * @param add_to_univ (optional) universe to which this cell will be added (default is root
   * universe)
   * @return std::shared_ptr<CSGCell> pointer to CSGCell that is created
   */
  std::shared_ptr<CSGCell> createCell(const std::string name,
                                      const CSGRegion & region,
                                      const std::shared_ptr<CSGUniverse> add_to_univ = nullptr);

  /**
   * @brief Create a Universe Cell object
   *
   * @param name unique cell name
   * @param fill_univ universe that will fill the cell
   * @param region cell region
   * @param add_to_univ (optional) universe to which this cell will be added (default is root
   * universe)
   * @return std::shared_ptr<CSGCell> pointer to cell that is created
   */
  std::shared_ptr<CSGCell> createCell(const std::string name,
                                      const std::shared_ptr<CSGUniverse> fill_univ,
                                      const CSGRegion & region,
                                      const std::shared_ptr<CSGUniverse> add_to_univ = nullptr);

  /**
   * @brief Get all cell objects
   *
   * @return map of all names to CSGCell objects in this CSGBase instance
   */
  const std::map<std::string, std::shared_ptr<CSGCell>> & getAllCells() const
  {
    return _cell_list.getAllCells();
  }

  /**
   * @brief Get a Cell object by name
   *
   * @param name cell name
   * @return shared pointer to CSGCell object
   */
  const std::shared_ptr<CSGCell> & getCellByName(const std::string name)
  {
    return _cell_list.getCell(name);
  }

  /**
   * @brief rename the specified cell
   *
   * @param cell pointer to CSGCell to rename
   * @param name new name
   */
  void renameCell(const std::shared_ptr<CSGCell> cell, const std::string name)
  {
    _cell_list.renameCell(cell, name);
  }

  /**
   * @brief change the region of the specified cell
   *
   * @param cell cell to update the region for
   * @param region new region to assign to cell
   */
  void updateCellRegion(const std::shared_ptr<CSGCell> cell, const CSGRegion & region);

  /**
   * @brief Get the Root Universe object
   *
   * @return  shared pointer to CSGUniverse
   */
  std::shared_ptr<CSGUniverse> getRootUniverse() const { return _universe_list.getRoot(); }

  /**
   * @brief rename the root universe for this instance (default is ROOT_UNIVERSE)
   *
   * @param name new name for the root universe
   */
  void renameRootUniverse(const std::string name)
  {
    _universe_list.renameUniverse(_universe_list.getRoot(), name);
  }

  /**
   * @brief rename the specified universe
   *
   * @param universe pointer to CSGUniverse to rename
   * @param name new name
   */
  void renameUniverse(const std::shared_ptr<CSGUniverse> universe, const std::string name)
  {
    _universe_list.renameUniverse(universe, name);
  }

  /**
   * @brief Create an empty Universe object
   *
   * @param name unique universe name
   * @return std::shared_ptr<CSGUniverse> pointer CSGUniverse that is created
   */
  std::shared_ptr<CSGUniverse> createUniverse(const std::string name)
  {
    return _universe_list.addUniverse(name);
  }

  /**
   * @brief Create a Universe object from list of cells
   *
   * @param name unique universe name
   * @param cells list of cells to add to universe
   * @return std::shared_ptr<CSGUniverse> pointer CSGUniverse that is created
   */
  std::shared_ptr<CSGUniverse> createUniverse(const std::string name,
                                              std::vector<std::shared_ptr<CSGCell>> cells);

  /**
   * @brief Add a cell to an existing universe
   *
   * @param universe universe to which to add the cell
   * @param cell cell to add
   */
  void addCellToUniverse(const std::shared_ptr<CSGUniverse> universe,
                         std::shared_ptr<CSGCell> cell);

  /**
   * @brief Add a list of cells to an existing universe
   *
   * @param universe universe to which to add the cells
   * @param cells list of cells to add
   */
  void addCellsToUniverse(const std::shared_ptr<CSGUniverse> universe,
                          std::vector<std::shared_ptr<CSGCell>> cells);

  /**
   * @brief Remove a cell from an existing universe
   *
   * @param universe universe from which to remove the cell
   * @param cell cell to remove
   */
  void removeCellFromUniverse(const std::shared_ptr<CSGUniverse> universe,
                              std::shared_ptr<CSGCell> cell);

  /**
   * @brief Remove a list of cells from an existing universe
   *
   * @param universe universe from which to remove the cells
   * @param cells list of cells to remove
   */
  void removeCellsFromUniverse(const std::shared_ptr<CSGUniverse> universe,
                               std::vector<std::shared_ptr<CSGCell>> cells);

  /**
   * @brief Get all universe objects
   *
   * @return map of all names to CSGUniverse objects in this CSGBase instance
   */
  const std::map<std::string, std::shared_ptr<CSGUniverse>> & getAllUniverses() const
  {
    return _universe_list.getAllUniverses();
  }

  /**
   * @brief Get a universe object by name
   *
   * @param name universe name
   * @return shared pointer to CSGUniverse object
   */
  const std::shared_ptr<CSGUniverse> & getUniverseByName(const std::string name)
  {
    return _universe_list.getUniverse(name);
  }

  /**
   * @brief Join another CSGBase object to this one. The cells of the root universe
   * of the incoming CSGBase will be added to the existing root universe of this
   * CSGBase.
   *
   * @param base pointer to a different CSGBase object
   */
  void joinOtherBase(std::unique_ptr<CSGBase> & base)
  {
    joinSurfaceList(base->getSurfaceList());
    joinCellList(base->getCellList());
    joinUniverseList(base->getUniverseList());
  }

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
  void joinOtherBase(std::unique_ptr<CSGBase> & base, std::string new_root_name_join)
  {
    joinSurfaceList(base->getSurfaceList());
    joinCellList(base->getCellList());
    joinUniverseList(base->getUniverseList(), new_root_name_join);
  }

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
  void joinOtherBase(std::unique_ptr<CSGBase> & base,
                     std::string new_root_name_base,
                     std::string new_root_name_join)
  {
    joinSurfaceList(base->getSurfaceList());
    joinCellList(base->getCellList());
    joinUniverseList(base->getUniverseList(), new_root_name_base, new_root_name_join);
  }

  /**
   * @brief generate the JSON representation output for the CSG object
   *
   */
  nlohmann::json generateOutput() const;

private:
  /// Check universes linked to root universe match universes defined in _universe_list
  void checkUniverseLinking() const;

  /**
   * @brief Recursive method to retrieve all universes linked to current universe
   *
   * @param univ Pointer to universe under consideration
   * @param linked_universe_name List of universe names linked to current universe
   */
  void getLinkedUniverses(const std::shared_ptr<CSGUniverse> & univ,
                          std::vector<std::string> & linked_universe_names) const;

  /**
   * @brief Get the CSGSurfaceList object
   *
   * @return CSGSurfaceList
   */
  CSGSurfaceList & getSurfaceList() { return _surface_list; }

  /**
   * @brief Get the CSGCellList object
   *
   * @return CSGCellList
   */
  CSGCellList & getCellList() { return _cell_list; }

  /**
   * @brief Get the CSGUniverseList object
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
  void joinUniverseList(CSGUniverseList & univ_list, std::string new_root_name_incoming);

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
                        std::string new_root_name_base,
                        std::string new_root_name_incoming);

  // check that surfaces used in this region are a part of this CSGBase instance
  void checkRegionSurfaces(const CSGRegion & region);

  // check that cell being accessed is a part of this CSGBase instance
  bool checkCellInBase(std::shared_ptr<CSGCell> cell);

  /// List of surfaces associated with CSG object
  CSGSurfaceList _surface_list;

  /// List of surfaces associated with CSG object
  CSGCellList _cell_list;

  /// List of universes associated with CSG object
  CSGUniverseList _universe_list;
};
} // namespace CSG
