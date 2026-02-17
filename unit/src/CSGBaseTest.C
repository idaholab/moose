//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGBase.h"
#include "CSGSphere.h"
#include "CSGXCylinder.h"
#include "CSGCartesianLattice.h"
#include "CSGHexagonalLattice.h"
#include "CSGTransformation.h"

#include "MooseUnitUtils.h"

namespace CSG
{

/**
 * Tests associated with CSGSurfaceList functionality as called through CSGBase
 */

/// tests CSG[Base/SurfaceList]::addSurface() and CSG[Base/SurfaceList]::getSurfaceByName()
TEST(CSGBaseTest, testAddGetSurface)
{
  // initialize a CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // make two surfaces that have the same name
  std::unique_ptr<CSG::CSGSphere> surf_ptr1 = std::make_unique<CSG::CSGSphere>("surf", 1.0);
  std::unique_ptr<CSG::CSGSphere> surf_ptr2 = std::make_unique<CSG::CSGSphere>("surf", 2.0);
  // add one surface to base initially
  const auto & added_surf = csg_obj->addSurface(std::move(surf_ptr1));
  // assert surface is present after adding by successfully using getSurfaceByName
  {
    // public method, returns const
    ASSERT_TRUE(added_surf == csg_obj->getSurfaceByName("surf"));
    // private method, returns non-const
    ASSERT_TRUE(added_surf == csg_obj->getSurface("surf"));
  }
  // try to add surface that already exists of the same name, should raise error
  {
    Moose::UnitUtils::assertThrows([&csg_obj, &surf_ptr2]()
                                   { csg_obj->addSurface(std::move(surf_ptr2)); },
                                   "Surface with name surf already exists in geometry.");
  }
  // try to get surface that doesn't exist in base, should raise error
  {
    Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->getSurfaceByName("fake_name"); },
                                   "No surface by name fake_name exists in the geometry.");
  }
}

/// tests CSG[Base/SurfaceList]::getAllSurfaces
TEST(CSGBaseTest, testGetAllSurfaces)
{
  // initialize a CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // make two surfaces to add to base
  std::unique_ptr<CSG::CSGSphere> surf_ptr1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  std::unique_ptr<CSG::CSGSphere> surf_ptr2 = std::make_unique<CSG::CSGSphere>("surf2", 2.0);
  csg_obj->addSurface(std::move(surf_ptr1));
  csg_obj->addSurface(std::move(surf_ptr2));
  auto all_surfs = csg_obj->getAllSurfaces();
  ASSERT_EQ(2, all_surfs.size());
}

/// tests CSG[Base/SurfaceList]::renameSurface
TEST(CSGBaseTest, testRenameSurface)
{
  // initialize a CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // make two surfaces to add to base
  std::unique_ptr<CSG::CSGSphere> surf_ptr1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  std::unique_ptr<CSG::CSGSphere> surf_ptr2 = std::make_unique<CSG::CSGSphere>("surf2", 2.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf_ptr1));
  const auto & s2 = csg_obj->addSurface(std::move(surf_ptr2));

  // successfully rename surface
  {
    csg_obj->renameSurface(s1, "george");
    ASSERT_EQ("george", s1.getName());
  }
  // error should be raised if try to rename to a name that already exists
  {
    Moose::UnitUtils::assertThrows([&csg_obj, &s2]() { csg_obj->renameSurface(s2, "george"); },
                                   "Surface with name george already exists in geometry");
  }
  // error should be raised if trying to rename a surface that is not a part of this instance
  {
    // initialize a new CSGBase object
    auto csg_obj_new = std::make_unique<CSG::CSGBase>();
    // make new surface to add to new base
    std::unique_ptr<CSG::CSGSphere> surf_ptr3 = std::make_unique<CSG::CSGSphere>("surf3", 1.0);
    const auto & s3 = csg_obj_new->addSurface(std::move(surf_ptr3));
    // try to rename s3 via original base where it was not added
    Moose::UnitUtils::assertThrows([&csg_obj, &s3]() { csg_obj->renameSurface(s3, "ringo"); },
                                   "cannot be renamed to ringo as it does not exist");
  }
}

/// tests CSGBase::checkRegionSurfaces
TEST(CSGBaseTest, testCheckRegionSurfaces)
{
  // make two sets of surfaces that are identical but different base ownership
  // create a region from surfaces in base 1 and make sure that base 2 recognizes the surfaces as
  // not available in that base even though names exist
  auto csg_obj1 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf", 1.0);
  const auto & s1 = csg_obj1->addSurface(std::move(surf1));

  auto csg_obj2 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf2 = std::make_unique<CSG::CSGSphere>("surf", 1.0);
  csg_obj2->addSurface(std::move(surf2));

  auto reg1 = +s1; // uses surfaces from base 1

  // expect error when surfaces are checked in base2
  Moose::UnitUtils::assertThrows([&csg_obj2, &reg1]() { csg_obj2->checkRegionSurfaces(reg1); },
                                 "Region is being set with a surface named surf that is different "
                                 "from the surface of the same name in the CSGBase instance.");
}

/**
 * Tests associated with CSGCellList or CSGCell functionality as called through CSGBase
 */

