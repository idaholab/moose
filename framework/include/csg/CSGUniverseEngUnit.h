//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_UNIT_TEST
#include "gtest/gtest.h"
#endif

#include "CSGUniverse.h"
#include "CSGEngUnit.h"

namespace CSG
{

class CSGBase; // forward declaration

/**
 * CSGUniverseEngUnit is an abstract base class for "engineering units" that are universe-like.
 * Derived classes define domain-specific attributes and implement expandUnit() to produce the
 * equivalent CSGUniverse representation, along with all supporting surfaces and cells.
 *
 * Derived classes must implement:
 *   - expandUnit(CSGBase &) — construct the equivalent CSGUniverse and any other necessary
 *     CSG components that are required for defining the unit in basic CSG components
 *     and add them to CSGBase. The created CSGUniverse should set to _expanded_universe once
 *     generated so it can be returned by getExpandedUniverse().
 *   - getAttributes() — return a map of domain-specific attribute name to value.
 */
class CSGUniverseEngUnit : public CSGUniverse, public CSGEngUnit
{
public:
  /// Satisfy CSGEngUnit::getName() — resolved via CSGUniverse::getName()
  const std::string & getName() const override { return CSGUniverse::getName(); }

  /// Disambiguate == and != by forwarding to CSGEngUnit (name/property-based comparison)
  bool operator==(const CSGUniverseEngUnit & other) const
  {
    return CSGEngUnit::operator==(other);
  }
  bool operator!=(const CSGUniverseEngUnit & other) const
  {
    return CSGEngUnit::operator!=(other);
  }

  /// Delegate to CSGUniverse (CSGTransformationHelper)
  const std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> &
  getTransformations() const override
  {
    return CSGUniverse::getTransformations();
  }

  /// Delegate to CSGUniverse (CSGTransformationHelper)
  std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>>
  getTransformationsAsStrings() const override
  {
    return CSGUniverse::getTransformationsAsStrings();
  }

protected:
  /**
   * @brief Constructor for derived classes.
   *
   * @param name unique name of the universe (will be preserved on the plain CSGUniverse)
   * @param unit_type class name of the concrete derived type — pass
   *   MooseUtils::prettyCppType<DerivedClass>() from the derived class constructor
   * @param is_root true if this is the root universe (default false)
   */
  CSGUniverseEngUnit(const std::string & name, const std::string & unit_type, bool is_root = false);

  /**
   * @brief Create the CSGUniverse and any other necessary components that replace this engineering
   * unit in CSGBase.
   *
   * Called exclusively by CSGBase. The implementation must:
   *   1. Create any needed surfaces, cells, and universe(s) in base via the appropriate creation
   * methods.
   *   2. Create the a CSGUniverse that represents the "root" of this engineering unit and store the
   * returned reference for retrieval by getExpandedUniverse().
   *
   * @param base CSGBase to which the universe and other components will be added to
   */
  void expandUnit(CSGBase & base) override = 0;

  /**
   * @brief Return the CSGUniverse created and stored during expandUnit().
   *
   * Checks that expandUnit() has already been called before returning.
   * Throws if called before expansion.
   *
   * @return const reference to the newly expanded CSGUniverse
   */
  const CSGUniverse & getExpandedUniverse() const;

  /// Stores a pointer to the plain CSGUniverse registered during expandUnit(); nullptr until then.
  /// Derived classes must set this (e.g. _expanded_universe = &base.createUniverse(...)) in expandUnit().
  const CSGUniverse * _expanded_universe = nullptr;

  // Only CSGBase should be calling expandUnit() and getExpandedUniverse()
  friend class CSGBase;
};

} // namespace CSG
