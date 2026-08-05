//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGSurfaceEngUnit.h"
#include "CSGCellEngUnit.h"
#include "CSGUniverseEngUnit.h"

namespace CSG
{

/**
 * CSGEngUnitList is a non-owning index of CSGEngUnit objects stored in the type lists
 * (CSGSurfaceList, CSGCellList, CSGUniverseList) of a CSGBase instance. It provides
 * lookup by name and typed queries without holding ownership of the objects.
 *
 * A vector of raw pointers is used rather than a name-keyed map so that renames require
 * no index update. The object's name is updated in-place by the owning type list, and
 * the pointer in this vector automatically reflects the new name.
 */
class CSGEngUnitList
{
protected:
  /**
   * Default constructor
   */
  CSGEngUnitList();

  /**
   * Destructor
   */
  virtual ~CSGEngUnitList() = default;

  /**
   * @brief Return whether an engineering unit with the given name exists in this list
   *
   * @param name name of the engineering unit
   * @return true if an engineering unit with the given name exists, false otherwise
   */
  bool hasEngUnit(const std::string & name) const
  {
    for (const auto * ptr : _eng_units)
      if (ptr->getName() == name)
        return true;
    return false;
  }

  /**
   * @brief Get all engineering units of all types in the list
   *
   * @return list of const references to all CSGEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGEngUnit>> getAllEngUnits() const;

  /**
   * @brief Get all surface-like engineering units
   *
   * @return list of const references to all CSGSurfaceEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGSurfaceEngUnit>> getAllSurfaceEngUnits() const;

  /**
   * @brief Get all cell-like engineering units
   *
   * @return list of const references to all CSGCellEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGCellEngUnit>> getAllCellEngUnits() const;

  /**
   * @brief Get all universe-like engineering units
   *
   * @return list of const references to all CSGUniverseEngUnit objects
   */
  std::vector<std::reference_wrapper<const CSGUniverseEngUnit>> getAllUniverseEngUnits() const;

  /**
   * @brief Register an engineering unit in this index. The object must already be owned
   * by the appropriate type list (CSGSurfaceList, CSGCellList, or CSGUniverseList).
   *
   * Throws if an engineering unit with the given name already exists.
   *
   * @param unit reference to the engineering unit to register (non-owning)
   * @return reference to the registered engineering unit
   */
  CSGEngUnit & addEngUnit(CSGEngUnit & unit);

  /**
   * @brief Get an engineering unit by name
   *
   * @param name name of the engineering unit
   * @return reference to the CSGEngUnit of the specified name
   */
  CSGEngUnit & getEngUnit(const std::string & name) const;

  /**
   * @brief Remove an engineering unit from this index by address.
   *
   * Called by CSGBase when a unit is deleted or expanded. Does not affect ownership because
   * the object is destroyed by the owning type list.
   *
   * @param unit reference to the engineering unit to remove
   */
  void removeEngUnit(const CSGEngUnit & unit);

  /// Operator overload for checking if two CSGEngUnitList objects are equal
  bool operator==(const CSGEngUnitList & other) const;

  /// Operator overload for checking if two CSGEngUnitList objects are not equal
  bool operator!=(const CSGEngUnitList & other) const;

  /// Non-owning index: raw pointers into the type lists. Rename does not require updating
  /// this vector because the pointed-to object's name is updated in-place by the type list.
  std::vector<CSGEngUnit *> _eng_units;

  // Only CSGBase should be calling the methods in CSGEngUnitList
  friend class CSGBase;
};

} // namespace CSG
