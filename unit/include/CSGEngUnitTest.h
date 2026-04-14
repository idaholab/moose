//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "CSGSphere.h"
#include "CSGBase.h"

#include "MooseObjectUnitTest.h"

/**
 * @brief fake engineering unit classes for the different kinds of units that are implemented for
 * testing purposes only.
 *
 * Characteristics of expanded units for testing purposes:
 *  - FakeSurfEngUnit:
 *    - 2 surfaces (so that cell region replacement using multiple surfaces can be tested)
 *
 *  - FakeCellEngUnit:
 *    - 1 cell
 *    - at least 1 surface is FakeSurfEngUnit
 *    - cell is filled with a universe
 *
 *  - FakeUnivEngUnit:
 *    - 2 cells where one is FakeCellEngUnit
 *    - 1 universe
 */

namespace CSG
{

class FakeSurfEngUnit : public CSGSurfaceEngUnit
{
public:
  FakeSurfEngUnit(const std::string & name)
    : CSGSurfaceEngUnit(name, MooseUtils::prettyCppType<FakeSurfEngUnit>())
  {
  }
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override
  {
    // contents don't matter for this fake class, just need to be able to test that this method is
    // callable
    return {};
  }

  Real evaluateSurfaceEquationAtPoint(const Point &) const override { return 1.0; }

protected:
  std::unique_ptr<CSGSurface> clone() const override
  {
    return std::make_unique<FakeSurfEngUnit>(*this);
  }

  void expandUnit(CSGBase & base) override
  {
    std::unique_ptr<CSGSurface> s1_ptr = std::make_unique<CSGSphere>("s1", 3.0);
    std::unique_ptr<CSGSurface> s2_ptr = std::make_unique<CSGSphere>("s2", 1.0);
    auto & s1 = base.addSurface(std::move(s1_ptr));
    auto & s2 = base.addSurface(std::move(s2_ptr));
    _expanded_region = -s1 & +s2;
  }

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(CSGEngUnitTest, testSurfUnit);
#endif
};

class FakeCellEngUnit : public CSGCellEngUnit
{
public:
  FakeCellEngUnit(const std::string & name)
    : CSGCellEngUnit(name, MooseUtils::prettyCppType<FakeCellEngUnit>())
  {
  }
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override
  {
    // contents don't matter for this fake class, just need to be able to test that this method is
    // callable
    return {};
  }

protected:
  std::unique_ptr<CSGCellEngUnit> clone() const override
  {
    return std::make_unique<FakeCellEngUnit>(getName());
  }

  void expandUnit(CSGBase & base) override
  {
    // use the surface unit in this one for nested units
    std::unique_ptr<FakeSurfEngUnit> s1_ptr = std::make_unique<FakeSurfEngUnit>("s1");
    auto & s1 = base.addEngUnit(std::move(s1_ptr));
    auto & univ = base.createUniverse("fill_univ");
    _expanded_cell = &base.createCell("real_cell", univ, -s1);
  }
#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(CSGEngUnitTest, testCellUnit);
#endif
};

class FakeUnivEngUnit : public CSGUniverseEngUnit
{
public:
  FakeUnivEngUnit(const std::string & name)
    : CSGUniverseEngUnit(name, MooseUtils::prettyCppType<FakeUnivEngUnit>())
  {
  }
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override
  {
    // contents don't matter for this fake class, just need to be able to test that this method is
    // callable
    return {};
  }

protected:
  std::unique_ptr<CSGUniverseEngUnit> clone() const override
  {
    return std::make_unique<FakeUnivEngUnit>(getName());
  }

  void expandUnit(CSGBase & base) override
  {
    std::unique_ptr<CSGSurface> s1_ptr = std::make_unique<CSGSphere>("s1", 3.0);
    auto & s1 = base.addSurface(std::move(s1_ptr));
    auto c1_prt = std::make_unique<FakeCellEngUnit>("c1_unit");
    auto & c1 = base.addEngUnit(std::move(c1_prt));
    auto & c2 = base.createCell("c2", +s1);
    std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {c1, c2};
    _expanded_universe = &base.createUniverse("real_univ", cells);
  }
#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(CSGEngUnitTest, testUnivUnit);
#endif
};
}