/// tests CSG[Base/CellList]::createCell
TEST(CSGBaseTest, testCreateCell)
{
  // create each type of cell, each w/ or w/out add_to_univ specified to test universe ownership

  // initialize a CSGBase object
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // surfaces for regions for cell
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  auto reg1 = +s1;

  // make a new universe to which the new cells can be added at time of creation
  auto & add_to_univ = csg_obj->createUniverse("add_univ");

  // root universe to check in tests
  auto & root_univ = csg_obj->getRootUniverse();

  // create lattice to be used as fill
  auto & lat_univ1 = csg_obj->createUniverse("latt_univ1");
  std::unique_ptr<CSG::CSGCartesianLattice> lat_ptr = std::make_unique<CSG::CSGCartesianLattice>(
      "lat1",
      1.0,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>>{
          {std::cref(lat_univ1), std::cref(lat_univ1)}});
  const auto & lattice = csg_obj->addLattice<CSG::CSGCartesianLattice>(std::move(lat_ptr));
  // make void cells and check universe ownership
  {
    // create cell to be auto added to root universe
    std::string cname1 = "void_cell1";
    // create a void cell with name cname1 and defined by region reg1
    csg_obj->createCell(cname1, reg1);
    // create a cell and add to different universe, not root
    std::string cname2 = "void_cell2";
    csg_obj->createCell(cname2, reg1, &add_to_univ);

    // cname1 should exist in root but not the other universe
    ASSERT_TRUE(root_univ.hasCell(cname1));
    ASSERT_FALSE(add_to_univ.hasCell(cname1));

    // cname2 should exist in add_to_univ but not root
    ASSERT_TRUE(add_to_univ.hasCell(cname2));
    ASSERT_FALSE(root_univ.hasCell(cname2));
  }
  // make material cells and check universe ownership
  {
    // create cell to be auto added to root universe
    std::string cname1 = "mat_cell1";
    // create a material-filled cell with name cname1, a fill with material matname,
    // and defined by region reg1
    csg_obj->createCell(cname1, "matname", reg1);
    // create a cell and add to different universe, not root
    std::string cname2 = "mat_cell2";
    csg_obj->createCell(cname2, "matname", reg1, &add_to_univ);

    // cname1 should exist in root but not the other universe
    ASSERT_TRUE(root_univ.hasCell(cname1));
    ASSERT_FALSE(add_to_univ.hasCell(cname1));

    // cname2 should exist in add_to_univ but not root
    ASSERT_TRUE(add_to_univ.hasCell(cname2));
    ASSERT_FALSE(root_univ.hasCell(cname2));
  }
  // make universe cells and check universe ownership
  {
    auto new_univ = csg_obj->createUniverse("new_univ");

    // create cell to be auto added to root universe
    std::string cname1 = "univ_cell1";
    // create a universe-filled cell with name cname1, a fill of universe new_univ,
    // and defined by region reg1
    csg_obj->createCell(cname1, new_univ, reg1);
    // create a cell and add to different universe, not root
    std::string cname2 = "univ_cell2";
    csg_obj->createCell(cname2, new_univ, reg1, &add_to_univ);

    // cname1 should exist in root but not the other universe
    ASSERT_TRUE(root_univ.hasCell(cname1));
    ASSERT_FALSE(add_to_univ.hasCell(cname1));

    // cname2 should exist in add_to_univ but not root
    ASSERT_TRUE(add_to_univ.hasCell(cname2));
    ASSERT_FALSE(root_univ.hasCell(cname2));
  }
  // expected error: create a universe cell and add it to the same universe
  {
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &add_to_univ, &reg1]()
        { csg_obj->createCell("c", add_to_univ, reg1, &add_to_univ); },
        "cannot be filled with the same universe to which it is being added");
  }
  // make lattice cells and check universe ownership
  {
    // create cell to be auto added to root universe
    std::string cname1 = "latt_cell1";
    // create a lattice-filled cell with name cname1, a fill of lattice,
    // and defined by region reg1
    csg_obj->createCell(cname1, lattice, reg1);
    // create a cell and add to different universe, not root
    std::string cname2 = "latt_cell2";
    csg_obj->createCell(cname2, lattice, reg1, &add_to_univ);

    // cname1 should exist in root but not the other universe
    ASSERT_TRUE(root_univ.hasCell(cname1));
    ASSERT_FALSE(add_to_univ.hasCell(cname1));

    // cname2 should exist in add_to_univ but not root
    ASSERT_TRUE(add_to_univ.hasCell(cname2));
    ASSERT_FALSE(root_univ.hasCell(cname2));
  }
  // expected error: create a lattice cell and add it to a universe that exists in the lattice
  // itself
  {
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &lattice, &lat_univ1, &reg1]()
        { csg_obj->createCell("c", lattice, reg1, &lat_univ1); },
        "cannot be filled with a lattice containing the same universe to which it is being added");
  }
  // expect error: create a cell with existing name
  {
    Moose::UnitUtils::assertThrows([&csg_obj, &reg1]() { csg_obj->createCell("void_cell1", reg1); },
                                   "Cell with name void_cell1 already exists");
  }
}

/// tests CSG[Base/CellList]::getAllCells
TEST(CSGBaseTest, testGetAllCells)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  csg_obj->createCell("c1", +s1);
  csg_obj->createCell("c2", -s1);
  // expect the 2 cells to be present
  auto all_cells = csg_obj->getAllCells();
  ASSERT_EQ(2, all_cells.size());
}

/// tests CSGBase::getCellByName / CSGCellList::getCell
TEST(CSGBaseTest, testGetCellByName)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  auto c1 = csg_obj->createCell("c1", +s1);

  // get cell that exists
  {
    auto c1_get = csg_obj->getCellByName("c1");
    ASSERT_EQ(c1, c1_get);
  }
  // try to get cell that doesn't exist in base, should raise error
  {
    Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->getCellByName("fake_name"); },
                                   "No cell by name fake_name exists in the geometry.");
  }
}

/// tests CSG[Base/CellList]::renameCell
TEST(CSGBaseTest, testRenameCell)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  auto & c1 = csg_obj->createCell("c1", +s1);

  // rename success
  {
    csg_obj->renameCell(c1, "paul");
    ASSERT_EQ("paul", c1.getName());
  }
  // rename cell to existing name
  {
    // make a second cell
    auto & c2 = csg_obj->createCell("c2", -s1);
    Moose::UnitUtils::assertThrows([&csg_obj, &c2]() { csg_obj->renameCell(c2, "paul"); },
                                   "Cell with name paul already exists");
  }
  // rename cell that does not exist in this base
  {
    // make an identical cell in a different base
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    std::unique_ptr<CSG::CSGSphere> surf2 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
    const auto & s2 = csg_obj2->addSurface(std::move(surf2));
    auto & c2 = csg_obj2->createCell("c1", +s2);

    // try to rename from the first base
    Moose::UnitUtils::assertThrows([&csg_obj, &c2]() { csg_obj->renameCell(c2, "john"); },
                                   "cannot be renamed to john as it does not exist");
  }
}

/// tests CSGBase::updateCellRegion
TEST(CSGBaseTest, testUpdateCellRegion)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  auto & c1 = csg_obj->createCell("c1", +s1);

  // successfully update cell region to new region
  {
    csg_obj->updateCellRegion(c1, -s1);
    ASSERT_EQ(-s1, c1.getRegion());
  }
  // try to update cell not in this base
  {
    // make an identical cell in a different base
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    std::unique_ptr<CSG::CSGSphere> surf2 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
    const auto & s2 = csg_obj2->addSurface(std::move(surf2));
    auto & c2 = csg_obj2->createCell("c1", +s2);
    Moose::UnitUtils::assertThrows([&csg_obj, &c2, &s1]() { csg_obj->updateCellRegion(c2, -s1); },
                                   "is being updated that is different from the cell of the same "
                                   "name in the CSGBase instance.");
  }
}

/**
 * Tests associated with CSGUniverseList and CSGUniverse functionality as called through CSGBase
 */

/// tests CSGBase::createUniverse
TEST(CSGBaseTest, testCreateUniverse)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // create empty universe
  {
    auto & univ = csg_obj->createUniverse("thelma");
    ASSERT_NO_THROW(csg_obj->getUniverseByName("thelma")); // no throw confirms existence
    ASSERT_EQ(0, univ.getAllCells().size());               // confirms empty
  }
  // create universe from cells
  {
    std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
    const auto & s1 = csg_obj->addSurface(std::move(surf1));
    auto & c1 = csg_obj->createCell("c1", +s1);
    auto & c2 = csg_obj->createCell("c2", -s1);
    // create a list of cells to be added to the universe
    std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {c1, c2};
    auto & univ = csg_obj->createUniverse("louise", cells);
    ASSERT_NO_THROW(csg_obj->getUniverseByName("louise")); // no throw confirms existence
    ASSERT_EQ(2, univ.getAllCells().size());               // confirms has cells
  }
  // create universe for name that already exists
  {
    Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->createUniverse("louise"); },
                                   "Universe with name louise already exists in geometry.");
  }
}

