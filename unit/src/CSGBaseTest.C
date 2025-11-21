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
  const auto & lattice = csg_obj->createCartesianLattice(
      "lat1",
      1.0,
      std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>>{
          {std::cref(lat_univ1), std::cref(lat_univ1)}});

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

/// tests CSGBase::createCartesianLattice methods
TEST(CSGBaseTest, testCreateCartLattice)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & u_out = csg_obj->createUniverse("univ_outer"); // universe for lattice outer
  auto & univ1 = csg_obj->createUniverse("u1");
  auto & univ2 = csg_obj->createUniverse("u2");
  auto csg_obj2 = std::make_unique<CSG::CSGBase>(); // second base for error tests

  // create cartesian lattice w/out universes + void outer
  {
    const auto & lat = csg_obj->createCartesianLattice("ron", 1.0);
    // expect no universe map to be present yet
    auto dims_map = lat.getAttributes();
    ASSERT_EQ(lat.getNRows(), 0);
    ASSERT_EQ(lat.getNCols(), 0);
    ASSERT_EQ(lat.getUniverses().size(), 0);
    // check other attributes
    ASSERT_EQ(lat.getPitch(), 1.0);
    ASSERT_TRUE(lat.getName() == "ron");
    ASSERT_TRUE(lat.getType() == "CSG::CSGCartesianLattice");
    ASSERT_TRUE(lat.getOuterType() == "VOID");
  }

  // create cartesian lattice w/out universes + material outer
  {
    const auto & lat = csg_obj->createCartesianLattice("hermione", 1.0, "mat_outer");
    // just check outer, other attributes are set the same as previous test
    ASSERT_TRUE(lat.getOuterType() == "CSG_MATERIAL");
    ASSERT_TRUE(lat.getOuterMaterial() == "mat_outer");
  }

  // create cartesian lattice w/out universes + universe outer
  {
    const auto & lat = csg_obj->createCartesianLattice("harry", 1.0, u_out);
    // just check outer, other attributes are set the same as previous test
    ASSERT_TRUE(lat.getOuterType() == "UNIVERSE");
    ASSERT_TRUE(lat.getOuterUniverse() == u_out);
  }

  // create cartesian lattice w/ provided list of universes + void outer
  {
    // create a 2x3 lattice of universes
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {
        {univ1, univ2, univ1}, {univ2, univ1, univ2}};
    const auto & lat = csg_obj->createCartesianLattice("alvin", 1.0, univs);
    auto dims_map = lat.getAttributes();
    ASSERT_EQ(lat.getNRows(), 2);
    ASSERT_EQ(lat.getNCols(), 3);
    ASSERT_EQ(lat.getPitch(), 1.0);
    ASSERT_EQ(lat.getUniverses().size(), 2);
    ASSERT_EQ(lat.getUniverses()[0].size(), 3);
    ASSERT_EQ(lat.getUniverses()[1].size(), 3);
    // check other attributes
    ASSERT_TRUE(lat.getName() == "alvin");
    ASSERT_TRUE(lat.getType() == "CSG::CSGCartesianLattice");
    ASSERT_TRUE(lat.getOuterType() == "VOID");
  }

  // create cartesian lattice w/ provided list of universes + material outer
  {
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {
        {univ1, univ2, univ1}, {univ2, univ1, univ2}};
    const auto & lat = csg_obj->createCartesianLattice("theo", 1.0, univs, "mat_outer");
    // just check outer, other attributes are set the same as previous test
    ASSERT_TRUE(lat.getOuterType() == "CSG_MATERIAL");
    ASSERT_TRUE(lat.getOuterMaterial() == "mat_outer");
  }

  // create cartesian lattice w/ provided list of universes + universe outer
  {
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {
        {univ1, univ2, univ1}, {univ2, univ1, univ2}};
    const auto & lat = csg_obj->createCartesianLattice("simon", 1.0, univs, u_out);
    // just check outer, other attributes are set the same as previous test
    ASSERT_TRUE(lat.getOuterType() == "UNIVERSE");
    ASSERT_TRUE(lat.getOuterUniverse() == u_out);
  }

  // create lattice trying to use universes from a different CSGBase instance - raise error
  {
    // make new CSGBase and add new universes to this
    auto & other_univ = csg_obj2->createUniverse("u1"); // same name as existing universe
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{other_univ},
                                                                                 {other_univ}};
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &univs]() { csg_obj->createCartesianLattice("dumbledore", 1.0, univs); },
        "Cannot create Cartesian lattice dumbledore. Universe u1 is not in the CSGBase instance.");
  }

  // create lattice w/ outer universe from different base instance - raise error
  {
    auto & other_univ = csg_obj2->createUniverse("univ_outer"); // same name but different instance
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &other_univ]()
        { csg_obj->createCartesianLattice("voldemort", 1.0, other_univ); },
        "Outer universe univ_outer is not in the CSGBase instance.");
  }
}

