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

#include "CSGRegion.h"
#include "CSGSurface.h"
#include "CSGEngUnit.h"

namespace CSG
{

class CSGBase; // forward declaration

/**
 * CSGSurfaceEngUnit is an abstract base class for "engineering units" that are surface-like
 * or region-like. Derived classes define domain-specific surface attributes and implement
 * expandUnit() and expandedRegion() methods to produce the equivalent representation using
 * the basic CSGSurface components.
 *
 * Derived classes must implement:
 *   - expandUnit(CSGBase &) — construct the equivalent CSGSurface(s) that are required
 *     for defining the unit in basic CSG components and add them to CSGBase.
 *   - getExpandedRegion() — return the CSGRegion formed by the surfaces built during
 *     expandUnit().
 *   - clone() — return a deep copy of the derived class.
 *   - getAttributes() — return a map of domain-specific attribute name to value.
 */
class CSGSurfaceEngUnit : public CSGSurface, public CSGEngUnit
{
public:
  /**
   * @brief Construct a new CSGSurfaceEngUnit
   *
   * @param name unique name of surface (will be preserved on the registered surface)
   * @param unit_type class name of the concrete derived type — pass
   *   MooseUtils::prettyCppType<DerivedClass>() from the derived class constructor
   */
  CSGSurfaceEngUnit(const std::string & name, const std::string & unit_type);

  /**
   * @brief Not supported before expansion — throws an error.
   *
   * Call CSGBase::addEngUnit() and then CSGBase::expandEngUnit() to expand this unit into
   * a concrete surface first. Derived classes may override this if pre-expansion querying
   * is required.
   *
   * @return map of coefficients (unreachable before expansion)
   */
  std::unordered_map<std::string, Real> getCoeffs() const override;

  /**
   * @brief Not supported before expansion — throws an error.
   *
   * Call CSGBase::addEngUnit() and then CSGBase::expandEngUnit() to expand this unit into
   * a concrete surface first. Derived classes may override this if pre-expansion querying
   * is required.
   *
   * @param p point to evaluate
   * @return evaluation result (unreachable before expansion)
   */
  Real evaluateSurfaceEquationAtPoint(const Point & p) const override;

  /// Satisfy CSGEngUnit::getName() — resolved via CSGSurface::getName()
  const std::string & getName() const override { return CSGSurface::getName(); }

  /// Delegate to CSGSurface (CSGTransformationHelper)
  const std::vector<std::pair<TransformationType, std::tuple<Real, Real, Real>>> &
  getTransformations() const override
  {
    return CSGSurface::getTransformations();
  }

  /// Delegate to CSGSurface (CSGTransformationHelper)
  std::vector<std::pair<std::string, std::tuple<Real, Real, Real>>>
  getTransformationsAsStrings() const override
  {
    return CSGSurface::getTransformationsAsStrings();
  }

protected:
  /**
   * @brief Create and add the concrete CSGSurface in the provided CSGBase.
   *
   * Called exclusively by CSGBase. The implementation must:
   *   1. Construct the concrete surface (e.g., std::make_unique<CSGZCylinder>(...))
   *   2. Add it via base.addSurface(std::move(surf)).
   *
   * @param base CSGBase to which the surface will be added to
   */
  void expandUnit(CSGBase & base) override = 0;

  /**
   * @brief Return the CSGRegion formed by expanded CSGSurfaces. This region should
   * represent the "NEGATIVE" half-space of the original engineering unit.
   *
   * Called by CSGBase immediately after expandUnit(). The derived class must store any
   * surface pointers returned by base.addSurface() during expandUnit() and use them here
   * to build the region.
   *
   * @return CSGRegion representing the negative halfspace of the surface engineering unit
   */
  virtual CSGRegion getExpandedRegion() const = 0;

  /// clone() is pure virtual — derived classes must implement
  std::unique_ptr<CSGSurface> clone() const override = 0;

  // Only CSGBase should be calling expandUnit() and getExpansionRegion()
  friend class CSGBase;
};

} // namespace CSG
