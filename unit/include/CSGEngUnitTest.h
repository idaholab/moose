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
 *  - TestSurfEngUnit:
 *    - 2 surfaces (so that cell region replacement using multiple surfaces can be tested)
 *
 *  - TestCellEngUnit:
 *    - 1 cell
 *    - at least 1 surface is TestSurfEngUnit
 *    - cell is filled with a universe
 *
 *  - TestUnivEngUnit:
 *    - 2 cells (one TestCellEngUnit, one real cell) both in root of _internal_base (= expanded
 * univ)
 *    - root is renamed to "<name>_real_univ"
 *    - 1 real surface (for the real cell's region)
 */

namespace CSG
{

class TestSurfEngUnit : public CSGSurfaceEngUnit
{
public:
  TestSurfEngUnit(const std::string & name)
    : CSGSurfaceEngUnit(name, MooseUtils::prettyCppType<TestSurfEngUnit>())
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
    return std::make_unique<TestSurfEngUnit>(getName());
  }

  void expandUnit() override
  {
    std::unique_ptr<CSGSurface> s1_ptr = std::make_unique<CSGSphere>(getName() + "_s1", 3.0);
    std::unique_ptr<CSGSurface> s2_ptr = std::make_unique<CSGSphere>(getName() + "_s2", 1.0);
    auto & s1 = _internal_base->addSurface(std::move(s1_ptr));
    auto & s2 = _internal_base->addSurface(std::move(s2_ptr));
    _expanded_region = -s1 & +s2;
  }

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(CSGEngUnitTest, testSurfUnit);
#endif
};

class TestCellEngUnit : public CSGCellEngUnit
{
public:
  TestCellEngUnit(const std::string & name)
    : CSGCellEngUnit(name, MooseUtils::prettyCppType<TestCellEngUnit>())
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
    return std::make_unique<TestCellEngUnit>(getName());
  }

  void expandUnit() override
  {
    // use the surface unit in this one for nested units
    std::unique_ptr<TestSurfEngUnit> s1_ptr = std::make_unique<TestSurfEngUnit>(getName() + "_s1");
    auto & s1 = _internal_base->addEngUnit(std::move(s1_ptr));
    auto & univ = _internal_base->createUniverse(getName() + "_fill_univ");
    _internal_base->createCell(getName() + "_real_cell", univ, -s1);
  }

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(CSGEngUnitTest, testCellUnit);
#endif
};

class TestUnivEngUnit : public CSGUniverseEngUnit
{
public:
  TestUnivEngUnit(const std::string & name)
    : CSGUniverseEngUnit(name, MooseUtils::prettyCppType<TestUnivEngUnit>())
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
    return std::make_unique<TestUnivEngUnit>(getName());
  }

  void expandUnit() override
  {
    // all cells go to _internal_base's root, which is the expanded universe
    auto c1_prt = std::make_unique<TestCellEngUnit>(getName() + "_c1_unit");
    _internal_base->addEngUnit(std::move(c1_prt)); // goes to root
    std::unique_ptr<CSGSurface> s1_ptr = std::make_unique<CSGSphere>(getName() + "_s1", 3.0);
    auto & s1 = _internal_base->addSurface(std::move(s1_ptr));
    _internal_base->createCell(getName() + "_c2", +s1); // goes to root
    _internal_base->renameRootUniverse(getName() + "_real_univ");
  }

#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(CSGEngUnitTest, testUnivUnit);
#endif
};
}