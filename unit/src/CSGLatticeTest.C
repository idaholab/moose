//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGCartesianLattice.h"
#include "CSGHexagonalLattice.h"
#include "CSGUniverse.h"

#include "MooseUnitUtils.h"

namespace CSG
{

/**
 * Tests associated with CSGLattice class and derived classes
 */

/// tests valid CSGCartesianLattice construction
TEST(CSGLatticeTest, testCreateCartLatticeValid)
{
  {
    // initialize without universes: nx0=2, nx1=3, pitch=1.0
    auto cart_lattice = CSGCartesianLattice("cartlat", 2, 3, 1.0);
    // check dimensions
    ASSERT_EQ(cart_lattice.getNRows(), 2);
    ASSERT_EQ(cart_lattice.getNCols(), 3);
    ASSERT_EQ(cart_lattice.getPitch(), 1.0);
    // expect no universe map to be present yet
    ASSERT_EQ(cart_lattice.getUniverses().size(), 0);
    // check other attributes
    ASSERT_TRUE(cart_lattice.getName() == "cartlat");
    ASSERT_TRUE(cart_lattice.getType() == "CSG::CSGCartesianLattice");
  }
  {
    // initialize with an array of universes, pitch=1.0
    const auto univ1 = CSGUniverse("univ1", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)}};
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
    // check dimensions
    ASSERT_EQ(cart_lattice.getNRows(), 2);
    ASSERT_EQ(cart_lattice.getNCols(), 3);
    ASSERT_EQ(cart_lattice.getPitch(), 1.0);
    // expect 2x3 array
    ASSERT_EQ(cart_lattice.getUniverses().size(), 2);
    ASSERT_EQ(cart_lattice.getUniverses()[0].size(), 3);
    ASSERT_EQ(cart_lattice.getUniverses()[1].size(), 3);
    // check other attributes
    ASSERT_TRUE(cart_lattice.getName() == "cartlat");
    ASSERT_TRUE(cart_lattice.getType() == "CSG::CSGCartesianLattice");
  }
}

/// tests invalid CSGCartesianLattice construction
TEST(CSGLatticeTest, testCreateCartLatticeInvalid)
{
  {
    // try initialize with invalid dimensions for nx0, nx1, and pitch
    std::string exp_msg = "Lattice cartlat of type CSG::CSGCartesianLattice must have pitch and "
                          "number of elements in both dimensions greater than 0.";
    Moose::UnitUtils::assertThrows([]() { CSGCartesianLattice("cartlat", -2, 3, 1.0); },
                                   exp_msg); // invalid nx0
    Moose::UnitUtils::assertThrows([]() { CSGCartesianLattice("cartlat", 2, 0, 1.0); },
                                   exp_msg); // invalid nx1
    Moose::UnitUtils::assertThrows([]() { CSGCartesianLattice("cartlat", 2, 3, -1.0); },
                                   exp_msg); // invalid pitch
  }
  {
    // try to initialize with universe array of invalid dimensions (second row is different length)
    const auto univ1 = CSGUniverse("univ1", false);
    const auto univ2 = CSGUniverse("univ2", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ2), std::cref(univ1)},
        {std::cref(univ2), std::cref(univ1)}};
    std::string exp_msg = "Cannot set lattice cartlat with universes. Does not have valid "
                          "dimensions for lattice type CSG::CSGCartesianLattice";
    Moose::UnitUtils::assertThrows([&univ_map]() { CSGCartesianLattice("cartlat", 1.0, univ_map); },
                                   exp_msg);
  }
}

