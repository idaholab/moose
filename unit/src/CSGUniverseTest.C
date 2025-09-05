//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGSphere.h"
#include "CSGCell.h"
#include "CSGUniverse.h"
#include "CSGRegion.h"

#include "MooseUnitUtils.h"

namespace CSG
{

// helper function create cells to use for universes
std::tuple<CSGCell, CSGCell, CSGCell>
setupCells()
{
  // sphere is static because it must remain in memory after calling to avoid errors in some tests
  static CSG::CSGSphere sphere("sphere_surf", 1.0);
  auto region1 = -sphere;
  auto region2 = +sphere;
  CSG::CSGCell c1("cell1", region1);
  CSG::CSGCell c2("cell2", region2);
  CSG::CSGCell c3("cell3", region1 | region2);
  return std::make_tuple(c1, c2, c3);
}

// Test attributes of root/empty CSGUniverse constructor
TEST(CSGUniverseTest, testRootUniverseConstructor)
{
  CSGUniverse root_univ("root_name", true);
  ASSERT_EQ("root_name", root_univ.getName());
  ASSERT_TRUE(root_univ.isRoot());
  ASSERT_EQ(0, root_univ.getAllCells().size());
}

// test attributes of CSGUniverse constructor with list of cells
TEST(CSGUniverseTest, testCellsUniverseConstructor)
{
  auto [c1, c2, c3] = setupCells();
  std::vector<CSGCell *> cells = {&c1, &c2};
  CSGUniverse cells_univ("cells_univ", cells);

  ASSERT_EQ("cells_univ", cells_univ.getName());
  ASSERT_FALSE(cells_univ.isRoot());
  ASSERT_EQ(2, cells_univ.getAllCells().size());
  ASSERT_TRUE(cells_univ.hasCell("cell1"));
  ASSERT_TRUE(cells_univ.hasCell("cell2"));
}

// CSGUniverse::getCell
TEST(CSGUniverseTest, testGetCell)
{
  auto [c1, c2, c3] = setupCells();
  std::vector<CSGCell *> cells = {&c1, &c2};
  CSGUniverse cells_univ("cells_univ", cells);

  // get cell success
  {
    ASSERT_EQ(c1, cells_univ.getCell("cell1"));
  }
  // get cell, raises error if cell doesn't exist
  {
    Moose::UnitUtils::assertThrows([&cells_univ]() { cells_univ.getCell("cell3"); },
                                   "Cell with name cell3 does not exist in universe cells_univ.");
  }
}

// CSGUniverse::addCell
TEST(CSGUniverseTest, testAddCell)
{
  // must unpack individually to be able to pass c1 into assertThrows
  auto all_cells = setupCells();
  auto c1 = std::get<0>(all_cells);
  auto c2 = std::get<1>(all_cells);
  auto c3 = std::get<2>(all_cells);

  std::vector<CSGCell *> cells = {&c1, &c2};
  CSGUniverse cells_univ("cells_univ", cells);

  // add cell, successful
  {
    cells_univ.addCell(c3);
    ASSERT_TRUE(cells_univ.hasCell("cell3"));
    ASSERT_EQ(3, cells_univ.getAllCells().size());
  }
  // try to add a cell that already exists, should raise warning and not add
  {
    Moose::UnitUtils::assertThrows([&cells_univ, &c1]() { cells_univ.addCell(c1); },
                                   "Skipping cell insertion for cell with duplicate name.");
    ASSERT_EQ(3, cells_univ.getAllCells().size());
  }
  // remove cell
  {
    cells_univ.removeCell("cell3");
    ASSERT_EQ(2, cells_univ.getAllCells().size());
    ASSERT_FALSE(cells_univ.hasCell("cell3"));
  }
  // try to remove cell that does not exist in universe
  {
    Moose::UnitUtils::assertThrows(
        [&cells_univ]() { cells_univ.removeCell("cell3"); },
        "Cannot remove cell. Cell with name cell3 does not exist in universe");
  }
}

// CSGUniverse::removeCell
TEST(CSGUniverseTest, testRemoveCell)
{
  auto [c1, c2, c3] = setupCells();
  std::vector<CSGCell *> cells = {&c1, &c2, &c3};
  CSGUniverse cells_univ("cells_univ", cells);

  // remove cell, success
  {
    cells_univ.removeCell("cell3");
    ASSERT_EQ(2, cells_univ.getAllCells().size());
    ASSERT_FALSE(cells_univ.hasCell("cell3"));
  }
  // try to remove cell that does not exist in universe (cell3 already removed), raises error
  {
    Moose::UnitUtils::assertThrows(
        [&cells_univ]() { cells_univ.removeCell("cell3"); },
        "Cannot remove cell. Cell with name cell3 does not exist in universe");
  }
}

// CSGUniverse::removeAllCells
TEST(CSGUniverseTest, testRemoveAllCells)
{
  auto [c1, c2, c3] = setupCells();
  std::vector<CSGCell *> cells = {&c1, &c2, &c3};
  CSGUniverse cells_univ("cells_univ", cells);
  cells_univ.removeAllCells();
  ASSERT_EQ(0, cells_univ.getAllCells().size());
}

// CSGUniverse::setName
TEST(CSGUniverseTest, testSetName)
{
  CSGUniverse univ("first_name");
  univ.setName("new_name");
  ASSERT_EQ("new_name", univ.getName());
}

} // namespace CSG
