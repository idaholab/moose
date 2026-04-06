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
 * CSGEngUnitList is a container for storing CSGEngUnit objects in the CSGBase object.
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
    return _eng_units.find(name) != _eng_units.end();
  }

  /**
   * @brief Get the non-const map of all names to engineering units
   *
   * @return map of all names to CSGEngUnit unique_ptrs
   */
  std::unordered_map<std::string, std::unique_ptr<CSGEngUnit>> & getEngUnitListMap()
  {
    return _eng_units;
  }

  /**
   * @brief Get the const map of all names to engineering units
   *
   * @return map of all names to CSGEngUnit unique_ptrs
   */
  const std::unordered_map<std::string, std::unique_ptr<CSGEngUnit>> & getEngUnitListMap() const
  {
    return _eng_units;
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
   * @brief Add an engineering unit to the list. Ownership of the unit is transferred to the list.
   *
   * Throws if an engineering unit with the given name already exists.
   *
   * @param unit unique_ptr to the engineering unit to add
   * @return reference to the CSGEngUnit that was added
   */
  CSGEngUnit & addEngUnit(std::unique_ptr<CSGEngUnit> unit);

  /**
   * @brief Get an engineering unit by name
   *
   * @param name name of the engineering unit
   * @return reference to the CSGEngUnit of the specified name
   */
  CSGEngUnit & getEngUnit(const std::string & name) const;

  /**
   * @brief Rename the specified engineering unit.
   *
   * @param unit reference to the engineering unit to rename
   * @param name new name to assign
   */
  void renameEngUnit(const CSGEngUnit & unit, const std::string & name);

  /// Operator overload for checking if two CSGEngUnitList objects are equal
  bool operator==(const CSGEngUnitList & other) const;

  /// Operator overload for checking if two CSGEngUnitList objects are not equal
  bool operator!=(const CSGEngUnitList & other) const;

  /// Mapping of engineering unit names to their stored CSGEngUnit objects
  std::unordered_map<std::string, std::unique_ptr<CSGEngUnit>> _eng_units;

  // Only CSGBase should be calling the methods in CSGEngUnitList
  friend class CSGBase;
};

} // namespace CSG