/// tests valiud CSGHexagonalLattice construction
TEST(CSGLatticeTest, testCreateHexLatticeValid)
{
  {
    // intiialize without universes: num_rings=3, pitch=1.0
    auto hex_lat = CSGHexagonalLattice("hexlat", 3, 1.0);
    // check dimensions and properties
    ASSERT_EQ(hex_lat.getNRings(), 3);
    ASSERT_EQ(hex_lat.getPitch(), 1.0);
    ASSERT_EQ(hex_lat.getNRows(), 5);            // should be 2*nring -1 (auto calculated)
    ASSERT_EQ(hex_lat.getUniverses().size(), 0); // no universe map yet
    ASSERT_TRUE(hex_lat.getName() == "hexlat");
    ASSERT_TRUE(hex_lat.getType() == "CSG::CSGHexagonalLattice");
  }
  {
    // initialize with universe map, pitch=1.0
    const auto univ1 = CSGUniverse("univ1", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)}};
    auto hex_lat = CSGHexagonalLattice("hexlat", 1.0, univ_map);
    ASSERT_EQ(hex_lat.getNRings(), 3);
    ASSERT_EQ(hex_lat.getPitch(), 1.0);
    ASSERT_EQ(hex_lat.getNRows(), 5); // should be 2*nring -1 (auto calculated)
    ASSERT_EQ(hex_lat.getUniverses().size(), 5);
    ASSERT_EQ(hex_lat.getUniverses()[0].size(), 3);
    ASSERT_EQ(hex_lat.getUniverses()[1].size(), 4);
    ASSERT_EQ(hex_lat.getUniverses()[2].size(), 5);
    ASSERT_EQ(hex_lat.getUniverses()[3].size(), 4);
    ASSERT_EQ(hex_lat.getUniverses()[4].size(), 3);
    ASSERT_TRUE(hex_lat.getName() == "hexlat");
    ASSERT_TRUE(hex_lat.getType() == "CSG::CSGHexagonalLattice");
  }
}

/// tests invalid CSGHexagonalLattice construction
TEST(CSGLatticeTest, testCreateHexLatticeInvalid)
{
  {
    // try initialize empty by providing invalid dimensions
    Moose::UnitUtils::assertThrows(
        []() { CSGHexagonalLattice("hexlat", -2, 1.0); },
        "Cannot convert number of rings -2 to number of rows in hexagonal lattice. Number of rings "
        "must be >= 1."); // invalid num_rings
    Moose::UnitUtils::assertThrows([]() { CSGHexagonalLattice("hexlat", 2, -1.0); },
                                   "Hex lattice hexlat has invalid dimensions. Must have number of "
                                   "rings > 0 and pitch > 0."); // invalid pitch
  }
  {
    // create universe map with invalid dimensions (even number of rows)
    const auto univ1 = CSGUniverse("univ1", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)}};
    std::string exp_msg = "Cannot set lattice hexlat with universes. Does not have valid "
                          "dimensions for lattice type CSG::CSGHexagonalLattice";
    Moose::UnitUtils::assertThrows([&univ_map]() { CSGHexagonalLattice("hexlat", 1.0, univ_map); },
                                   exp_msg);
  }
  {
    // create universe map with invalid dimensions (one row has wrong number of elements)
    const auto univ1 = CSGUniverse("univ1", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1),
         std::cref(univ1),
         std::cref(univ1),
         std::cref(univ1)}, // should have 5 elements
        {std::cref(univ1), std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)}};
    std::string exp_msg = "Cannot set lattice hexlat with universes. Does not have valid "
                          "dimensions for lattice type CSG::CSGHexagonalLattice";
    Moose::UnitUtils::assertThrows([&univ_map]() { CSGHexagonalLattice("hexlat", 1.0, univ_map); },
                                   exp_msg);
  }
}

TEST(CSGLatticeTest, testGetDimensions)
{
  {
    // cartesian lattice
    auto cart_lattice = CSGCartesianLattice("cartlat", 2, 3, 1.0);
    auto dims_map = cart_lattice.getDimensions();
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nx0"]), 2);
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nx1"]), 3);
    ASSERT_EQ(*std::any_cast<Real>(&dims_map["pitch"]), 1.0);
  }
  {
    // hexagonal lattice
    auto hex_lattice = CSGHexagonalLattice("hexlat", 3, 1.0);
    auto dims_map = hex_lattice.getDimensions();
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nrow"]), 5); // should be 2*nring -1
    ASSERT_EQ(*std::any_cast<Real>(&dims_map["pitch"]), 1.0);
  }
}