/// tests CSG[Base/UniverseList]::renameUniverse and CSGBase::renameRootUniverse
TEST(CSGBaseTest, renameUniverse)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & root = csg_obj->getRootUniverse();
  std::string new_name_1 = "simon";
  std::string new_name_2 = "alvin";
  std::string new_name_3 = "theo";

  // rename root through root-specific function
  {
    csg_obj->renameRootUniverse(new_name_1);
    ASSERT_EQ(new_name_1, root.getName());
  }
  // rename root by passing to method explicitly
  {
    csg_obj->renameUniverse(root, new_name_2);
    ASSERT_EQ(new_name_2, root.getName());
  }
  // rename a different universe to name that already exists, should raise error
  {
    auto & univ = csg_obj->createUniverse("new_univ");
    Moose::UnitUtils::assertThrows([&csg_obj, &univ, &new_name_2]()
                                   { csg_obj->renameUniverse(univ, new_name_2); },
                                   "Universe with name " + new_name_2 + " already exists");
  }
  // rename a universe that doesn't exist in the current base
  {
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    auto & univ = csg_obj2->createUniverse("new_univ");
    Moose::UnitUtils::assertThrows([&csg_obj, &univ, &new_name_3]()
                                   { csg_obj->renameUniverse(univ, new_name_3); },
                                   "cannot be renamed to " + new_name_3 + " as it does not exist");
  }
}

/// tests CSGBase::addCell[s]ToUniverse
TEST(CSGBaseTest, testAddCellToUniverse)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  auto & c1 = csg_obj->createCell("c1", +s1);
  auto & c2 = csg_obj->createCell("c2", -s1);
  auto & c3 = csg_obj->createCell("c3", -s1 | +s1);

  auto & univ = csg_obj->createUniverse("univ");

  // add a list of cells to an existing universe
  {
    std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {c1, c2};
    csg_obj->addCellsToUniverse(univ, cells);
    ASSERT_EQ(2, univ.getAllCells().size());
  }
  // add individual cell
  {
    csg_obj->addCellToUniverse(univ, c3);
    ASSERT_EQ(3, univ.getAllCells().size());
  }
  // add cell that is not in current base but has the same name and attributes, should raise error
  {
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    std::unique_ptr<CSG::CSGSphere> surf2 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
    const auto & s2 = csg_obj2->addSurface(std::move(surf2));
    auto & c4 = csg_obj2->createCell("c1", +s2);
    Moose::UnitUtils::assertThrows([&csg_obj, &univ, &c4]()
                                   { csg_obj->addCellToUniverse(univ, c4); },
                                   "is being added to universe univ that is different from the "
                                   "cell of the same name in the CSGBase instance.");
  }
  // add cell that is in the base a universe that is not in the base, should raise error
  {
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    auto & univ_new = csg_obj2->createUniverse("univ");

    Moose::UnitUtils::assertThrows(
        [&csg_obj, &univ_new, &c1]() { csg_obj->addCellToUniverse(univ_new, c1); },
        "Cells are being added to a universe named univ that is different "
        "from the universe of the same name in the CSGBase instance.");
  }
}

/// tests CSGBase::removeCell[s]FromUniverse
TEST(CSGBaseTest, testRemoveCellFromUniverse)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  auto & c1 = csg_obj->createCell("c1", +s1);
  auto & c2 = csg_obj->createCell("c2", -s1);
  auto & c3 = csg_obj->createCell("c3", -s1 | +s1);
  std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {c1, c2, c3};
  auto & univ = csg_obj->createUniverse("univ", cells);

  // remove inidividual cell
  {
    csg_obj->removeCellFromUniverse(univ, c1);
    ASSERT_EQ(2, univ.getAllCells().size());
  }
  // remove list of cells
  {
    std::vector<std::reference_wrapper<const CSG::CSGCell>> cells_remove = {c2, c3};
    csg_obj->removeCellsFromUniverse(univ, cells_remove);
    ASSERT_EQ(0, univ.getAllCells().size());
  }
  // remove cell that is not in current base but has the same name and attributes, should raise
  // error
  {
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    std::unique_ptr<CSG::CSGSphere> surf2 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
    const auto & s2 = csg_obj2->addSurface(std::move(surf2));
    auto & c4 = csg_obj2->createCell("c1", +s2);
    Moose::UnitUtils::assertThrows([&csg_obj, &univ, &c4]()
                                   { csg_obj->removeCellFromUniverse(univ, c4); },
                                   "is being removed from universe univ that is different from the "
                                   "cell of the same name in the CSGBase instance.");
  }
  // remove cell that is in the base a universe that is not in the base, should raise error
  {
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    auto & univ_new = csg_obj2->createUniverse("univ");

    Moose::UnitUtils::assertThrows(
        [&csg_obj, &univ_new, &c1]() { csg_obj->removeCellFromUniverse(univ_new, c1); },
        "Cells are being removed from a universe named univ that is different "
        "from the universe of the same name in the CSGBase instance.");
  }
}

/// tests CSGBase::get*Universe* methods
TEST(CSGBaseTest, testGetUniverse)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ = csg_obj->createUniverse("harry");

  // get root
  {
    auto & root = csg_obj->getRootUniverse();
    ASSERT_TRUE(root.isRoot());
  }
  // successful getUniverseByName call
  {
    auto & univ_get = csg_obj->getUniverseByName("harry");
    ASSERT_EQ(univ, univ_get);
  }
  // get universe for name that does not exist, expect error
  {
    Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->getUniverseByName("potter"); },
                                   "No universe by name potter exists in the geometry.");
  }
  // getAllUniverses
  {
    // two universes expected: ROOT_UNIVERSE and harry
    auto all_univs = csg_obj->getAllUniverses();
    ASSERT_EQ(2, all_univs.size());
  }
}

/**
 * Tests associated with CSGLattice or CSGLatticeList functionality through CSGBase
 */

/// tests the [re]setLatticeOuter methods
TEST(CSGBaseTest, testLatticeOuter)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGCartesianLattice> lat_ptr =
      std::make_unique<CSG::CSGCartesianLattice>("lat1", 1.0);
  const auto & lat = csg_obj->addLattice<CSG::CSGCartesianLattice>(std::move(lat_ptr));

  // initial outer should be VOID
  {
    ASSERT_TRUE(lat.getOuterType() == "VOID");
  }
  // update to CSG_MATERIAL type
  {
    csg_obj->setLatticeOuter(lat, "mat_outer");
    ASSERT_TRUE(lat.getOuterType() == "CSG_MATERIAL");
    ASSERT_TRUE(lat.getOuterMaterial() == "mat_outer");
  }
  // update to UNIVERSE type
  {
    auto & u_out = csg_obj->createUniverse("univ_outer"); // universe for lattice outer
    csg_obj->setLatticeOuter(lat, u_out);
    ASSERT_TRUE(lat.getOuterType() == "UNIVERSE");
    ASSERT_TRUE(lat.getOuterUniverse() == u_out);
  }
  // reset back to VOID
  {
    csg_obj->resetLatticeOuter(lat);
    ASSERT_TRUE(lat.getOuterType() == "VOID");
  }
  // try to set outer universe that is not in this base
  {
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    auto & u_out2 = csg_obj2->createUniverse("univ_outer");
    Moose::UnitUtils::assertThrows([&csg_obj, &lat, &u_out2]()
                                   { csg_obj->setLatticeOuter(lat, u_out2); },
                                   "Cannot set outer universe for lattice lat1. Outer universe "
                                   "univ_outer is not in the CSGBase instance.");
  }
}

