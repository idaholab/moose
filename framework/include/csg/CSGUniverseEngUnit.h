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
 *   - expandUnit() — populate _internal_base: create cells via _internal_base->createCell(...)
 *     (they go to the root automatically), create any non-root fill universes via
 *     _internal_base->createUniverse(...), and rename the root with
 *     _internal_base->renameRootUniverse("name"). The root IS the expanded universe.
 *     CSGBase::expandEngUnit() joins _internal_base via joinOtherBase() afterward.
 *   - clone() — return a deep copy of the derived class.
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

  /// addCell updates are not permitted on engineering units — the geometry
  /// is defined by the derived class and produced only via expandUnit().
  ///@{
  void addCell(const CSGCell &) = delete;
  ///@}

  /**
   * @brief Create a deep copy of this engineering unit for use in a different CSGBase instance.
   *
   * Derived classes must implement this and return a fully-constructed copy with
   * the same attributes.
   *
   * @return unique_ptr to a fresh copy of this engineering unit
   */
  virtual std::unique_ptr<CSGUniverseEngUnit> clone() const = 0;

  /**
   * @brief Populate _internal_base with the CSGUniverse and any supporting objects.
   *
   * Called exclusively by CSGBase. The root universe of _internal_base represents this
   * engineering unit's expanded universe. The implementation should:
   *   1. Create any needed surfaces via _internal_base->addSurface(...).
   *   2. Create cells via _internal_base->createCell(...) — they go to the root automatically.
   *   3. Create any non-root fill universes via _internal_base->createUniverse(...).
   *   4. Rename the root with _internal_base->renameRootUniverse("desired_name") if needed.
   */
  void expandUnit() override = 0;

  /**
   * @brief Return the root universe of _internal_base, which IS the expanded universe.
   *
   * Valid after expandUnit() has populated _internal_base. Not valid after
   * CSGBase::expandEngUnit() has run (the internal base is consumed by the join).
   *
   * @return const reference to the root CSGUniverse of _internal_base
   */
  const CSGUniverse & getExpandedUniverse() const;

  // Only CSGBase should be calling expandUnit() and getExpandedUniverse()
  friend class CSGBase;
};

} // namespace CSG