/// tests CSGCartesianLattice::setUniverses function
TEST(CSGLatticeTest, testCartSetUniverses)
{
  {
    // create a lattice w/out universe map and then set one w/ correct dimensions - valid
    const auto univ1 = CSGUniverse("univ1", false);
    const auto univ2 = CSGUniverse("univ2", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ2), std::cref(univ1)},
        {std::cref(univ2), std::cref(univ1), std::cref(univ2)}};
    auto cart_lattice = CSGCartesianLattice("cartlat", 2, 3, 1.0);
    // should not have any universe map initially
    ASSERT_EQ(cart_lattice.getUniverses().size(), 0);
    ASSERT_NO_THROW(cart_lattice.setUniverses(univ_map););
    // should have 2x3 map after being set
    ASSERT_EQ(cart_lattice.getUniverses().size(), 2);
    ASSERT_EQ(cart_lattice.getUniverses()[0].size(), 3);
    ASSERT_EQ(cart_lattice.getUniverses()[1].size(), 3);
  }
  {
    // create a lattice w/out universe map and then set one w/ different dimensions - invalid
    auto cart_lattice = CSGCartesianLattice("cartlat", 2, 3, 1.0);
    // create universe map with dimensions that differ from the 2x3 as specified
    const auto univ1 = CSGUniverse("univ1", false);
    const auto univ2 = CSGUniverse("univ2", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ2)}, {std::cref(univ2), std::cref(univ1)}};
    Moose::UnitUtils::assertThrows([&cart_lattice, &univ_map]()
                                   { cart_lattice.setUniverses(univ_map); },
                                   "Cannot set lattice cartlat with universes. Does not have valid "
                                   "dimensions for lattice type");
  }
  {
    // create lattice w/ universe map, overwrite w/ other valid universe map - valid
    const auto univ1 = CSGUniverse("univ1", false);
    const auto univ2 = CSGUniverse("univ2", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)}};
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
    // initial map should contain structure matching univ_map (all univ1)
    for (auto univ_list : cart_lattice.getUniverses())
    {
      for (const CSGUniverse & univ : univ_list)
        ASSERT_EQ(univ, univ1);
    }
    // create new map and update lattice
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> new_univ_map = {
        {std::cref(univ2), std::cref(univ2), std::cref(univ2)},
        {std::cref(univ2), std::cref(univ2), std::cref(univ2)}};
    cart_lattice.setUniverses(new_univ_map);
    // expect map to contain all univ2
    for (auto univ_list : cart_lattice.getUniverses())
    {
      for (const CSGUniverse & univ : univ_list)
        ASSERT_EQ(univ, univ2);
    }
  }
}