/// tests CSGBase::addLattice
TEST(CSGBaseTest, testAddLattice)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ = csg_obj->createUniverse("uni");
  auto csg_obj2 = std::make_unique<CSG::CSGBase>(); // used for error checking
  auto & univ2 = csg_obj2->createUniverse("uni");   // universe of same name from different base

  {
    // create a lattice as a unique pointer and manually add it to the CSGBase
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ}};
    std::unique_ptr<CSGCartesianLattice> custom_lat =
        std::make_unique<CSGCartesianLattice>("custom_lat", 1.0, univs);
    // add to CSGBase
    const auto & lat_ref = csg_obj->addLattice(std::move(custom_lat));
    // check that it exists in the base now
    auto all_lats = csg_obj->getAllLattices();
    ASSERT_EQ(1, all_lats.size());
    ASSERT_EQ(lat_ref, all_lats[0]);
  }
  {
    // create a custom lattice containing a universe that was not in this base (raise error)
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs2 = {{univ2}};
    std::unique_ptr<CSGCartesianLattice> custom_lat2 =
        std::make_unique<CSGCartesianLattice>("custom_lat2", 1.0, univs2);
    // try to add to first CSGBase - raises error because universe is not in this base
    Moose::UnitUtils::assertThrows([&csg_obj, &custom_lat2]()
                                   { csg_obj->addLattice(std::move(custom_lat2)); },
                                   "Cannot add lattice custom_lat2 of type "
                                   "CSG::CSGCartesianLattice. Universe uni is not in the CSGBase "
                                   "instance.");
  }
  {
    // create a custom lattice with a universe outer that is not a part of this base
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ}};
    std::unique_ptr<CSGCartesianLattice> custom_lat3 =
        std::make_unique<CSGCartesianLattice>("custom_lat3", 1.0, univs);
    // set outer universe to one from different base
    custom_lat3->updateOuter(univ2);
    // try to add to first CSGBase - raises error because outer universe is not in this base
    Moose::UnitUtils::assertThrows([&csg_obj, &custom_lat3]()
                                   { csg_obj->addLattice(std::move(custom_lat3)); },
                                   "Cannot add lattice custom_lat3 of type "
                                   "CSG::CSGCartesianLattice. Outer universe uni is not in the "
                                   "CSGBase instance.");
  }
}

/// tests the CSGBase::setUniverseAtLatticeIndex method
TEST(CSGBaseTest, testSetUniverseAtLatticeIndex)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ1 = csg_obj->createUniverse("spidey");
  auto & univ2 = csg_obj->createUniverse("spin");
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ1}, {univ1}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("spiderverse", 1.0, univs);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));
  {
    // test valid add new univ
    csg_obj->setUniverseAtLatticeIndex(lat, univ2, std::make_pair<int, int>(1, 0));
    auto all_univs = lat.getUniverses();
    ASSERT_EQ(all_univs[0][0].get(), univ1);
    ASSERT_EQ(all_univs[1][0].get(), univ2);
  }
  {
    // try to add a universe that is not from this base
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    auto & univ3 = csg_obj2->createUniverse("spidey");
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &lat, &univ3]()
        { csg_obj->setUniverseAtLatticeIndex(lat, univ3, std::make_pair<int, int>(1, 0)); },
        "Cannot add universe spidey to lattice spiderverse. Universe is not in the CSGBase "
        "instance.");
  }
}

/// tests the CSGBase::setLatticeUniverses method
TEST(CSGBaseTest, testSetLatticeUniverses)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ1 = csg_obj->createUniverse("batman");
  auto & univ2 = csg_obj->createUniverse("robin");
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ1}, {univ1}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("batverse", 1.0, univs);
  const auto & cartlat = csg_obj->addLattice(std::move(lat_ptr));
  {
    // test valid set universes - overwrite old universes
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> new_univs = {{univ2},
                                                                                     {univ2}};
    csg_obj->setLatticeUniverses(cartlat, new_univs);
    auto all_univs = cartlat.getUniverses();
    ASSERT_EQ(all_univs[0][0].get(), univ2);
    ASSERT_EQ(all_univs[1][0].get(), univ2);
  }
  {
    // try to set universes with one that is not from this base
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    auto & univ3 = csg_obj2->createUniverse("batman");
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> new_univs = {{univ3},
                                                                                     {univ2}};
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &cartlat, &new_univs]() { csg_obj->setLatticeUniverses(cartlat, new_univs); },
        "Cannot set universes for lattice batverse. Universe batman is not in the CSGBase "
        "instance.");
  }
  {
    // initialize a lattice without universes and then add universes with setLatticeUniverses
    std::unique_ptr<CSGCartesianLattice> new_lat_ptr =
        std::make_unique<CSGCartesianLattice>("new_lattice", 1.0);
    const auto & lat = csg_obj->addLattice(std::move(new_lat_ptr));
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> new_univs = {{univ1},
                                                                                     {univ1}};
    csg_obj->setLatticeUniverses(lat, new_univs);
    auto all_univs = lat.getUniverses();
    ASSERT_EQ(all_univs[0][0].get(), univ1);
    ASSERT_EQ(all_univs[1][0].get(), univ1);
  }
}

/// tests CSGBase::renameLattice
TEST(CSGBaseTest, testRenameLattice)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("original_name", 1.0);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));
  {
    // successful rename
    csg_obj->renameLattice(lat, "new_name");
    ASSERT_EQ("new_name", lat.getName());
  }
  {
    // try to rename to existing name
    std::unique_ptr<CSGCartesianLattice> lat_ptr2 =
        std::make_unique<CSGCartesianLattice>("another_lattice", 1.0);
    const auto & lat2 = csg_obj->addLattice(std::move(lat_ptr2));
    Moose::UnitUtils::assertThrows([&csg_obj, &lat2]()
                                   { csg_obj->renameLattice(lat2, "new_name"); },
                                   "Lattice with name new_name already exists in geometry.");
  }
  {
    // try to rename lattice that does not exist in this base
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    std::unique_ptr<CSGCartesianLattice> lat_ptr3 =
        std::make_unique<CSGCartesianLattice>("another_lattice", 1.0);
    const auto & lat3 = csg_obj2->addLattice(std::move(lat_ptr3));
    Moose::UnitUtils::assertThrows([&csg_obj, &lat3]()
                                   { csg_obj->renameLattice(lat3, "some_name"); },
                                   "another_lattice cannot be renamed to some_name as it does not "
                                   "exist in this CSGBase instance.");
  }
}

