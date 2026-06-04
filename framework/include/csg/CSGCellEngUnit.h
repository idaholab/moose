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

#include "CSGCell.h"
#include "CSGEngUnit.h"

namespace CSG
{

class CSGBase; // forward declaration

/**
 * CSGCellEngUnit is an abstract base class for "engineering units" that are cell-like.
 * Derived classes define domain-specific attributes and implement expandUnit() to produce the
 * equivalent CSGCell representation using basic CSG components.
 *
 * Derived classes must implement:
 *   - expandUnit() — populate _internal_base with the expanded cell and any supporting objects.
 *     Create surfaces via _internal_base->addSurface(...), any fill universes via
 *     _internal_base->createUniverse(...), and the cell via _internal_base->createCell(...) —
 *     the cell goes to the root automatically. Exactly one cell must be in the root.
 *     CSGBase::expandEngUnit() joins _internal_base via joinOtherBase() afterward.
 *   - clone() — return a deep copy of the derived class.
 *   - getAttributes() — return a map of domain-specific attribute name to value.
 */
class CSGCellEngUnit : public CSGCell, public CSGEngUnit
{
public:
  /// Satisfy CSGEngUnit::getName() — resolved via CSGCell::getName()
  const std::string & getName() const override { return CSGCell::getName(); }

  /// Disambiguate == and != by forwarding to CSGEngUnit (name/property-based comparison)
  bool operator==(const CSGCellEngUnit & other) const
  {
    return CSGEngUnit::operator==(other);
  }
  bool operator!=(const CSGCellEngUnit & other) const
  {
    return CSGEngUnit::operator!=(other);
  }

  /// Delegate to CSGCell (CSGTransformationHelper)
  const std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> &
  getTransformations() const override
  {
    return CSGCell::getTransformations();
  }

  /// Delegate to CSGCell (CSGTransformationHelper)
  std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>>
  getTransformationsAsStrings() const override
  {
    return CSGCell::getTransformationsAsStrings();
  }

protected:
  /**
   * @brief Constructor for derived classes.
   *
   * Initializes the base CSGCell with a placeholder empty CSGRegion.
   *
   * @param name name of the cell (will be preserved on the plain CSGCell after expansion)
   * @param unit_type class name of the concrete derived type — pass
   *   MooseUtils::prettyCppType<DerivedClass>() from the derived class constructor
   */
  CSGCellEngUnit(const std::string & name, const std::string & unit_type);

  /// Cell fill and region updates are not permitted on engineering units — the geometry
  /// is defined by the derived class and produced only via expandUnit().
  ///@{
  void resetCellFill() = delete;
  void updateCellFill(const std::string &) = delete;
  void updateCellFill(const CSGUniverse *) = delete;
  void updateCellFill(const CSGLattice *) = delete;
  void updateRegion(const CSGRegion &) = delete;
  ///@}

  /**
   * @brief Create a deep copy of this engineering unit for use in a different CSGBase instance.
   *
   * Derived classes must implement this and return a fully-constructed copy with
   * the same attributes.
   *
   * @return unique_ptr to a fresh copy of this engineering unit
   */
  virtual std::unique_ptr<CSGCellEngUnit> clone() const = 0;

  /**
   * @brief Create the CSGCell and any other necessary components in _internal_base.
   *
   * Called exclusively by CSGBase. The implementation should:
   *   1. Create any needed CSGSurfaces via _internal_base->addSurface(...)
   *   2. Create a CSGRegion from those surfaces
   *   3. Create any fill universe if necessary via _internal_base->createUniverse(...)
   *   4. Register exactly one CSGCell via _internal_base->createCell(...) — it goes to root.
   *
   * Do NOT set any pointer — CSGBase::expandEngUnit() retrieves the expanded cell from root.
   */
  void expandUnit() override = 0;

  /**
   * @brief Return the single cell in _internal_base's root, which IS the expanded cell.
   *
   * Enforces that exactly one cell is in the root (errors otherwise). Valid after
   * expandUnit() has been called. Not valid after CSGBase::expandEngUnit() has consumed
   * the internal base.
   *
   * @return const reference to the expanded CSGCell
   */
  const CSGCell & getExpandedCell() const;

  // Only CSGBase should be calling expandUnit() and getExpandedCell()
  friend class CSGBase;
};

} // namespace CSG