/// tests CSGCartesianLattice::setUniverseAtIndex function
TEST(CSGLatticeTest, testCartSetUniverseAtIndex)
{
  // create initial lattice of all univ1 elements
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
      {std::cref(univ1), std::cref(univ1), std::cref(univ1)},
      {std::cref(univ1), std::cref(univ1), std::cref(univ1)}};
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
  // initial map should contain structure matching univ_map (all univ1)
  for (auto univ_list : cart_lattice.getUniverses())
  {
    for (const CSGUniverse & univ : univ_list)
      ASSERT_EQ(univ, univ1);
  }
  {
    // replace element in universe map with another using setUniverseAtIndex (valid index location)
    cart_lattice.setUniverseAtIndex(std::cref(univ2), std::make_pair(1, 2));
    auto univs = cart_lattice.getUniverses();
    for (auto i : index_range(univs))
    {
      for (auto j : index_range(univs[i]))
      {
        // all universes should be univ1 except (1, 2) location
        const CSGUniverse & univ = univs[i][j];
        if (i == 1 && j == 2)
          ASSERT_EQ(univ, univ2);
        else
          ASSERT_EQ(univ, univ1);
      }
    }
  }
  {
    // try replacing element at an invalid index - should raise error
    Moose::UnitUtils::assertThrows(
        [&cart_lattice, &univ2]() { cart_lattice.setUniverseAtIndex(univ2, std::make_pair(3, 3)); },
        "Cannot set universe at location (3, 3) for lattice cartlat. Not a valid location.");
  }
  {
    // create a lattice without any map initialized yet and try setting just one element
    // should raise error about map not being initialized yet
    auto cart_lattice = CSGCartesianLattice("cartlat", 2, 3, 1.0);
    Moose::UnitUtils::assertThrows(
        [&cart_lattice, &univ2]() { cart_lattice.setUniverseAtIndex(univ2, std::make_pair(0, 0)); },
        "Cannot set universe at location (0, 0) for lattice cartlat. "
        "Universe map has not been initialized.");
  }
}

/// tests CSGCartesianLattice different methods for retrieving universes or locations of universes
TEST(CSGLatticeTest, testCartGetMethods)
{
  // test get all and get by name (valid and invalid) and get at index (valid and invalid)
  // create initial lattice of all univ1 elements
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
      {std::cref(univ1), std::cref(univ2), std::cref(univ1)},
      {std::cref(univ2), std::cref(univ1), std::cref(univ2)}};
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
  {
    // get universe indices by name - valid name
    auto loc_list = cart_lattice.getUniverseIndices("univ1");
    std::vector<std::pair<int, int>> exp_locs = {
        std::make_pair(0, 0), std::make_pair(0, 2), std::make_pair(1, 1)};
    ASSERT_EQ(loc_list, exp_locs);
  }
  {
    // get universe indices by name - invalid name; should raise error
    Moose::UnitUtils::assertThrows([&cart_lattice]()
                                   { cart_lattice.getUniverseIndices("fake_name"); },
                                   "Universe fake_name does not exist in lattice");
  }
  {
    // get universe at index - valid index
    const CSGUniverse & retr_univ = cart_lattice.getUniverseAtIndex(std::make_pair(0, 1));
    ASSERT_EQ(retr_univ, univ2);
  }
  {
    // get universe at index - invalid index; should raise error
    Moose::UnitUtils::assertThrows([&cart_lattice]()
                                   { cart_lattice.getUniverseAtIndex(std::make_pair(3, 3)); },
                                   "Index (3, 3) is not a valid index for lattice ");
  }
}

// test setName functionality
TEST(CSGLatticeTest, testSetName)
{
  auto cart_lattice = CSGCartesianLattice("cartlat", 2, 3, 1.0);
  cart_lattice.setName("new_name");
  ASSERT_EQ(cart_lattice.getName(), "new_name");
}

