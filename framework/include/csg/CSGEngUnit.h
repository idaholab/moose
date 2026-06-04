//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CSGTransformationHelper.h"
#include "JsonIO.h"
#include "MooseEnum.h"

namespace CSG
{

class CSGBase; // forward declaration

/**
 * CSGEngUnit is the abstract base class for all "engineering unit" types in the CSG system.
 * It captures the interface shared by CSGSurfaceEngUnit, CSGCellEngUnit, and
 * CSGUniverseEngUnit: a name, a domain-specific attribute map, and a protected expandUnit()
 * method that is called exclusively by CSGBase to convert the EngUnit into equivalent basic
 * CSG objects.
 *
 * Derived classes (CSGSurfaceEngUnit, CSGCellEngUnit, CSGUniverseEngUnit) further
 * specialise the expansion contract for their respective CSG object types and allow the
 * units to act as those basic types during CSGBase object creation.
 */
class CSGEngUnit
{
public:
  virtual ~CSGEngUnit();

  /**
   * @brief Get the unique instance name of this engineering unit.
   *
   * Satisfied by the getName() inherited from the corresponding CSG base class
   * (CSGSurface, CSGCell, or CSGUniverse) via the disambiguation override in each
   * intermediate EngUnit class.
   *
   * @return const reference to the instance name string
   */
  virtual const std::string & getName() const = 0;

  /**
   * @brief Get the behavior of this engineering unit (surface (i.e. region), cell, or universe).
   *
   * Returns "SURFACE", "CELL", or "UNIVERSE" depending on which intermediate
   * EngUnit class the concrete type derives from. Set automatically at construction.
   *
   * @return const reference to the behavior string
   */
  const std::string getBehavior() const { return _behavior; }

  /**
   * @brief Get the type name of this engineering unit.
   *
   * Returns the class name of the concrete derived type, should be set automatically at
   * construction using MooseUtils::prettyCppType<DerivedClass>().
   *
   * @return const reference to the unit type string
   */
  const std::string & getUnitType() const { return _unit_type; }

  /**
   * @brief Get the attributes that define this engineering unit.
   *
   * Returns a map of domain-specific parameter names to their values, describing
   * the EngUnit in meaningful engineering terms.
   *
   * @return map of attribute name to AttributeVariant value
   */
  virtual std::unordered_map<std::string, AttributeVariant> getAttributes() const = 0;

  /// Get the list of transformations applied to this engineering unit
  virtual const std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> &
  getTransformations() const = 0;

  /// Get the transformations with string representations for types
  virtual std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>>
  getTransformationsAsStrings() const = 0;

  /// Operator overload for checking if two CSGEngUnit objects are equal
  bool operator==(const CSGEngUnit & other) const;

  /// Operator overload for checking if two CSGEngUnit objects are not equal
  bool operator!=(const CSGEngUnit & other) const;

protected:
  /**
   * @brief Constructor for intermediate EngUnit abstract classes.
   *
   * @param behavior the basic CSG type this unit produces — must be "SURFACE", "CELL",
   *   or "UNIVERSE"; set by the intermediate class, not the concrete derived class
   * @param unit_type the class name of the concrete derived type — set by passing
   *   MooseUtils::prettyCppType<DerivedClass>() from the derived class constructor
   */
  CSGEngUnit(const std::string & behavior, const std::string & unit_type);

  /**
   * @brief Expand this engineering unit into basic CSG objects stored in _internal_base.
   *
   * Called exclusively by CSGBase via the typed expandEngUnit() overloads. The implementation
   * must create all necessary basic CSG objects by calling the appropriate methods on
   * _internal_base (e.g., _internal_base->addSurface(...), _internal_base->createCell(...)).
   * CSGBase::expandEngUnit() then joins _internal_base into the parent CSGBase after
   * this method returns.
   */
  virtual void expandUnit() = 0;

  /**
   * @brief Get the internal CSGBase owned by this engineering unit.
   *
   * @return reference to the internal CSGBase
   */
  CSGBase & getBase() { return *_internal_base; }

  /**
   * @brief Transfer ownership of the internal CSGBase out of this unit.
   *
   * Called once by CSGBase::expandEngUnit() to join the internal base into the parent.
   * After this call _internal_base is null.
   *
   * @return unique_ptr to the internal CSGBase
   */
  std::unique_ptr<CSGBase> releaseBase() { return std::move(_internal_base); }

  /// CSGBase populated by expandUnit(); joined into the parent CSGBase by expandEngUnit()
  std::unique_ptr<CSGBase> _internal_base;

  /// The basic CSG type this unit produces — one of "SURFACE", "CELL", "UNIVERSE"
  MooseEnum _behavior{"SURFACE CELL UNIVERSE"};

  /// The class name of the concrete derived type, set via MooseUtils::prettyCppType<T>()
  const std::string _unit_type;

  friend class CSGBase;
};

} // namespace CSG
