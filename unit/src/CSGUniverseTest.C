//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGUniverseTest.h"

namespace CSG
{

// create cells to use for universes
CSGSphere sphere("sphere_surf", 1.0);
auto region1 = -sphere;
auto region2 = +sphere;
CSGCell c1("cell1", region1);
CSGCell c2("cell2", region2);
CSGCell c3("cell3", region1 | region2);

// Test attributes of root/empty CSGUniverse constructor
TEST_F(CSGUniverseTest, testRootUniverseConstructor)
{
  CSGUniverse root_univ("root_name", true);
  ASSERT_EQ("root_name", root_univ.getName());
  ASSERT_TRUE(root_univ.isRoot());
  ASSERT_EQ(0, root_univ.getAllCells().size());
}

// test attributes of CSGUniverse constructor with list of cells
TEST_F(CSGUniverseTest, testCellsUniverseConstructor)
{
  std::vector<CSGCell *> cells = {&c1, &c2};
  CSGUniverse cells_univ("cells_univ", cells);

  ASSERT_EQ("cells_univ", cells_univ.getName());
  ASSERT_FALSE(cells_univ.isRoot());
  ASSERT_EQ(2, cells_univ.getAllCells().size());
  ASSERT_TRUE(cells_univ.hasCell("cell1"));
  ASSERT_TRUE(cells_univ.hasCell("cell2"));
}

// CSGUniverse::getCell
TEST_F(CSGUniverseTest, testGetCell)
{
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
TEST_F(CSGUniverseTest, testAddCell)
{
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
    Moose::UnitUtils::assertThrows([&cells_univ]() { cells_univ.addCell(c1); },
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
TEST_F(CSGUniverseTest, testRemoveCell)
{
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
TEST_F(CSGUniverseTest, testRemoveAllCells)
{
  std::vector<CSGCell *> cells = {&c1, &c2, &c3};
  CSGUniverse cells_univ("cells_univ", cells);
  cells_univ.removeAllCells();
  ASSERT_EQ(0, cells_univ.getAllCells().size());
}

// CSGUniverse::setName
TEST_F(CSGUniverseTest, testSetName)
{
  CSGUniverse univ("first_name");
  univ.setName("new_name");
  ASSERT_EQ("new_name", univ.getName());
}

} // namespace CSG
