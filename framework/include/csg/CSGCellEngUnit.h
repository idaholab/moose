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
 *   - expandUnit(CSGBase &) — construct the equivalent CSGCell and any other necessary
 *     CSG components that are required for defining the unit in basic CSG components
 *     and add them to CSGBase. The created CSGCell should set to _expanded_cell once
 *     generated so it can be returned by getExpandedCell().
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
   * @brief Create the CSGCell and any other necessary components that replace this engineering
   * unit in CSGBase.
   *
   * Called exclusively by CSGBase. The implementation should:
   *   1. Create any needed CSGSurfaces in base
   *   2. Create a CSGRegion from those surfaces
   *   3. Create any other components required to define the fill, if necessary (CSGUniverse, etc.)
   *   4. Register a the CSGCell via base.createCell(...) and store the returned reference
   *      for retrieval by getExpandedCell().
   *
   * @param base CSGBase to which the cell and other components will be added to
   */
  void expandUnit(CSGBase & base) override = 0;

  /**
   * @brief Return the CSGCell created and stored during expandUnit().
   *
   * Checks that expandUnit() has already been called before returning.
   * Throws if called before expansion.
   *
   * @return const reference to the newly expanded CSGCell
   */
  const CSGCell & getExpandedCell() const;

  /// Stores a pointer to the plain CSGCell registered during expandUnit(); nullptr until then.
  /// Derived classes must set this (e.g. _expanded_cell = &base.createCell(...)) in expandUnit().
  const CSGCell * _expanded_cell = nullptr;

  // Only CSGBase should be calling expandUnit() and getExpandedCell()
  friend class CSGBase;
};

} // namespace CSG
