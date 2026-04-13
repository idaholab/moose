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
#include "CSGTransformationHelper.h"
#include "CSGEngUnitList.h"
#include "nlohmann/json.h"

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

namespace CSG
{

/**
 * Enumeration of axis types for rotations
 */
enum class RotationAxisType
{
  X = 0, // X axis
  Y = 1, // Y axis
  Z = 2  // Z axis
};

/**
 * Define a variant type that can hold references to different CSG object types
 */
typedef std::variant<std::reference_wrapper<const CSGSurface>,
                     std::reference_wrapper<const CSGCell>,
                     std::reference_wrapper<const CSGUniverse>,
                     std::reference_wrapper<const CSGRegion>,
                     std::reference_wrapper<const CSGLattice>,
                     std::reference_wrapper<const CSGEngUnit>>
    CSGObjectVariant;

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
  const CSGSurface & addSurface(std::unique_ptr<CSGSurface> surf);

  /**
   * @brief Remove a Surface object passed in by reference from the stored surface list. Any CSG
   * components connected to surface will not be recursively removed.
   *
   * @param surface reference to surface to delete
   */
  void deleteSurface(const CSGSurface & surface);

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
   * @brief Check if a surface with given name exists in CSGBase object
   *
   * @param name surface name
   * @return true if surface with given name exists in CSGBase
   */
  bool hasSurface(const std::string & name) const { return _surface_list.hasSurface(name); }

  /**
   * @brief rename the specified surface
   *
   * @param surface CSGSurface to rename
   * @param name new name
   */
  void renameSurface(const CSGSurface & surface, const std::string & name);

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
   * @brief Create a Lattice Cell object
   *
   * @param name unique cell name
   * @param fill_lattice lattice that will fill the cell
   * @param region cell region
   * @param add_to_univ (optional) universe to which this cell will be added (default is root
   * universe)
   * @return reference to cell that is created
   */
  const CSGCell & createCell(const std::string & name,
                             const CSGLattice & fill_lattice,
                             const CSGRegion & region,
                             const CSGUniverse * add_to_univ = nullptr);

  /**
   * @brief Remove a Cell object passed in by reference from the stored cell list. Any CSG
   * components connected to cell will not be recursively removed.
   *
   * @param cell reference to cell to delete
   */
  void deleteCell(const CSGCell & cell);

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
   * @brief Check if a cell with given name exists in CSGBase object
   *
   * @param name cell name
   * @return true if cell with given name exists in CSGBase
   */
  bool hasCell(const std::string & name) const { return _cell_list.hasCell(name); }

  /**
   * @brief rename the specified cell
   *
   * @param cell reference to CSGCell to rename
   * @param name new name
   */
  void renameCell(const CSGCell & cell, const std::string & name);

  /**
   * @brief change the region of the specified cell
   *
   * @param cell cell to update the region for
   * @param region new region to assign to cell
   */
  void updateCellRegion(const CSGCell & cell, const CSGRegion & region);

  /**
   * @brief reset the fill of the specified cell to void
   *
   * @param cell cell to update the fill for
   */
  void resetCellFill(const CSGCell & cell);

  /**
   * @brief change the fill of the specified cell to a material fill
   *
   * @param cell cell to update the fill for
   * @param mat_name name of material fill
   */
  void updateCellFill(const CSGCell & cell, const std::string & mat_name);

  /**
   * @brief change the fill of the specified cell to a universe fill
   *
   * @param cell cell to update the fill for
   * @param univ pointer to universe fill
   */
  void updateCellFill(const CSGCell & cell, const CSGUniverse * univ);