/// tests CSGBase::getLatticeByName and CSGBase::getAllLattices
TEST(CSGBaseTest, testGetLatticeMethods)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lattice1", 1.0);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));
  {
    // get lattice by name successfully
    const auto & lat_get = csg_obj->getLatticeByName<CSGCartesianLattice>("lattice1");
    ASSERT_EQ(lat, lat_get);
    ASSERT_EQ(typeid(lat_get), typeid(CSGCartesianLattice));
  }
  {
    // get lattice by name without specifying type, assumes default CSGLattice
    const auto & lat_get = csg_obj->getLatticeByName("lattice1");
    ASSERT_EQ(lat, lat_get);
    static_assert(std::is_same<decltype(lat_get), const CSGLattice &>::value);
  }
  {
    // try to get lattice by name that does not exist
    Moose::UnitUtils::assertThrows([&csg_obj]()
                                   { csg_obj->getLatticeByName<CSGCartesianLattice>("fake_name"); },
                                   "No lattice by name fake_name exists in the geometry.");
  }
  {
    // try to get lattice by name with wrong type
    Moose::UnitUtils::assertThrows(
        [&csg_obj]() { csg_obj->getLatticeByName<CSGHexagonalLattice>("lattice1"); },
        "Cannot get lattice lattice1. Lattice is not of specified type CSG::CSGHexagonalLattice");
  }
  {
    // get all lattices
    std::unique_ptr<CSGCartesianLattice> lat_ptr2 =
        std::make_unique<CSGCartesianLattice>("lattice2", 1.0);
    const auto & lat2 = csg_obj->addLattice(std::move(lat_ptr2));
    auto all_lats = csg_obj->getAllLattices();
    ASSERT_EQ(2, all_lats.size());
    ASSERT_TRUE(((all_lats[0].get() == lat) && (all_lats[1].get() == lat2)) ||
                ((all_lats[0].get() == lat2) && (all_lats[1].get() == lat)));
  }
}

/**
 * CSGBase::applyTransformation methods
 */

/// tests the various CSGBase::apply*Rotation convenience methods
TEST(CSGBaseTest, testApplyRotation)
{
  // for each object type, test applying rotations using the various methods
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // create various objects to apply transformations to
  std::unique_ptr<CSG::CSGXCylinder> surf_ptr =
      std::make_unique<CSG::CSGXCylinder>("cyl", 0.0, 0.0, 1.0);
  const auto & surf = csg_obj->addSurface(std::move(surf_ptr));
  const auto & reg = +surf;
  const auto & cell = csg_obj->createCell("cell", reg);
  std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {cell};
  const auto & univ = csg_obj->createUniverse("univ", cells);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat", 1.0, univs);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));

  // rotation values to use for all tests
  // simple axis rotation around each axis (x, y, z)
  Real angle = 45.0;
  // euler rotation
  std::vector<Real> euler_angles = {30.0, 45.0, 60.0};

  // expected vector of rotations to be applied in this order (x, y, z, euler):
  std::vector<std::pair<TransformationType, std::vector<Real>>> expected_rotations = {
      {TransformationType::ROTATION, {0.0, angle, 0.0}},    // around x-axis
      {TransformationType::ROTATION, {90.0, angle, -90.0}}, // around y-axis
      {TransformationType::ROTATION, {angle, 0.0, 0.0}},    // around z-axis
      {TransformationType::ROTATION, euler_angles}};        // euler angless

  // apply to surface
  {
    csg_obj->applyAxisRotation(surf, "x", angle);
    csg_obj->applyAxisRotation(surf, "y", angle);
    csg_obj->applyAxisRotation(surf, "z", angle);
    csg_obj->applyRotation(surf, euler_angles);
    ASSERT_EQ(surf.getTransformations(), expected_rotations);
  }
  // apply to cell
  {
    csg_obj->applyAxisRotation(cell, "x", angle);
    csg_obj->applyAxisRotation(cell, "y", angle);
    csg_obj->applyAxisRotation(cell, "z", angle);
    csg_obj->applyRotation(cell, euler_angles);
    ASSERT_EQ(cell.getTransformations(), expected_rotations);
  }
  // apply to universe
  {
    csg_obj->applyAxisRotation(univ, "x", angle);
    csg_obj->applyAxisRotation(univ, "y", angle);
    csg_obj->applyAxisRotation(univ, "z", angle);
    csg_obj->applyRotation(univ, euler_angles);
    ASSERT_EQ(univ.getTransformations(), expected_rotations);
  }
  // apply to lattice
  {
    csg_obj->applyAxisRotation(lat, "x", angle);
    csg_obj->applyAxisRotation(lat, "y", angle);
    csg_obj->applyAxisRotation(lat, "z", angle);
    csg_obj->applyRotation(lat, euler_angles);
    ASSERT_EQ(lat.getTransformations(), expected_rotations);
  }
  // apply to region (should apply to the surface)
  {
    csg_obj->applyAxisRotation(reg, "x", angle);
    csg_obj->applyAxisRotation(reg, "y", angle);
    csg_obj->applyAxisRotation(reg, "z", angle);
    csg_obj->applyRotation(reg, euler_angles);
    // surface should have the transformations applied x2 (from the above transformations applied
    // directly to the surface and then from the region)
    auto double_rotations = expected_rotations;
    double_rotations.insert(
        double_rotations.end(), expected_rotations.begin(), expected_rotations.end());
    ASSERT_EQ(surf.getTransformations(), double_rotations);
  }

  // assert error is raised if invalid axis is provided
  {
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &surf]() { csg_obj->applyAxisRotation(surf, "Fake", 1.0); },
        "Invalid axis 'fake' provided for axis rotation. Must be 'x', 'y', or 'z'.");
  }
}

