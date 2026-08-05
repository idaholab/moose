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

namespace CSG
{

/**
 * @brief fake, but valid, engineering unit classes for the different kinds of units that are
 *  implemented for testing purposes only.
 *
 * Characteristics of expanded units for testing purposes:
 *  TestSurfEngUnit:
 *    - 2 surfaces (so that cell region replacement using multiple surfaces can be tested)
 *
 *  TestCellEngUnit:
 *    - 1 cell
 *    - at least 1 surface is TestSurfEngUnit
 *    - cell is filled with a universe
 *
 *  TestUnivEngUnit:
 *    - 2 cells (one TestCellEngUnit, one real cell) both in root of _internal_base (= expanded
 * univ)
 *    - root is renamed to "<name>_real_univ"
 *    - 1 real surface (for the real cell's region)
 */

class TestSurfEngUnit : public CSGSurfaceEngUnit
{
public:
  TestSurfEngUnit(const std::string & name) : CSGSurfaceEngUnit(name) {}
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
  TestCellEngUnit(const std::string & name) : CSGCellEngUnit(name) {}
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
  TestUnivEngUnit(const std::string & name) : CSGUniverseEngUnit(name) {}
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
    // We expand this universe engineering unit into the cells and surfaces it represents.
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

/**
 * A cyclic engineering unit pair for testing that circular dependencies of engineering units are
 * caught in expandAllEngUnits(). Expanding TestCycleUnivEngUnit creates a TestCycleCellEngUnit, and
 * expanding TestCycleCellEngUnit creates a TestCycleUnivEngUnit.
 */
class TestCycleCellEngUnit;

class TestCycleUnivEngUnit : public CSGUniverseEngUnit
{
public:
  TestCycleUnivEngUnit(const std::string & name) : CSGUniverseEngUnit(name) {}
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override { return {}; }

protected:
  std::unique_ptr<CSGUniverseEngUnit> clone() const override
  {
    return std::make_unique<TestCycleUnivEngUnit>(getName());
  }

  void expandUnit() override
  {
    // Creates a TestCycleCellEngUnit (will trigger cycle)
    auto cell_ptr = std::make_unique<TestCycleCellEngUnit>(getName() + "_cycle_cell");
    getBase().addEngUnit(std::move(cell_ptr));
    getBase().renameRootUniverse(getName() + "_expanded");
  }
};

class TestCycleCellEngUnit : public CSGCellEngUnit
{
public:
  TestCycleCellEngUnit(const std::string & name) : CSGCellEngUnit(name) {}
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override { return {}; }

protected:
  std::unique_ptr<CSGCellEngUnit> clone() const override
  {
    return std::make_unique<TestCycleCellEngUnit>(getName());
  }

  void expandUnit() override
  {
    // Creates a TestCycleUnivEngUnit (will trigger cycle)
    auto univ_ptr = std::make_unique<TestCycleUnivEngUnit>(getName() + "_cycle_univ");
    const auto & univ_unit = getBase().addEngUnit(std::move(univ_ptr));
    auto & surf = getBase().addSurface(std::make_unique<CSGSphere>(getName() + "_surf", 1.0));
    getBase().createCell(getName() + "_expanded_cell", univ_unit, -surf);
  }
};

/**
 * The following set of engineering unit implementations have an incorrect expansion method for the
 * type.
 *    TestSurfaceBadExpansion: expandUnit() should create only surfaces, not any cells or universes
 *      (except root). This test implementation creates cell(s) and universe(s). Should raise an
 *      error when expandEngUnit() is called from CSGBase.
 *    TestCellBadExpansionMulti: expandUnit() should create exactly one cell that belongs to the
 * root universe (but additional cells can be created and nested within the tree). This test
 *      implementation creates 2 cells that both belong to root. Error should be raised when
 *      getExpandedCell() is called.
 *    TestCellBadExpansionUnlinked: any nested universes and cells should be linked back to the
 *      single expanded cell. expandUnit() creates an extra cell and universe that are not linked in
 *      any way.
 *    TestUniverseBadExpansion: expandUnit() should modify the root universe and there should only
 *      be the root universe at that level and any other universes must be nested within cells.
 *      Universe linking is checked by CSGBase to ensure this is consistent set of universes. If
 *      more than one universe exists at the same level as the root universe, then the linking check
 *      will fail. This test expandUnit() creates an extra unlinked universe to check this.
 */

class TestSurfBadExpansion : public CSGSurfaceEngUnit
{
public:
  TestSurfBadExpansion(const std::string & name) : CSGSurfaceEngUnit(name) {}
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override { return {}; }

  Real evaluateSurfaceEquationAtPoint(const Point &) const override { return 1.0; }

protected:
  std::unique_ptr<CSGSurface> clone() const override
  {
    return std::make_unique<TestSurfBadExpansion>(getName());
  }

  void expandUnit() override
  {
    std::unique_ptr<CSGSurface> s1_ptr = std::make_unique<CSGSphere>(getName() + "_s1", 3.0);
    auto & s1 = getBase().addSurface(std::move(s1_ptr));
    _expanded_region = -s1;
    // create a cell and universe (considered invalid)
    getBase().createCell("bad_cell", _expanded_region);
    getBase().createUniverse("bad_universe");
  }
};

class TestCellBadExpansionMulti : public CSGCellEngUnit
{
public:
  TestCellBadExpansionMulti(const std::string & name) : CSGCellEngUnit(name) {}
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override { return {}; }

protected:
  std::unique_ptr<CSGCellEngUnit> clone() const override
  {
    return std::make_unique<TestCellBadExpansionMulti>(getName());
  }

  void expandUnit() override
  {
    // create two cells that belong to root - invalid
    auto & s1 = getBase().addSurface(std::make_unique<CSGSphere>(getName() + "_s1", 3.0));
    getBase().createCell("bad_cell_1", -s1);
    getBase().createCell("bad_cell_2", -s1);
  }
};

class TestCellBadExpansionUnlinked : public CSGCellEngUnit
{
public:
  TestCellBadExpansionUnlinked(const std::string & name) : CSGCellEngUnit(name) {}
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override { return {}; }

protected:
  std::unique_ptr<CSGCellEngUnit> clone() const override
  {
    return std::make_unique<TestCellBadExpansionUnlinked>(getName());
  }

  void expandUnit() override
  {
    auto & s1 = getBase().addSurface(std::make_unique<CSGSphere>(getName() + "_s1", 3.0));
    getBase().createCell("exp_cell", -s1);
    // create an extra universe that is not linked to the expanded cell - invalid
    getBase().createUniverse("bad_univ");
  }
};

class TestUnivEngUnitBadExpansion : public CSGUniverseEngUnit
{
public:
  TestUnivEngUnitBadExpansion(const std::string & name) : CSGUniverseEngUnit(name) {}
  std::unordered_map<std::string, AttributeVariant> getAttributes() const override { return {}; }

protected:
  std::unique_ptr<CSGUniverseEngUnit> clone() const override
  {
    return std::make_unique<TestUnivEngUnitBadExpansion>(getName());
  }

  void expandUnit() override
  {
    // need at least one cell in root so the "no cells" check passes and we reach areUniversesLinked
    auto & s1 = getBase().addSurface(std::make_unique<CSGSphere>(getName() + "_s1", 3.0));
    getBase().createCell(getName() + "_cell", +s1);
    getBase().createUniverse("bad_univ"); // orphaned universe at same level as root (invalid)
    getBase().renameRootUniverse(getName() + "_expanded");
  }
};
}