  /**
   * @brief change the fill of the specified cell to a lattice fill
   *
   * @param cell cell to update the fill for
   * @param lattice pointer to lattice fill
   */
  void updateCellFill(const CSGCell & cell, const CSGLattice * lattice);

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
    renameUniverse(_universe_list.getRoot(), name);
  }

  /**
   * @brief rename the specified universe
   *
   * @param universe reference to CSGUniverse to rename
   * @param name new name
   */
  void renameUniverse(const CSGUniverse & universe, const std::string & name);

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
   * @brief Remove a Universe object passed in by reference from the stored universe list. Any CSG
   * components connected to universe will not be recursively removed.
   *
   * @param univ reference to universe to delete
   */
  void deleteUniverse(const CSGUniverse & univ);

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
   * @brief Check if a universe with given name exists in CSGBase object
   *
   * @param name universe name
   * @return true if universe with given name exists in CSGBase
   */
  bool hasUniverse(const std::string & name) const { return _universe_list.hasUniverse(name); }

  /**
   * @brief add a unique lattice pointer to this base instance; universes that make the lattice
   * must already be a part of this CSGBase instance.
   *
   * @param lattice pointer to lattice to add
   *
   * @return reference to CSGLattice that was added
   */
  template <typename LatticeType = CSGLattice>
  const LatticeType & addLattice(std::unique_ptr<LatticeType> lattice)
  {
    static_assert(std::is_base_of_v<CSGLattice, LatticeType>, "Is not a CSGLattice");
    // make sure all universes are a part of this base instance
    auto universes = lattice->getUniverses();
    for (auto univ_list : universes)
      for (const CSGUniverse & univ : univ_list)
        if (!checkUniverseInBase(univ))
          mooseError("Cannot add lattice " + lattice->getName() + " of type " + lattice->getType() +
                     ". Universe " + univ.getName() + " is not in the CSGBase instance.");

    if (lattice->getOuterType() == "UNIVERSE")
    {
      const CSGUniverse & outer_univ = lattice->getOuterUniverse();
      if (!checkUniverseInBase(outer_univ))
        mooseError("Cannot add lattice " + lattice->getName() + " of type " + lattice->getType() +
                   ". Outer universe " + outer_univ.getName() + " is not in the CSGBase instance.");
    }
    auto & lat_ref = _lattice_list.addLattice(std::move(lattice));
    return dynamic_cast<LatticeType &>(lat_ref);
  }

  /**
   * @brief Remove a Lattice object passed in by reference from the stored lattice list. Any CSG
   * components connected to lattice will not be recursively removed.
   *
   * @param lattice reference to lattice to delete
   */
  void deleteLattice(const CSGLattice & lattice);

  /**
   * @brief set location in the lattice to be the provided universe
   *
   * @param lattice lattice to update
   * @param universe universe to set at the location
   * @param index index of the lattice element (int, int)
   */
  void setUniverseAtLatticeIndex(const CSGLattice & lattice,
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
   * @brief Set the outer fill for the lattice to the material name provided. This will set the
   * outer type to CSG_MATERIAL regardless of its previous outer type.
   *
   * @param lattice lattice to update
   * @param outer_name name of material to use as outer fill between lattice elements
   */
  void setLatticeOuter(const CSGLattice & lattice, const std::string & outer_name);

  /**
   * @brief Set the outer fill for the lattice to the universe provided. This will set the outer
   * type to UNIVERSE regardless of its previous outer type.
   *
   * @param lattice lattice to update
   * @param outer_univ universe to use as outer fill between lattice elements
   */
  void setLatticeOuter(const CSGLattice & lattice, const CSGUniverse & outer_univ);

  /**
   * @brief reset the outer fill for the lattice to VOID
   *
   * @param lattice lattice to update
   */
  void resetLatticeOuter(const CSGLattice & lattice);

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
   * @brief Get a lattice object of the specified type by name
   *
   * This is a templated method with a default type of CSGLattice. If a specific lattice type
   * is needed, it can be specified when calling. If the type is unknown or not specified,
   * it will default to CSGLattice to get the base class reference.
   * NOTE: if CSGLattice is used as the template type, any lattice type-specific attributes or
   * methods may not be accessible, except using a reference cast.
   *
   * @param name lattice name
   * @return reference to CSGLattice object
   */
  template <typename LatticeType = CSGLattice>
  const LatticeType & getLatticeByName(const std::string & name)
  {
    const CSGLattice & lattice = _lattice_list.getLattice(name);
    const LatticeType * typed_lattice = dynamic_cast<const LatticeType *>(&lattice);
    if (!typed_lattice)
      mooseError("Cannot get lattice " + name + ". Lattice is not of specified type " +
                 MooseUtils::prettyCppType<LatticeType>());
    return *typed_lattice;
  }

  /**
   * @brief Check if a lattice with given name exists in CSGBase object
   *
   * @param name lattice name
   * @return true if lattice with given name exists in CSGBase
   */
  bool hasLattice(const std::string & name) const { return _lattice_list.hasLattice(name); }

  /**
   * @brief Add an engineering unit (surface-, cell-, or universe-like object) to this CSGBase.
   *
   * @tparam T a type derived from CSGSurfaceEngUnit, CSGCellEngUnit, or CSGUniverseEngUnit
   * @param unit unique_ptr to the engineering unit to register
   * @return const reference to the stored engineering unit
   */
  template <typename T>
  const T & addEngUnit(std::unique_ptr<T> unit)
  {
    static_assert(std::is_base_of_v<CSGEngUnit, T>, "T must derive from CSGEngUnit");
    static_assert(std::is_base_of_v<CSGSurface, T> || std::is_base_of_v<CSGCell, T> ||
                      std::is_base_of_v<CSGUniverse, T>,
                  "T must also derive from CSGSurface, CSGCell, or CSGUniverse");
    T * raw = unit.get();
    // Transfer ownership to the appropriate type list
    if constexpr (std::is_base_of_v<CSGSurface, T>)
      _surface_list.addSurface(std::move(unit));
    else if constexpr (std::is_base_of_v<CSGCell, T>)
      _cell_list.addCell(std::move(unit));
    else if constexpr (std::is_base_of_v<CSGUniverse, T>)
      _universe_list.addUniverse(std::move(unit));
    // Register non-owning pointer in the CSGEngUnitList
    _eng_unit_list.addEngUnit(static_cast<CSGEngUnit &>(*raw));
    return *raw;
  }

  /**
   * @brief Remove an engineering unit passed in by reference from the stored engineering unit list
   *
   * @param unit reference to the engineering unit to delete
   */
  void deleteEngUnit(const CSGEngUnit & unit);

  /**
   * @brief Get a engineering unit object of the specified type by name
   *
   * This is a templated method with a default type of CSGEngUnit. If a specific unit type
   * is needed, it can be specified when calling. If the type is unknown or not specified,
   * it will default to CSGEngUnit to get the base class reference.
   * NOTE: if CSGEngUnit is used as the template type, any type-specific attributes or
   * methods may not be accessible, except using a reference cast.
   *
   * @param name engineering unit name
   * @return const reference to the engineering unit
   */
  template <typename EngUnitType = CSGEngUnit>
  const EngUnitType & getEngUnitByName(const std::string & name) const
  {
    const CSGEngUnit & unit = _eng_unit_list.getEngUnit(name);
    const EngUnitType * typed_unit = dynamic_cast<const EngUnitType *>(&unit);
    if (!typed_unit)
      mooseError("Cannot get engineering unit " + name +
                 ". Engineering unit is not of specified type " +
                 MooseUtils::prettyCppType<EngUnitType>());
    return *typed_unit;
  }

  /**
   * @brief Check if an engineeering unit with given name exists in CSGBase object
   *
   * @param name engineering unit name
   * @return Check if an engineering unit with given name exists in CSGBase object
   */
  bool hasEngUnit(const std::string & name) const { return _eng_unit_list.hasEngUnit(name); }

  /**
   * @brief Rename the specified engineering unit.
   *
   * @param unit reference to engineering unit to rename
   * @param name new name
   */
  void renameEngUnit(const CSGEngUnit & unit, const std::string & name);

  /**
   * @brief Get all engineering units of all types in CSGBase
   *
   * @return list of const references to all CSGEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGEngUnit>> getAllEngUnits() const
  {
    return _eng_unit_list.getAllEngUnits();
  }

  /**
   * @brief Get all surface-like engineering units in CSGBase
   *
   * @return list of const references to all CSGSurfaceEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGSurfaceEngUnit>> getAllSurfaceEngUnits() const
  {
    return _eng_unit_list.getAllSurfaceEngUnits();
  }

  /**
   * @brief Get all cell-like engineering units in CSGBase
   *
   * @return list of const references to all CSGCellEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGCellEngUnit>> getAllCellEngUnits() const
  {
    return _eng_unit_list.getAllCellEngUnits();
  }

  /**
   * @brief Get all universe-like engineering units
   *
   * @return list of const references to all CSGUniverseEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGUniverseEngUnit>> getAllUniverseEngUnits() const
  {
    return _eng_unit_list.getAllUniverseEngUnits();
  }

  /**
   * @brief Expand a surface-like engineering unit into basic CSGSurface(s) and return the
   * CSGRegion representing the EngUnit's geometry.
   *
   * Calls expandUnit() then calls getExpansionRegion() to obtain the combined CSGRegion.
   * Every occurrence of the EngUnit as a surface reference in any existing cell region is replaced
   * with the sub-region. The EngUnit is then removed from the engineering unit list and destroyed.
   *
   * @param unit the CSGSurfaceEngUnit to expand
   * @return CSGRegion formed by the expanded CSGSurface(s)
   */
  CSGRegion expandEngUnit(const CSGSurfaceEngUnit & unit);

  /**
   * @brief Expand a cell-like engineering unit into a basic CSGCell.
   *
   * Calls expandUnit() on the EngUnit to create a basic CSGCell, then replaces every reference to
   * the EngUnit in every universe with the new expanded CSGCell. The EngUnit is then removed from
   * the engineering unit list and destroyed.
   *
   * @param unit the CSGCellEngUnit to expand into a CSGCell
   * @return const reference to the expanded CSGCell
   */
  const CSGCell & expandEngUnit(const CSGCellEngUnit & unit);

  /**
   * @brief Expand a universe-like engineering unit into a basic CSGUniverse.
   *
   * Calls expandUnit() on the EngUnit to create the equivalent CSGUniverse and any necessary
   * additional CSG components. All references to the original EngUnit in all cell fills, lattice
   * universe maps/outers, and the root universe pointer are replaced with new CSGUniverse. The
   * EngUnit is then removed from the engineering unit list and destroyed.
   *
   * @param unit the CSGUniverseEngUnit to expand into a CSGUniverse
   * @return const reference to the expanded CSGUniverse
   */
  const CSGUniverse & expandEngUnit(const CSGUniverseEngUnit & unit);

  /**
   * @brief Expand all registered engineering units into basic CSG objects.
   *
   * Each expanded EngUnit is removed from the engineering unit list and replaced by the
   * corresponding plain CSG object(s) in the standard lists. After this call, the
   * engineering unit list is empty.
   */
  void expandAllEngUnits();

  /**
   * @brief Join another CSGBase object to this one. The cells of the root universe
   * of the incoming CSGBase will be added to the existing root universe of this
   * CSGBase.
   *
   * @param base pointer to a different CSGBase object
   * @param ignore_identical_components if true, will skip adding identical components (surfaces,
   *                                    cells, universes, lattices) to the CSGBase object
   */
  void joinOtherBase(std::unique_ptr<CSGBase> base, const bool ignore_identical_components);

  /**
   * @brief Join another CSGBase object to this one. For the incoming CSGBase object,
   * the root universe is added to this CSGBase object as a new non-root universe with
   * the specified new name.
   * Note: this newly created universe will not be connected to the root universe
   * of this CSGBase object by default.
   *
   * @param base pointer to a different CSGBase object
   * @param ignore_identical_components if true, will skip adding identical components (surfaces,
   *                                    cells, universes, lattices) to the CSGBase object
   * @param new_root_name_join new name for the universe generated from the incoming root universe
   */
  void joinOtherBase(std::unique_ptr<CSGBase> base,
                     const bool ignore_identical_components,
                     const std::string & new_root_name_join);

  /**
   * @brief Join another CSGBase object to this one. The root universe for the incoming CSGBase
   * object is added to this CSGBase object as a non-root universe with a new name. The root
   * universe of this CSGBase object will be renamed and designated as non-root.
   * Note: upon completion of this join method, the root universe of this CSGBase
   * object will be empty. Neither of the new non-root universes will be connected to the
   * new root universe by default.
   *
   * @param base pointer to a different CSGBase object
   * @param ignore_identical_components if true, will skip adding identical components (surfaces,
   *                                    cells, universes, lattices) to the CSGBase object
   * @param new_root_name_base new name for universe generated from this root universe
   * @param new_root_name_join new name for the universe generated from the incoming root universe
   */
  void joinOtherBase(std::unique_ptr<CSGBase> base,
                     const bool ignore_identical_components,
                     const std::string & new_root_name_base,
                     const std::string & new_root_name_join);

  /**
   * @brief generate the JSON representation output for the CSG object
   *
   * @return nlohmann::json JSON object representing the CSG model
   */
  nlohmann::json generateOutput() const;

  /// Operator overload for checking if two CSGBase objects are equal
  bool operator==(const CSGBase & other) const;

  /// Operator overload for checking if two CSGBase objects are not equal
  bool operator!=(const CSGBase & other) const;

  /**
   * @brief Apply a transformation to a CSG object
   *
   * @param csg_object The CSG object to transform (Surface, Cell, Universe, Region, or Lattice)
   * @param type The type of transformation to apply (TRANSLATION, ROTATION, SCALE)
   * @param values tuple of transformation values (3 values for any transformation type)
   */
  void addTransformation(const CSGObjectVariant & csg_object,
                         TransformationType type,
                         const std::tuple<Real, Real, Real> & values);

  /**
   * @brief Apply a transformation to a CSGEngUnit object
   *
   * @param unit The CSGEngUnit object to transform (CSGEngUnit, CSGSurfaceEngUnit, CSGCellEngUnit,
   * CSGUniverseEngUnit, or any derived type)
   * @param type The type of transformation to apply (TRANSLATION, ROTATION, SCALE)
   * @param values tuple of transformation values (3 values for any transformation type)
   */
  void addTransformation(const CSGEngUnit & unit,
                         TransformationType type,
                         const std::tuple<Real, Real, Real> & values)
  {
    addTransformation(CSGObjectVariant{std::cref(unit)}, type, values);
  }

  /**
   * @brief Apply a translation to a CSG object in the specified x, y, and z directions.
   *
   * @param csg_object The CSG object to translate (Surface, Cell, Universe, Region, or Lattice)
   * @param distances size 3 tuple with translation distances in x, y, and z directions {x, y, z}
   */
  void applyTranslation(const CSGObjectVariant & csg_object,
                        const std::tuple<Real, Real, Real> & distances)
  {
    addTransformation(csg_object, TransformationType::TRANSLATION, distances);
  }

  /**
   * @brief Apply a translation to a CSGEngUnit object in the specified x, y, and z directions.
   *
   * @param unit The CSGEngUnit object to translate (CSGEngUnit, CSGSurfaceEngUnit, CSGCellEngUnit,
   * CSGUniverseEngUnit, or any derived type)
   * @param distances size 3 tuple with
   * translation distances in x, y, and z directions {x, y, z}
   */
  void applyTranslation(const CSGEngUnit & unit, const std::tuple<Real, Real, Real> & distances)
  {
    addTransformation(unit, TransformationType::TRANSLATION, distances);
  }

  /**
   * @brief Apply a rotation to a CSG object using (phi, theta, psi) angle notation (in degrees).
   *
   * @param csg_object The CSG object to rotate (Surface, Cell, Universe, Region, or Lattice)
   * @param angles size 3 tuple {phi, theta, psi} with rotation angles in degrees
   */
  void applyRotation(const CSGObjectVariant & csg_object,
                     const std::tuple<Real, Real, Real> & angles)
  {
    addTransformation(csg_object, TransformationType::ROTATION, angles);
  }

  /**
   * @brief Apply a rotation to a CSG object using (phi, theta, psi) angle notation (in degrees).
   *
   * @param unit The CSGEngUnit object to rotate (CSGEngUnit, CSGSurfaceEngUnit, CSGCellEngUnit,
   * CSGUniverseEngUnit, or any derived type)
   * @param angles size 3 tuple {phi, theta, psi} with rotation angles in degrees
   */
  void applyRotation(const CSGEngUnit & unit, const std::tuple<Real, Real, Real> & angles)
  {
    addTransformation(unit, TransformationType::ROTATION, angles);
  }

  /**
   * @brief Apply a rotation to a CSG object about a specified axis (X, Y, Z).
   *
   * @param csg_object The CSG object to rotate (Surface, Cell, Universe, Region, or Lattice)
   * @param axis Axis type (X, Y, or Z) about which to rotate
   * @param angle angle in degrees to rotate about the specified axis
   */
  void
  applyAxisRotation(const CSGObjectVariant & csg_object, RotationAxisType axis, const Real angle);

  /**
   * @brief Apply a rotation to a CSGEngUnit object about a specified axis (X, Y, Z).
   *
   * @param unit The CSGEngUnit object to rotate (CSGEngUnit, CSGSurfaceEngUnit, CSGCellEngUnit,
   * CSGUniverseEngUnit, or any derived type)
   * @param axis Axis type (X, Y, or Z) about which to rotate
   * @param angle angle in degrees to rotate about the specified axis
   */
  void applyAxisRotation(const CSGEngUnit & unit, RotationAxisType axis, const Real angle)
  {
    applyAxisRotation(CSGObjectVariant{std::cref(unit)}, axis, angle);
  }

  /**
   * @brief Scale a CSG object in the specified x, y, and z directions.
   *
   * @param csg_object The CSG object to scale (Surface, Cell, Universe, Region, or Lattice)
   * @param values size 3 tuple with scaling values in x, y, and z directions {x, y, z}
   */
  void applyScaling(const CSGObjectVariant & csg_object,
                    const std::tuple<Real, Real, Real> & values)
  {
    addTransformation(csg_object, TransformationType::SCALE, values);
  }

  /**
   * @brief Scale a CSGEngUnit object in the specified x, y, and z directions.
   *
   * @param unit The CSGEngUnit object to scale (CSGEngUnit, CSGSurfaceEngUnit, CSGCellEngUnit,
   * CSGUniverseEngUnit, or any derived type)
   * @param values size 3 tuple with scaling values in x, y, and z directions {x, y, z}
   */
  void applyScaling(const CSGEngUnit & unit, const std::tuple<Real, Real, Real> & values)
  {
    addTransformation(unit, TransformationType::SCALE, values);
  }

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
   * @brief Recursive method to retrieve all universes and cells linked to current universe
   *
   * @param univ Reference to universe under consideration
   * @param linked_universe_names List of universe names linked to current universe
   * @param linked_cell_names List of cell names linked to current universe
   */
  void getLinkedUniverses(const CSGUniverse & univ,
                          std::vector<std::string> & linked_universe_names,
                          std::vector<std::string> & linked_cell_names) const;

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
   * @brief Get a const reference to the CSGLatticeList object
   *
   * @return CSGLatticeList
   */
  const CSGLatticeList & getLatticeList() const { return _lattice_list; }

  /**
   * @brief Get the CSGLatticeList object
   *
   * @return CSGLatticeList
   */
  CSGLatticeList & getLatticeList() { return _lattice_list; }

  /**
   * @brief update references of incoming CSGbase to point to those of existing CSGBase object. This
   * includes updating the surface references of cell regions, universe/lattice references of cell
   * fills, cell references of universes, and universe references of lattices
   *
   * @param incoming_base CSGBase object that contains references that should be updated
   */
  void updateIncomingCSGReferences(CSGBase & incoming_base);

  /**
   * @brief update surface references of incoming CSGBase to point to those of existing CSGBase
   * object based on CSGSurface name. This involves updating the surface references of cell regions.
   *
   * @param identical_surface_refs map of surface names to surface references that will be used for
   *                               replacement
   * @param base CSGBase object that contains references that should be updated
   */
  void replaceSurfaceRefsByName(
      std::map<std::string, std::reference_wrapper<const CSGSurface>> & identical_surface_refs,
      CSGBase & base);

  /**
   * @brief update cell references of incoming CSGbase to point to those of existing CSGBase object
   * based on CSGCell name. This involves updating the cell references of universes.
   *
   * @param identical_cell_refs map of cell names to cell references that will be used for
   *                            replacement
   * @param base CSGBase object that contains references that should be updated
   */
  void replaceCellRefsByName(
      std::map<std::string, std::reference_wrapper<const CSGCell>> & identical_cell_refs,
      CSGBase & base);

  /**
   * @brief update universe references of incoming CSGbase to point to those of existing CSGBase
   * object based on CSGUniverse name. This involves updating the universe references of cells and
   * universe references of lattices.
   *
   * @param identical_universe_refs map of universe names to universe references that will be used
   *                                for replacement
   * @param base CSGBase object that contains references that should be updated
   */
  void replaceUniverseRefsByName(
      std::map<std::string, std::reference_wrapper<const CSGUniverse>> & identical_universe_refs,
      CSGBase & base);

  /**
   * @brief update lattice references of incoming CSGbase to point to those of existing CSGBase
   * object based on CSGLattice name. This involves updating the lattice references of cells.
   *
   * @param identical_lattice_refs map of lattice names to lattice references that will be used for
   *                               replacement
   * @param base CSGBase object that contains references that should be updated
   */
  void replaceLatticeRefsByName(
      std::map<std::string, std::reference_wrapper<const CSGLattice>> & identical_lattice_refs,
      CSGBase & base);

  /** @brief Get a const reference to the CSGEngUnitList object
   *
   * @return CSGEngUnitList
   */
  const CSGEngUnitList & getEngUnitList() const { return _eng_unit_list; }

  /**
   * @brief Get a non-const reference to the CSGEngUnitList object
   *
   * @return CSGEngUnitList
   */
  CSGEngUnitList & getEngUnitList() { return _eng_unit_list; }

  /**
   * @brief join a separate CSGSurfaceList object to this one
   *
   * @param surf_list CSGSurfaceList from a separate CSGBase object
   * @param ignore_identical_surfaces if true, will skip adding identical surfaces to the CSGBase
   *                                  object
   */
  void joinSurfaceList(CSGSurfaceList & surf_list, const bool ignore_identical_surfaces);

  /**
   * @brief join a separate CSGCellList object to this one
   *
   * @param cell_list CSGCellList from a separate CSGBase object
   * @param ignore_identical_cells if true, will skip adding identical cells to the CSGBase
   *                               object
   */
  void joinCellList(CSGCellList & cell_list, const bool ignore_identical_cells);

  /**
   * @brief join a separate CSGLatticeList object to this one
   *
   * @param lattice_list CSGLatticeList from a separate CSGBase object
   * @param ignore_identical_lattices if true, will skip adding identical lattices to the CSGBase
   *                                  object
   */
  void joinLatticeList(CSGLatticeList & lattice_list, const bool ignore_identical_lattices);

  /**
   * @brief join a separate CSGUniverseList object to this one;
   * root universes from univ_list will be combined into this root universe
   *
   * @param univ_list CSGUniverseList from a separate CSGBase object
   * @param ignore_identical_universes if true, will skip adding identical universes to the CSGBase
   *                                   object
   */
  void joinUniverseList(CSGUniverseList & univ_list, const bool ignore_identical_universes);

  /**
   * @brief join a separate CSGUniverseList object to this one;
   * the incoming root universe will be moved to a new universe of the new
   * name specified.
   *
   * @param univ_list CSGUniverseList from a separate CSGBase object
   * @param ignore_identical_universes if true, will skip adding identical universes to the CSGBase
   *                                   object
   * @param new_root_name_incoming new name for the universe generated from the incoming root
   * universe
   */
  void joinUniverseList(CSGUniverseList & univ_list,
                        const bool ignore_identical_universes,
                        const std::string & new_root_name_incoming);

  /**
   * @brief join a separate CSGUniverseList object to this one;
   * both this root universe and the incoming root universe will be
   * maintained as separate universes of the specified names.
   * Note: upon completion of this join method, the root universe will be empty.
   *
   * @param univ_list CSGUniverseList from a separate CSGBase object
   * @param ignore_identical_universes if true, will skip adding identical lattices to the CSGBase
   *                                   object
   * @param new_root_name_base new name for universe generated from this root universe
   * @param new_root_name_incoming new name for the universe generated from the incoming root
   * universe
   */
  void joinUniverseList(CSGUniverseList & univ_list,
                        const bool ignore_identical_universes,
                        const std::string & new_root_name_base,
                        const std::string & new_root_name_incoming);

  /**
   * @brief join a separate CSGEngUnitList object to this one
   *
   * @param eng_unit_list CSGEngUnitList from a separate CSGBase object
   */
  void joinEngUnitList(CSGEngUnitList & eng_unit_list);

  /// Replace all cell fills and lattice elements/outers referencing old_univ with new_univ
  void replaceUniverseRefs(const CSGUniverse & old_univ, const CSGUniverse & new_univ);

  /// Replace all old_cell references with new_cell across all universes in the base
  void replaceCellRefs(const CSGCell & old_cell, const CSGCell & new_cell);

  /// Replace all region occurrences of old_surf with the tokens from sub_region across all cells in the base
  void replaceSurfaceRefsWithRegion(const CSGSurface & old_surf, const CSGRegion & sub_region);

  // check that surfaces used in this region are a part of this CSGBase instance
  void checkRegionSurfaces(const CSGRegion & region) const;

  // check that surface being accessed is a part of this CSGBase instance
  bool checkSurfaceInBase(const CSGSurface & surface) const;

  // check that cell being accessed is a part of this CSGBase instance
  bool checkCellInBase(const CSGCell & cell) const;

  // check that engineering unit being accessed is a part of this CSGBase instance
  bool checkEngUnitInBase(const CSGEngUnit & unit) const;

  // check that universe being accessed is a part of this CSGBase instance
  bool checkUniverseInBase(const CSGUniverse & universe) const;

  // check that lattice being accessed is a part of this CSGBase instance
  bool checkLatticeInBase(const CSGLattice & lattice) const;

  // error if surface is referenced in any cell region
  void prepareSurfaceDeletion(const CSGSurface & surface) const;

  // warn and remove cell from any universe that contains it
  void prepareCellDeletion(const CSGCell & cell);

  // error if universe is the root, used in any lattice, or used as a cell fill
  void prepareUniverseDeletion(const CSGUniverse & universe) const;

  // Returns the CSGSurfaceEngUnit pointer if surf is an eng unit, nullptr otherwise.
  // Usable as a boolean check or to access the eng unit directly.
  const CSGSurfaceEngUnit * isSurfaceEngUnit(const CSGSurface & surf) const
  {
    return dynamic_cast<const CSGSurfaceEngUnit *>(&surf);
  }

  // Returns the CSGCellEngUnit pointer if cell is an eng unit, nullptr otherwise.
  // Usable as a boolean check or to access the eng unit directly.
  const CSGCellEngUnit * isCellEngUnit(const CSGCell & cell) const
  {
    return dynamic_cast<const CSGCellEngUnit *>(&cell);
  }

  // Returns the CSGUniverseEngUnit pointer if univ is an eng unit, nullptr otherwise.
  // Usable as a boolean check or to access the eng unit directly.
  const CSGUniverseEngUnit * isUniverseEngUnit(const CSGUniverse & univ) const
  {
    return dynamic_cast<const CSGUniverseEngUnit *>(&univ);
  }

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

  /**
   * @brief Add a new lattice to the lattice list based on a lattice reference.
   * This method is called by the copy constructor of CSGBase
   *
   * @param lattice reference to CSGLattice that should be added to universe list
   */
  const CSGLattice & addLatticeToList(const CSGLattice & lattice);

  /// List of surfaces associated with CSG object
  CSGSurfaceList _surface_list;

  /// List of cells associated with CSG object
  CSGCellList _cell_list;

  /// List of universes associated with CSG object
  CSGUniverseList _universe_list;

  /// List of lattices associated with CSG object
  CSGLatticeList _lattice_list;

  /// Container for all associated engineering units
  CSGEngUnitList _eng_unit_list;

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