/// tests the various CSGBase::apply*Translation convenience methods
TEST(CSGBaseTest, testApplyTranslation)
{
  // for each object type, test applying rotations using the various methods
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // create various objects to apply transformations to
  std::unique_ptr<CSG::CSGXCylinder> surf_ptr =
      std::make_unique<CSG::CSGXCylinder>("cyl", 0.0, 0.0, 1.0);
  const auto & surf = csg_obj->addSurface(std::move(surf_ptr));
  const auto & reg = +surf;
  const auto & cell = csg_obj->createCell("cell", reg);
  std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {cell};
  const auto & univ = csg_obj->createUniverse("univ", cells);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat", 1.0, univs);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));

  // translation values to use for all tests
  // simple monodirectional translation in each direction (x, y, z)
  Real dist = 1.0;
  // multidirectional translation
  std::vector<Real> multi_dists = {1.0, -2.0, 3.0};

  // expected vector of translations to be applied in this order (x, y, z, dists):
  std::vector<std::pair<TransformationType, std::vector<Real>>> expected_trans = {
      {TransformationType::TRANSLATION, {1.0, 0.0, 0.0}}, // along x axis
      {TransformationType::TRANSLATION, {0.0, 1.0, 0.0}}, // along y axis
      {TransformationType::TRANSLATION, {0.0, 0.0, 1.0}}, // along z axis
      {TransformationType::TRANSLATION, multi_dists}};    // multidirectional

  // apply to surface
  {
    csg_obj->applyMonodirectionalTranslation(surf, "x", dist);
    csg_obj->applyMonodirectionalTranslation(surf, "y", dist);
    csg_obj->applyMonodirectionalTranslation(surf, "z", dist);
    csg_obj->applyTranslation(surf, multi_dists);
    ASSERT_EQ(surf.getTransformations(), expected_trans);
  }
  // apply to cell
  {
    csg_obj->applyMonodirectionalTranslation(cell, "x", dist);
    csg_obj->applyMonodirectionalTranslation(cell, "y", dist);
    csg_obj->applyMonodirectionalTranslation(cell, "z", dist);
    csg_obj->applyTranslation(cell, multi_dists);
    ASSERT_EQ(cell.getTransformations(), expected_trans);
  }
  // apply to universe
  {
    csg_obj->applyMonodirectionalTranslation(univ, "x", dist);
    csg_obj->applyMonodirectionalTranslation(univ, "y", dist);
    csg_obj->applyMonodirectionalTranslation(univ, "z", dist);
    csg_obj->applyTranslation(univ, multi_dists);
    ASSERT_EQ(univ.getTransformations(), expected_trans);
  }
  // apply to lattice
  {
    csg_obj->applyMonodirectionalTranslation(lat, "x", dist);
    csg_obj->applyMonodirectionalTranslation(lat, "y", dist);
    csg_obj->applyMonodirectionalTranslation(lat, "z", dist);
    csg_obj->applyTranslation(lat, multi_dists);
    ASSERT_EQ(lat.getTransformations(), expected_trans);
  }
  // apply to region (should apply to the surface)
  {
    csg_obj->applyMonodirectionalTranslation(reg, "x", dist);
    csg_obj->applyMonodirectionalTranslation(reg, "y", dist);
    csg_obj->applyMonodirectionalTranslation(reg, "z", dist);
    csg_obj->applyTranslation(surf, multi_dists);
    // surface should have the transformations applied x2 (from the above transformations applied
    // directly to the surface and then from the region)
    auto double_trans = expected_trans;
    double_trans.insert(double_trans.end(), expected_trans.begin(), expected_trans.end());
    ASSERT_EQ(surf.getTransformations(), double_trans);
  }

  // assert error is raised if invalid direction is provided
  {
    Moose::UnitUtils::assertThrows([&csg_obj, &surf]()
                                   { csg_obj->applyMonodirectionalTranslation(surf, "Fake", 1.0); },
                                   "Invalid direction 'fake' provided for monodirectional "
                                   "translation. Must be 'x', 'y', or 'z'.");
  }
}

/// tests the CSGBase::applyScaling method
TEST(CSGBaseTest, testApplyScaling)
{
  // for each object type, test applying rotations using the various methods
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  // create various objects to apply transformations to
  std::unique_ptr<CSG::CSGXCylinder> surf_ptr =
      std::make_unique<CSG::CSGXCylinder>("cyl", 0.0, 0.0, 1.0);
  const auto & surf = csg_obj->addSurface(std::move(surf_ptr));
  const auto & reg = +surf;
  const auto & cell = csg_obj->createCell("cell", reg);
  std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {cell};
  const auto & univ = csg_obj->createUniverse("univ", cells);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat", 1.0, univs);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));

  // scaling vector
  std::vector<Real> scales = {2.0, 1.0, 4.0};

  // expected vector of scalings to be applied (only one scaling transformation):
  std::vector<std::pair<TransformationType, std::vector<Real>>> expected_scaling = {
      {TransformationType::SCALE, scales}};

  // apply to surface
  {
    csg_obj->applyScaling(surf, scales);
    ASSERT_EQ(surf.getTransformations(), expected_scaling);
  }
  // apply to cell
  {
    csg_obj->applyScaling(cell, scales);
    ASSERT_EQ(cell.getTransformations(), expected_scaling);
  }
  // apply to universe
  {
    csg_obj->applyScaling(univ, scales);
    ASSERT_EQ(univ.getTransformations(), expected_scaling);
  }
  // apply to lattice
  {
    csg_obj->applyScaling(lat, scales);
    ASSERT_EQ(lat.getTransformations(), expected_scaling);
  }
  // apply to region (should apply to the surface)
  {
    csg_obj->applyScaling(reg, scales);
    // surface should have the scaling transformation applied twice (from the above transformations
    // applied directly to the surface and then from the region)
    auto double_scaling = expected_scaling;
    double_scaling.insert(double_scaling.end(), expected_scaling.begin(), expected_scaling.end());
    ASSERT_EQ(surf.getTransformations(), double_scaling);
  }
}

/// tests errors are properly raised in CSGBase::ApplyTransromation methods
TEST(CSGBaseTest, testApplyTransformationErrors)
{
  // for each object type, test that errors are raised if trying to apply transformations to
  // objects that are not in the CSGBase instance
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGXCylinder> surf_ptr =
      std::make_unique<CSG::CSGXCylinder>("cyl", 0.0, 0.0, 1.0);
  const auto & surf = csg_obj->addSurface(std::move(surf_ptr));
  const auto & reg = +surf;
  const auto & cell = csg_obj->createCell("cell", reg);
  std::vector<std::reference_wrapper<const CSG::CSGCell>> cells = {cell};
  const auto & univ = csg_obj->createUniverse("univ", cells);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat", 1.0, univs);
  csg_obj->addLattice(std::move(lat_ptr));

  // second set of objects in different CSGBase instance
  auto csg_obj2 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGXCylinder> surf_ptr2 =
      std::make_unique<CSG::CSGXCylinder>("cyl", 0.0, 0.0, 1.0);
  const auto & surf2 = csg_obj2->addSurface(std::move(surf_ptr2));
  const auto & reg2 = +surf2;
  const auto & cell2 = csg_obj2->createCell("cell", reg2);
  std::vector<std::reference_wrapper<const CSG::CSGCell>> cells2 = {cell2};
  const auto & univ2 = csg_obj2->createUniverse("univ", cells2);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs2 = {{univ2}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr2 =
      std::make_unique<CSGCartesianLattice>("lat", 1.0, univs2);
  const auto & lat2 = csg_obj2->addLattice(std::move(lat_ptr2));

  // try to apply transformations to each object via the first base, should raise errors
  {
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &surf2]() { csg_obj->applyAxisRotation(surf2, "x", 90); },
        "Cannot apply transformation to surface cyl that is not in this CSGBase instance.");
    Moose::UnitUtils::assertThrows([&csg_obj, &reg2]()
                                   { csg_obj->applyAxisRotation(reg2, "x", 90); },
                                   "Cannot apply transformation to region with surface cyl that is "
                                   "not in this CSGBase instance.");
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &cell2]() { csg_obj->applyAxisRotation(cell2, "x", 90); },
        "Cannot apply transformation to cell cell that is not in this CSGBase instance.");
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &univ2]() { csg_obj->applyAxisRotation(univ2, "x", 90); },
        "Cannot apply transformation to universe univ that is not in this CSGBase instance.");
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &lat2]() { csg_obj->applyAxisRotation(lat2, "x", 90); },
        "Cannot apply transformation to lattice lat that is not in this CSGBase instance.");
  }
  // try to apply an invalid value for a transformation
  {
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &surf]()
        { csg_obj->applyTransformation(surf, TransformationType::SCALE, {0.0, 0.0, 0.0}); },
        "Invalid transformation values provided for transformation type ");
  }
}