// test the == and != overloaded operators for lattices
TEST(CSGLatticeTest, testCartLatticeEquality)
{
  // universe maps to use for different lattice comparisons
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map1 = {
      {std::cref(univ1), std::cref(univ1)}, {std::cref(univ1), std::cref(univ1)}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map2 = {
      {std::cref(univ2), std::cref(univ2)}, {std::cref(univ2), std::cref(univ2)}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map3 = {
      {std::cref(univ1), std::cref(univ1)}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map4 = {
      {std::cref(univ1)}, {std::cref(univ1)}};
  // identical lattices
  auto l1 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  auto l2 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  // lattice that differs by name only
  auto l3 = CSGCartesianLattice("cartlat1", 1.0, univ_map1);
  // lattice that differs by universe map items
  auto l4 = CSGCartesianLattice("cartlat", 1.0, univ_map2);
  // lattice that differs by pitch
  auto l5 = CSGCartesianLattice("cartlat", 2.0, univ_map1);
  // lattice that differs by nx0
  auto l6 = CSGCartesianLattice("cartlat", 1.0, univ_map3);
  // lattice that differs by nx1
  auto l7 = CSGCartesianLattice("cartlat", 1.0, univ_map4);

  // check equality
  {
    ASSERT_TRUE(l1 == l2);
  }
  // check inequality
  {
    // all lattices 1-7 should differ from each other in some way
    std::vector<CSGCartesianLattice> diff_compare = {l2, l3, l4, l5, l6, l7};
    for (std::size_t i = 0; i < diff_compare.size(); i++)
    {
      for (std::size_t j = i + 1; j < diff_compare.size(); ++j)
        ASSERT_TRUE(diff_compare[i] != diff_compare[j]);
    }
  }
}

// test CSGLattice::getUniqueUniverses
TEST(CSGLatticeTest, testGetUniqueUniverses)
{
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
      {std::cref(univ1), std::cref(univ1)}, {std::cref(univ2), std::cref(univ1)}};
  auto lat = CSGCartesianLattice("cartlat", 1.0, univ_map);
  auto unique = lat.getUniqueUniverses();
  ASSERT_EQ(unique.size(), 2);
  ASSERT_EQ(unique[0].get(), univ1);
  ASSERT_EQ(unique[1].get(), univ2);
}

// test CSGCartesianLattice::UpdateDimension
TEST(CSGLatticeTest, testCartUpdateDimension)
{
  auto cart_lattice = CSGCartesianLattice("cartlat", 6, 4, 1.0);
  {
    // update valid: pitch, nx0, and nx0
    cart_lattice.updateDimension("pitch", 1.5);
    cart_lattice.updateDimension("nx0", 2);
    cart_lattice.updateDimension("nx1", 3);
    // check new dimensions
    auto dims_map = cart_lattice.getDimensions();
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nx0"]), 2);
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nx1"]), 3);
    ASSERT_EQ(*std::any_cast<Real>(&dims_map["pitch"]), 1.5);
  }
  {
    // try update each value to something invalid (raise error)
    Moose::UnitUtils::assertThrows([&cart_lattice]()
                                   { cart_lattice.updateDimension("pitch", 0.0); },
                                   "Updated pitch value for lattice cartlat is not valid.");
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.updateDimension("nx0", -3); },
                                   "Updated nx0 value for lattice cartlat is not valid.");
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.updateDimension("nx1", -6); },
                                   "Updated nx1 value for lattice cartlat is not valid.");
  }
  {
    // update invalid: nx0 or nx1 when universes are already set (raise error)
    // set the universe map first
    const auto univ1 = CSGUniverse("univ1", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)},
        {std::cref(univ1), std::cref(univ1), std::cref(univ1)}};
    auto new_lat = CSGCartesianLattice("cartlat2", 1.0, univ_map);
    // try to update each dimension: pitch is valid, nx0 and nx1 are not valid
    new_lat.updateDimension("pitch", 2.0);
    auto dims_map = new_lat.getDimensions();
    ASSERT_EQ(*std::any_cast<Real>(&dims_map["pitch"]), 2.0);
    Moose::UnitUtils::assertThrows([&new_lat]() { new_lat.updateDimension("nx0", 3); },
                                   "Cannot update the dimension nx0 of the lattice cartlat2. "
                                   "Universe map is already defined.");
    Moose::UnitUtils::assertThrows([&new_lat]() { new_lat.updateDimension("nx1", 6); },
                                   "Cannot update the dimension nx1 of the lattice cartlat2. "
                                   "Universe map is already defined.");
  }
  {
    // try to update a dimension that does not exists
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.updateDimension("elmo", 3); },
                                   "Dimension elmo is not an allowable dimension for lattice type");
  }
}

TEST(CSGLatticeTest, testConvert)
{
  // test converting ring to row indices for hex lattices and vice versa
}
}