/// tests CSGBase::createHexagonalLattice methods
TEST(CSGBaseTest, testCreateHexLattice)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ1 = csg_obj->createUniverse("u1");
  auto & univ2 = csg_obj->createUniverse("u2");
  auto & u_out = csg_obj->createUniverse("univ_outer"); // universe for lattice outer
  auto csg_obj2 = std::make_unique<CSG::CSGBase>();     // second base obj for error tests

  // create hexagonal lattice w/out universes + void outer
  {
    const auto & lat = csg_obj->createHexagonalLattice("dexter", 1.0);
    // expect no universe map to be present yet
    auto dims_map = lat.getAttributes();
    ASSERT_EQ(lat.getNRows(), 0);
    ASSERT_EQ(lat.getNRings(), 0);
    ASSERT_EQ(lat.getUniverses().size(), 0);
    // check other attributes
    ASSERT_EQ(lat.getPitch(), 1.0);
    ASSERT_TRUE(lat.getName() == "dexter");
    ASSERT_TRUE(lat.getType() == "CSG::CSGHexagonalLattice");
    ASSERT_TRUE(lat.getOuterType() == "VOID");
  }

  // create hexagonal lattice w/out universes + material outer
  {
    const auto & lat = csg_obj->createHexagonalLattice("deedee", 1.0, "mat_outer");
    ASSERT_TRUE(lat.getOuterType() == "CSG_MATERIAL");
    ASSERT_TRUE(lat.getOuterMaterial() == "mat_outer");
  }

  // create hexagonal lattice w/out universes + universe outer
  {
    const auto & lat = csg_obj->createHexagonalLattice("mandark", 1.0, u_out);
    ASSERT_TRUE(lat.getOuterType() == "UNIVERSE");
    ASSERT_TRUE(lat.getOuterUniverse() == u_out);
  }

  // create hexagonal lattice w/ provided list of universes + void outer
  {
    // create a 3-ring hexagonal lattice of universes
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {
        {univ1, univ1, univ1},
        {univ1, univ2, univ2, univ1},
        {univ1, univ2, univ1, univ2, univ1},
        {univ1, univ2, univ2, univ1},
        {univ1, univ1, univ1}};
    const auto & lat = csg_obj->createHexagonalLattice("hex_lat", 1.0, univs);
    auto dims_map = lat.getAttributes();
    ASSERT_EQ(lat.getNRows(), 5);
    ASSERT_EQ(lat.getNRings(), 3);
    ASSERT_EQ(lat.getPitch(), 1.0);
    ASSERT_EQ(lat.getUniverses().size(), 5);
    ASSERT_EQ(lat.getUniverses()[0].size(), 3);
    ASSERT_EQ(lat.getUniverses()[1].size(), 4);
    ASSERT_EQ(lat.getUniverses()[2].size(), 5);
    ASSERT_EQ(lat.getUniverses()[3].size(), 4);
    ASSERT_EQ(lat.getUniverses()[4].size(), 3);
    // check other attributes
    ASSERT_TRUE(lat.getName() == "hex_lat");
    ASSERT_TRUE(lat.getType() == "CSG::CSGHexagonalLattice");
    ASSERT_TRUE(lat.getOuterType() == "VOID");
  }

  // create hexagonal lattice w/ provided list of universes + material outer
  {
    // create a 3-ring hexagonal lattice of universes
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {
        {univ1, univ1, univ1},
        {univ1, univ2, univ2, univ1},
        {univ1, univ2, univ1, univ2, univ1},
        {univ1, univ2, univ2, univ1},
        {univ1, univ1, univ1}};
    const auto & lat = csg_obj->createHexagonalLattice("hex_lat1", 1.0, univs, "mat_outer");
    ASSERT_TRUE(lat.getOuterType() == "CSG_MATERIAL");
    ASSERT_TRUE(lat.getOuterMaterial() == "mat_outer");
  }

  // create hexagonal lattice w/ provided list of universes + universe outer
  {
    // create a 3-ring hexagonal lattice of universes
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {
        {univ1, univ1, univ1},
        {univ1, univ2, univ2, univ1},
        {univ1, univ2, univ1, univ2, univ1},
        {univ1, univ2, univ2, univ1},
        {univ1, univ1, univ1}};
    const auto & lat = csg_obj->createHexagonalLattice("hex_lat2", 1.0, univs, u_out);
    ASSERT_TRUE(lat.getOuterType() == "UNIVERSE");
    ASSERT_TRUE(lat.getOuterUniverse() == u_out);
  }

  // create lattice trying to use universes from a different CSGBase instance
  {
    auto & univ1 = csg_obj2->createUniverse("u1"); // same name as existing universe
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {
        {univ1, univ1}, {univ1, univ1, univ1}, {univ1, univ1}};
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &univs]() { csg_obj->createHexagonalLattice("failure", 1.0, univs); },
        "Cannot create hexagonal lattice failure. Universe u1 is not in the CSGBase instance.");
  }

  // create lattice w/ outer universe from different base instance - raise error
  {
    auto & other_univ = csg_obj2->createUniverse("univ_outer"); // same name but different instance
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &other_univ]() { csg_obj->createHexagonalLattice("lollipop", 1.0, other_univ); },
        "Outer universe univ_outer is not in the CSGBase instance.");
  }
}