/**
 * CSGBase::joinOtherBase methods
 */

/// test CSGBase::joinOtherBase no passed name
TEST(CSGBaseTest, joinOtherBaseJoinRoot)
{
  // Case 1: Create two CSGBase objects to join together into a single root
  // CSGBase 1: only one cell containing a lattice of one universe, which lives in the ROOT_UNIVERSE
  std::unique_ptr<CSGBase> base1 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf_ptr1 = std::make_unique<CSG::CSGSphere>("s1", 1.0);
  const auto & surf1 = base1->addSurface(std::move(surf_ptr1));
  // create a lattice of one universe
  auto & univ_in_lat = base1->createUniverse("univ_in_lat");
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ_in_lat}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat1", 1.0, univs);
  const auto & lat = base1->addLattice(std::move(lat_ptr));
  // create cell containing lattice
  auto & c1 = base1->createCell("c1", lat, +surf1);
  // CSGBase 2: two total unverses (ROOT_UNIVERSE and extra_univ) with a cell in each
  std::unique_ptr<CSGBase> base2 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf_ptr2 = std::make_unique<CSG::CSGSphere>("s2", 1.0);
  const auto & surf2 = base2->addSurface(std::move(surf_ptr2));
  auto & c2 = base2->createCell("c2", +surf2);
  auto & extra_univ = base2->createUniverse("extra_univ");
  auto & c3 = base2->createCell("c3", -surf2, &extra_univ);

  // Joining: two universes will remain
  // base1 ROOT_UNIVERSE will gain all cells from base2 ROOT_UNIVERSE
  // base2 ROOT_UNIVERSE will not exist as a separate universe
  // the "extra_univ" from base2 and "univ_in_lat" from base1 will remain separate universes
  base1->joinOtherBase(std::move(base2));

  // expect 3 universes: root, extra, lattice universe
  // 3 cells: 2 owned by root, 1 owned by extra
  ASSERT_EQ(3, base1->getAllUniverses().size());
  auto & root = base1->getRootUniverse();
  ASSERT_EQ(3, base1->getAllCells().size());
  ASSERT_EQ(2, root.getAllCells().size());
  ASSERT_TRUE(root.hasCell(c1.getName()));
  ASSERT_TRUE(root.hasCell(c2.getName()));
  auto & new_extra = base1->getUniverseByName("extra_univ");
  ASSERT_EQ(1, new_extra.getAllCells().size());
  ASSERT_TRUE(new_extra.hasCell(c3.getName()));
  // expect 2 surfaces
  ASSERT_EQ(2, base1->getAllSurfaces().size());
  // expect 1 lattice
  ASSERT_EQ(1, base1->getAllLattices().size());
}

/// test CSGBase::joinOtherBase one passed name
TEST(CSGBaseTest, joinOtherBaseOneNewRoot)
{
  // Case 2: Create two CSGBase objects to join together but keep incoming root separate
  // CSGBase 1: only one cell containing a lattice of one universe, which lives in the ROOT_UNIVERSE
  std::unique_ptr<CSGBase> base1 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf_ptr1 = std::make_unique<CSG::CSGSphere>("s1", 1.0);
  const auto & surf1 = base1->addSurface(std::move(surf_ptr1));
  // create a lattice of one universe
  auto & univ_in_lat = base1->createUniverse("univ_in_lat");
  std::vector<std::vector<std::reference_wrapper<const CSG::CSGUniverse>>> univs = {{univ_in_lat}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat1", 1.0, univs);
  const auto & lat = base1->addLattice(std::move(lat_ptr));
  // create cell containing lattice
  auto & c1 = base1->createCell("c1", lat, +surf1);
  // CSGBase 2: two total unverses (ROOT_UNIVERSE and extra_univ) with a cell in each
  std::unique_ptr<CSGBase> base2 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf_ptr2 = std::make_unique<CSG::CSGSphere>("s2", 1.0);
  const auto & surf2 = base2->addSurface(std::move(surf_ptr2));
  auto & c2 = base2->createCell("c2", +surf2);
  auto & extra_univ = base2->createUniverse("extra_univ");
  auto & c3 = base2->createCell("c3", -surf2, &extra_univ);

  // Joining: 4 universes will remain
  // base1 ROOT_UNIVERSE and univ_in_lat will remain untouched
  // all cells from ROOT_UNIVERSE in base2 create new universe called "new_univ"
  // the "extra_univ" from base2 and "univ_in_lat" from base1 will remain separate universes
  std::string new_root_name = "new_univ";
  base1->joinOtherBase(std::move(base2), new_root_name);

  // expect 4 universes: root, extra, new, and lat
  // 3 cells: 1 owned by root, 1 owned by new, 1 owned by extra
  ASSERT_EQ(4, base1->getAllUniverses().size());
  auto & root = base1->getRootUniverse();
  ASSERT_EQ(3, base1->getAllCells().size());
  // root should have c1 from original root
  ASSERT_EQ(1, root.getAllCells().size());
  ASSERT_TRUE(root.hasCell(c1.getName()));
  // new_univ should have c2 from root of base 2
  auto new_univ = base1->getUniverseByName(new_root_name);
  ASSERT_EQ(1, new_univ.getAllCells().size());
  ASSERT_TRUE(new_univ.hasCell(c2.getName()));
  // original existing extra universe should still only have c3
  auto & new_extra = base1->getUniverseByName("extra_univ");
  ASSERT_EQ(1, new_extra.getAllCells().size());
  ASSERT_TRUE(new_extra.hasCell(c3.getName()));
  // expect 2 surfaces
  ASSERT_EQ(2, base1->getAllSurfaces().size());
  // expect 1 lattice
  ASSERT_EQ(1, base1->getAllLattices().size());
}

/// test CSGBase::joinOtherBase two passed names
TEST(CSGBaseTest, joinOtherBaseTwoNewRoot)
{
  // Case 3: Create two CSGBase objects to join together with each root becoming a new universe
  // CSGBase 1: only one cell containing a lattice of one universe, which lives in the ROOT_UNIVERSE
  std::unique_ptr<CSGBase> base1 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf_ptr1 = std::make_unique<CSG::CSGSphere>("s1", 1.0);
  const auto & surf1 = base1->addSurface(std::move(surf_ptr1));
  // create a lattice of one universe
  auto & univ_in_lat = base1->createUniverse("univ_in_lat");
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ_in_lat}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat1", 1.0, univs);
  const auto & lat = base1->addLattice(std::move(lat_ptr));
  // create cell containing lattice
  auto & c1 = base1->createCell("c1", lat, +surf1);
  // CSGBase 2: two total unverses (ROOT_UNIVERSE and extra_univ) with a cell in each
  std::unique_ptr<CSGBase> base2 = std::make_unique<CSG::CSGBase>();
  std::unique_ptr<CSG::CSGSphere> surf_ptr2 = std::make_unique<CSG::CSGSphere>("s2", 1.0);
  const auto & surf2 = base2->addSurface(std::move(surf_ptr2));
  auto & c2 = base2->createCell("c2", +surf2);
  auto & extra_univ = base2->createUniverse("extra_univ");
  auto & c3 = base2->createCell("c3", -surf2, &extra_univ);

  // Joining: 5 universes will remain
  // all cells from base1 ROOT_UNIVERSE will be moved to a new universe called "new_univ1"
  // all cells from base2 ROOT_UNIVERSE will be moved to a new universe called "new_univ2"
  // base1 ROOT_UNIVERSE will be empty
  // the "extra_univ" from base2 and "univ_in_lat" from base1 will remain separate universes
  std::string new_name1 = "new_univ1";
  std::string new_name2 = "new_univ2";
  base1->joinOtherBase(std::move(base2), new_name1, new_name2);

  // expect 5 universes: root, extra, lat, new1 and new2
  // 3 cells: 0 owned by root, 1 owned by new1, 1 owned by new2, 1 owned by extra
  ASSERT_EQ(5, base1->getAllUniverses().size());
  auto & root = base1->getRootUniverse();
  ASSERT_EQ(3, base1->getAllCells().size());
  // root should have 0 cells since all were moved
  ASSERT_EQ(0, root.getAllCells().size());
  // new_univ1 should have c1 from original root of base 1
  auto new_univ1 = base1->getUniverseByName(new_name1);
  ASSERT_TRUE(new_univ1.hasCell(c1.getName()));
  // new_univ2 should have c2 from original root of base 2
  auto new_univ2 = base1->getUniverseByName(new_name2);
  ASSERT_TRUE(new_univ2.hasCell(c2.getName()));
  // original existing extra universe should still only have c3
  auto & new_extra = base1->getUniverseByName("extra_univ");
  ASSERT_TRUE(new_extra.hasCell(c3.getName()));
  ASSERT_EQ(1, new_extra.getAllCells().size());
  // expect 2 surfaces
  ASSERT_EQ(2, base1->getAllSurfaces().size());
  // expect 1 lattice
  ASSERT_EQ(1, base1->getAllLattices().size());
}