/// tests the resetLatticeOuter method (setOuter methods tested inherently in the create tests)
TEST(CSGBaseTest, testLatticeOuter)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();

  {
    // reset from CSG_MATERIAL type to VOID
    const auto & lat1 = csg_obj->createCartesianLattice("lat1", 1.0, "mat_outer");
    ASSERT_TRUE(lat1.getOuterType() == "CSG_MATERIAL");
    ASSERT_TRUE(lat1.getOuterMaterial() == "mat_outer");
    csg_obj->resetLatticeOuter(lat1);
    ASSERT_TRUE(lat1.getOuterType() == "VOID");
  }
  {
    // reset from UNIVERSE type to VOID
    auto & u_out = csg_obj->createUniverse("univ_outer"); // universe for lattice outer
    const auto & lat2 = csg_obj->createCartesianLattice("lat2", 1.0, u_out);
    ASSERT_TRUE(lat2.getOuterType() == "UNIVERSE");
    ASSERT_TRUE(lat2.getOuterUniverse() == u_out);
    csg_obj->resetLatticeOuter(lat2);
    ASSERT_TRUE(lat2.getOuterType() == "VOID");
  }
}

/// tests CSGBase::addLattice - meant to simulate creating a custom lattice type
TEST(CSGBaseTest, testAddLattice)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ = csg_obj->createUniverse("uni");
  {
    // create a lattice as a unique pointer and manually add it to the CSGBase (this is what would
    // be done for a custom lattice type defined outside of the main framework)
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
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    auto & univ2 = csg_obj2->createUniverse("uni");
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
}

/// tests the CSGBase::setUniverseAtLatticeIndex method
TEST(CSGBaseTest, testSetUniverseAtLatticeIndex)
{
  auto csg_obj = std::make_unique<CSG::CSGBase>();
  auto & univ1 = csg_obj->createUniverse("spidey");
  auto & univ2 = csg_obj->createUniverse("spin");
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univs = {{univ1}, {univ1}};
  const auto & lat = csg_obj->createCartesianLattice("spiderverse", 1.0, univs);
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
  const auto & cartlat = csg_obj->createCartesianLattice("batverse", 1.0, univs);
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
    const auto & lat = csg_obj->createCartesianLattice("new_lattice", 1.0);
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
  const auto & lat = csg_obj->createCartesianLattice("original_name", 1.0);
  {
    // successful rename
    csg_obj->renameLattice(lat, "new_name");
    ASSERT_EQ("new_name", lat.getName());
  }
  {
    // try to rename to existing name
    const auto & lat2 = csg_obj->createCartesianLattice("another_lattice", 1.0);
    Moose::UnitUtils::assertThrows(
        [&csg_obj, &lat2]() { csg_obj->renameLattice(lat2, "new_name"); },
        "Lattice with name new_name already exists in geometry.");
  }
  {
    // try to rename lattice that does not exist in this base
    auto csg_obj2 = std::make_unique<CSG::CSGBase>();
    const auto & lat3 = csg_obj2->createCartesianLattice("another_lattice", 1.0);
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
  const auto & lat = csg_obj->createCartesianLattice("lattice1", 1.0);
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
    const auto & lat2 = csg_obj->createCartesianLattice("lattice2", 1.0);
    auto all_lats = csg_obj->getAllLattices();
    ASSERT_EQ(2, all_lats.size());
    ASSERT_TRUE(((all_lats[0].get() == lat) && (all_lats[1].get() == lat2)) ||
                ((all_lats[0].get() == lat2) && (all_lats[1].get() == lat)));
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
  const auto & lat = base1->createCartesianLattice("lat1", 1.0, univs);
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
  const auto & lat = base1->createCartesianLattice("lat1", 1.0, univs);
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
  const auto & lat = base1->createCartesianLattice("lat1", 1.0, univs);
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
  const auto & lat = csg_obj->createCartesianLattice("lat1", 1.0, univs);
  Moose::UnitUtils::assertThrows([&csg_obj]() { csg_obj->checkUniverseLinking(); },
                                 "Universe with name univ2 is not linked to root universe.");

  // fill a new cell with the lattice, linking it to root, confirm no warning is raised when checked
  // linking tree: ROOT_UNIVERSE -> c2 -> lat1 -> univ2
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
  const auto & lat = csg_obj->createCartesianLattice("lat1", 2.0, univs);
  csg_obj->createCell("cell_lat_fill", lat, -csg_sphere_outer);

  auto csg_obj_clone = csg_obj->clone();
  ASSERT_TRUE(*csg_obj == *csg_obj_clone);

  // Add new surface to csg_obj, csg_obj and csg_obj_clone should no longer be equal
  std::unique_ptr<CSG::CSGSurface> sphere_ptr_new =
      std::make_unique<CSG::CSGSphere>("new_surf", 6.0);
  csg_obj->addSurface(std::move(sphere_ptr_new));
  ASSERT_TRUE(*csg_obj != *csg_obj_clone);
}
}