/// test CSGBase::checkUniverseLinking / getLinkedUniverses
TEST(CSGBaseTest, testUniverseLinking)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ1 = csg_obj->createUniverse("univ1");

  // new universe is not inherently linked to ROOT_UNIVERSE, should raise warning when checked
  Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->checkUniverseLinking(); },
                                 "Universe with name univ1 is not linked to root universe.");

  // link the universe by adding it to a cell that is created in root
  std::unique_ptr<CSG::CSGSphere> surf1 = std::make_unique<CSG::CSGSphere>("surf1", 1.0);
  const auto & s1 = csg_obj->addSurface(std::move(surf1));
  csg_obj->createCell("c1", univ1, +s1);

  // no warning should be raised because it is a part of c1, which is a part of root
  // linking tree: ROOT_UNIVERSE -> c1 -> univ1
  ASSERT_NO_THROW(csg_obj->checkUniverseLinking());

  // create a lattice of universes that is not linked to root, should raise warning when checked
  auto & univ2 = csg_obj->createUniverse("univ2");
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ2}};
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat1", 1.0, univs);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));
  Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->checkUniverseLinking(); },
                                 "Universe with name univ2 is not linked to root universe.");

  // set the outer to a universe, universe should also not be linked
  auto & univ_out = csg_obj->createUniverse("univ_out");
  csg_obj->setLatticeOuter(lat, univ_out);
  Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->checkUniverseLinking(); },
                                 "Universe with name univ_out is not linked to root universe.");

  // fill a new cell with the lattice, linking it to root, confirm no warning is raised when checked
  // linking tree: ROOT_UNIVERSE -> c2 -> lat1 -> univ2 + univ_out
  csg_obj->createCell("c2", lat, +s1);
  ASSERT_NO_THROW(csg_obj->checkUniverseLinking());
}

/**
 * Tests associated with CSGBase::clone
 */
/// test CSGBase::clone and equality operators for CSGBase and CSG[Surface|Cell|Universe|Lattice]List
TEST(CSGBaseTest, testCSGBaseClone)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & inner_univ = csg_obj->createUniverse("univ1");
  std::unique_ptr<CSG::CSGSurface> sphere_ptr_inner =
      std::make_unique<CSG::CSGSphere>("inner_surf", 3.0);
  auto & csg_sphere_inner = csg_obj->addSurface(std::move(sphere_ptr_inner));
  csg_obj->createCell("cell_inner", "mat1", -csg_sphere_inner, &inner_univ);

  // create cell with universe fill
  std::unique_ptr<CSG::CSGSurface> sphere_ptr_outer =
      std::make_unique<CSG::CSGSphere>("outer_surf", 5.0);
  auto & csg_sphere_outer = csg_obj->addSurface(std::move(sphere_ptr_outer));
  csg_obj->createCell("cell_univ_fill", inner_univ, -csg_sphere_outer);
  csg_obj->createCell("cell_void", +csg_sphere_outer);
  // create lattice and cell with lattice fill
  auto & lat_univ = csg_obj->createUniverse("lat_univ");
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{lat_univ}};
  auto & outer_univ = csg_obj->createUniverse("outer_univ");
  std::unique_ptr<CSGCartesianLattice> lat_ptr =
      std::make_unique<CSGCartesianLattice>("lat1", 2.0, univs);
  const auto & lat = csg_obj->addLattice(std::move(lat_ptr));
  csg_obj->setLatticeOuter(lat, outer_univ);
  csg_obj->createCell("cell_lat_fill", lat, -csg_sphere_outer);

  auto csg_obj_clone = csg_obj->clone();
  ASSERT_TRUE(*csg_obj == *csg_obj_clone);

  // Add new surface to csg_obj, csg_obj and csg_obj_clone should no longer be equal
  std::unique_ptr<CSG::CSGSurface> sphere_ptr_new =
      std::make_unique<CSG::CSGSphere>("new_surf", 6.0);
  csg_obj->addSurface(std::move(sphere_ptr_new));
  ASSERT_TRUE(*csg_obj != *csg_obj_clone);

  // Add same surface to cloned csg_obj, so that csg_obj and csg_obj_clone are equal again
  sphere_ptr_new = std::make_unique<CSG::CSGSphere>("new_surf", 6.0);
  csg_obj_clone->addSurface(std::move(sphere_ptr_new));
  ASSERT_TRUE(*csg_obj == *csg_obj_clone);

  // Reset outer universe in csg_obj and test equality of csg_obj and csg_obj_clone
  csg_obj->resetLatticeOuter(lat);
  ASSERT_TRUE(*csg_obj != *csg_obj_clone);
}
}
