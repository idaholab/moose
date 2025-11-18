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
#include "CSGCell.h"

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
    // initialize without universes: nrow=2, ncol=3, pitch=1.0
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0);
    // check dimensions/attributes
    ASSERT_EQ(cart_lattice.getNRows(), 0);
    ASSERT_EQ(cart_lattice.getNCols(), 0);
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
        {univ1, univ1, univ1}, {univ1, univ1, univ1}};
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
    // check dimensions/attributes
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
    // try initialize with invalid dimension for pitch
    Moose::UnitUtils::assertThrows(
        []() { CSGCartesianLattice("cartlat", -1.0); },
        "Lattice cartlat must have pitch greater than 0."); // invalid pitch
  }
  {
    // try to initialize with universe array of invalid dimensions (second row is different length)
    const auto univ1 = CSGUniverse("univ1", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1, univ1}, {univ1, univ1}};
    std::string exp_msg = "Cannot set lattice cartlat with universes. Does not have valid "
                          "dimensions for lattice type CSG::CSGCartesianLattice";
    Moose::UnitUtils::assertThrows([&univ_map]() { CSGCartesianLattice("cartlat", 1.0, univ_map); },
                                   exp_msg);
  }
}

/// tests valid CSGHexagonalLattice construction
TEST(CSGLatticeTest, testCreateHexLatticeValid)
{
  {
    // intiialize without universes: pitch=1.0
    auto hex_lat = CSGHexagonalLattice("hexlat", 1.0);
    // check dimensions and properties
    ASSERT_EQ(hex_lat.getNRows(), 0);
    ASSERT_EQ(hex_lat.getNRings(), 0);
    ASSERT_EQ(hex_lat.getPitch(), 1.0);
    ASSERT_EQ(hex_lat.getUniverses().size(), 0); // no universe map yet
    ASSERT_TRUE(hex_lat.getName() == "hexlat");
    ASSERT_TRUE(hex_lat.getType() == "CSG::CSGHexagonalLattice");
  }
  {
    // initialize with universe map, pitch=1.0
    const auto univ1 = CSGUniverse("univ1", false);
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1, univ1},
        {univ1, univ1, univ1, univ1},
        {univ1, univ1, univ1, univ1, univ1},
        {univ1, univ1, univ1, univ1},
        {univ1, univ1, univ1}};
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
  const auto univ1 = CSGUniverse("univ1", false);
  {
    // try initialize empty by providing invalid dimensions
    Moose::UnitUtils::assertThrows(
        []() { CSGHexagonalLattice("hexlat", -1.0); },
        "Lattice hexlat must have pitch greater than 0."); // invalid pitch
  }
  {
    // create universe map with invalid dimensions (even number of rows)
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1, univ1},
        {univ1, univ1, univ1, univ1},
        {univ1, univ1, univ1, univ1},
        {univ1, univ1, univ1}};
    std::string exp_msg = "Cannot set lattice hexlat with universes. Does not have valid "
                          "dimensions for lattice type CSG::CSGHexagonalLattice";
    Moose::UnitUtils::assertThrows([&univ_map]() { CSGHexagonalLattice("hexlat", 1.0, univ_map); },
                                   exp_msg);
  }
  {
    // create universe map with invalid dimensions (one row has wrong number of elements)
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1, univ1},
        {univ1, univ1, univ1, univ1},
        {univ1, univ1, univ1, univ1}, // should have 5 elements
        {univ1, univ1, univ1, univ1},
        {univ1, univ1, univ1}};
    std::string exp_msg = "Cannot set lattice hexlat with universes. Does not have valid "
                          "dimensions for lattice type CSG::CSGHexagonalLattice";
    Moose::UnitUtils::assertThrows([&univ_map]() { CSGHexagonalLattice("hexlat", 1.0, univ_map); },
                                   exp_msg);
  }
}

/// tests getAttributes function for both CSGCartesianLattice and CSGHexagonalLattice
TEST(CSGLatticeTest, testGetAttributes)
{
  const auto univ1 = CSGUniverse("univ1", false);
  {
    // cartesian lattice
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1, univ1}, {univ1, univ1, univ1}};
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
    auto dims_map = cart_lattice.getAttributes();
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nrow"]), 2);
    ASSERT_EQ(*std::any_cast<int>(&dims_map["ncol"]), 3);
    ASSERT_EQ(*std::any_cast<Real>(&dims_map["pitch"]), 1.0);
  }
  {
    // hexagonal lattice
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1}, {univ1, univ1, univ1}, {univ1, univ1}};
    auto hex_lattice = CSGHexagonalLattice("hexlat", 1.0, univ_map);
    auto dims_map = hex_lattice.getAttributes();
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nrow"]), 3);
    ASSERT_EQ(*std::any_cast<int>(&dims_map["nring"]), 2); // should be (nrow + 1)/2
    ASSERT_EQ(*std::any_cast<Real>(&dims_map["pitch"]), 1.0);
  }
}

/// tests CSGCartesianLattice::setUniverses function
TEST(CSGLatticeTest, testCartSetUniverses)
{
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  // create initial lattice without a map
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0);
  {
    // create universe map and set it on the initialized lattice
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1, univ1}, {univ1, univ1, univ1}};
    ASSERT_NO_THROW(cart_lattice.setUniverses(univ_map));
    // should have 1x4 map after being set
    ASSERT_EQ(cart_lattice.getUniverses().size(), 2);
    ASSERT_EQ(cart_lattice.getUniverses()[0].size(), 3);
    ASSERT_EQ(cart_lattice.getUniverses()[1].size(), 3);
    // make sure dimensions were updated
    ASSERT_EQ(cart_lattice.getNRows(), 2);
    ASSERT_EQ(cart_lattice.getNCols(), 3);
  }
  {
    // overwrite w/ new universe map of different dimensions - valid
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> new_univ_map = {
        {univ2, univ2, univ2, univ2}};
    ASSERT_NO_THROW(cart_lattice.setUniverses(new_univ_map));
    // expect map to contain all univ2
    for (auto univ_list : cart_lattice.getUniverses())
    {
      for (const CSGUniverse & univ : univ_list)
        ASSERT_EQ(univ, univ2);
    }
    // should have 1x4 map after being set
    ASSERT_EQ(cart_lattice.getUniverses().size(), 1);
    ASSERT_EQ(cart_lattice.getUniverses()[0].size(), 4);
    // make sure dimensions were updated
    ASSERT_EQ(cart_lattice.getNRows(), 1);
    ASSERT_EQ(cart_lattice.getNCols(), 4);
  }
}

/// tests CSGHexagonalLattice::setUniverses function
TEST(CSGLatticeTest, testHexSetUniverses)
{
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  // initial empty lattice
  auto lat = CSGHexagonalLattice("lat", 1.0);
  {
    // create universe map and then set it on the initialized lattice
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
        {univ1, univ1}, {univ1, univ1, univ1}, {univ1, univ1}};
    ASSERT_NO_THROW(lat.setUniverses(univ_map););
    // should have a 2-ring map (3 rows) after being set
    ASSERT_EQ(lat.getUniverses().size(), 3);
    ASSERT_EQ(lat.getUniverses()[0].size(), 2);
    ASSERT_EQ(lat.getUniverses()[1].size(), 3);
    ASSERT_EQ(lat.getUniverses()[2].size(), 2);
    // make sure dimensions were set
    ASSERT_EQ(lat.getNRows(), 3);
  }
  {
    // create new map with new dimensions and update lattice
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> new_univ_map = {
        {univ2, univ2, univ2},
        {univ2, univ2, univ2, univ2},
        {univ2, univ2, univ2, univ2, univ2},
        {univ2, univ2, univ2, univ2},
        {univ2, univ2, univ2}};
    lat.setUniverses(new_univ_map);
    // expect map to contain all univ2
    for (auto univ_list : lat.getUniverses())
    {
      for (const CSGUniverse & univ : univ_list)
        ASSERT_EQ(univ, univ2);
    }
    // should have 3-ring (5 row) map after being set
    ASSERT_EQ(lat.getUniverses().size(), 5);
    ASSERT_EQ(lat.getUniverses()[0].size(), 3);
    ASSERT_EQ(lat.getUniverses()[1].size(), 4);
    ASSERT_EQ(lat.getUniverses()[2].size(), 5);
    ASSERT_EQ(lat.getUniverses()[3].size(), 4);
    ASSERT_EQ(lat.getUniverses()[4].size(), 3);
    // make sure dimensions were updated
    ASSERT_EQ(lat.getNRows(), 5);
  }
}

/// tests CSGLattice::getUniverseNameMap function
TEST(CSGLatticeTest, testGetUniverseNameMap)
{
  std::string name1 = "pinky";
  std::string name2 = "brain";
  const auto univ1 = CSGUniverse(name1, false);
  const auto univ2 = CSGUniverse(name2, false);
  // create cartesian lattice with 2x2 universe map
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {{univ1, univ2},
                                                                                  {univ2, univ1}};
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
  auto name_map = cart_lattice.getUniverseNameMap();
  ASSERT_EQ(name_map.size(), 2);
  ASSERT_EQ(name_map[0].size(), 2);
  ASSERT_EQ(name_map[1].size(), 2);
  ASSERT_EQ(name_map[0][0], name1);
  ASSERT_EQ(name_map[0][1], name2);
  ASSERT_EQ(name_map[1][0], name2);
  ASSERT_EQ(name_map[1][1], name1);
}

/// tests CSGLattice::hasUniverse function
TEST(CSGLatticeTest, testHasUniverse)
{
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {{univ1, univ2}};
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
  // check for existing universes
  ASSERT_TRUE(cart_lattice.hasUniverse("univ1"));
  ASSERT_TRUE(cart_lattice.hasUniverse("univ2"));
  // check for non-existing universe
  ASSERT_FALSE(cart_lattice.hasUniverse("univ3"));
}

/// tests CSGCartesianLattice::isValidIndex function
TEST(CSGLatticeTest, testCartIsValidIndex)
{
  // create initial lattice of all univ1 elements
  const auto univ1 = CSGUniverse("univ1", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
      {univ1, univ1, univ1}, {univ1, univ1, univ1}};
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
  {
    // test valid index locations
    ASSERT_TRUE(cart_lattice.isValidIndex(std::make_pair(0, 0)));
    ASSERT_TRUE(cart_lattice.isValidIndex(std::make_pair(1, 2)));
  }
  {
    // test invalid index locations
    ASSERT_FALSE(cart_lattice.isValidIndex(std::make_pair(2, 0)));  // row out of bounds
    ASSERT_FALSE(cart_lattice.isValidIndex(std::make_pair(0, 3)));  // col out of bounds
    ASSERT_FALSE(cart_lattice.isValidIndex(std::make_pair(-1, 0))); // negative row
    ASSERT_FALSE(cart_lattice.isValidIndex(std::make_pair(0, -1))); // negative col
  }
}

/// tests CSGHexagonalLattice::isValidIndex function
TEST(CSGLatticeTest, testHexIsValidIndex)
{
  // create initial lattice of all univ1 elements
  const auto univ1 = CSGUniverse("univ1", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
      {univ1, univ1}, {univ1, univ1, univ1}, {univ1, univ1}};
  auto hex_lattice = CSGHexagonalLattice("hexlat", 1.0, univ_map);
  {
    // valid list of indices for 2-ring hex lattice:
    std::vector<std::pair<unsigned int, unsigned int>> valid_indices = {std::make_pair(0, 0),
                                                                        std::make_pair(0, 1),
                                                                        std::make_pair(1, 0),
                                                                        std::make_pair(1, 1),
                                                                        std::make_pair(1, 2),
                                                                        std::make_pair(2, 0),
                                                                        std::make_pair(2, 1)};
    // check that all valid indices return true
    for (const auto & index : valid_indices)
      ASSERT_TRUE(hex_lattice.isValidIndex(index));
  }
  {
    // check invalid for each case is caught
    ASSERT_FALSE(hex_lattice.isValidIndex(std::make_pair(0, 2)));  // col out of bounds (row 0)
    ASSERT_FALSE(hex_lattice.isValidIndex(std::make_pair(1, 3)));  // col out of bounds (row 1)
    ASSERT_FALSE(hex_lattice.isValidIndex(std::make_pair(3, 0)));  // row out of bounds
    ASSERT_FALSE(hex_lattice.isValidIndex(std::make_pair(-1, 0))); // negative row
    ASSERT_FALSE(hex_lattice.isValidIndex(std::make_pair(0, -1))); // negative col
  }
}

/// tests CSGCartesianLattice::setUniverseAtIndex function
TEST(CSGLatticeTest, testCartSetUniverseAtIndex)
{
  // create initial lattice of all univ1 elements
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
      {univ1, univ1, univ1}, {univ1, univ1, univ1}};
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
  // initial map should contain structure matching univ_map (all univ1)
  for (auto univ_list : cart_lattice.getUniverses())
  {
    for (const CSGUniverse & univ : univ_list)
      ASSERT_EQ(univ, univ1);
  }
  {
    // replace element in universe map with another using setUniverseAtIndex (valid index location)
    cart_lattice.setUniverseAtIndex(univ2, std::make_pair(1, 2));
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
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0);
    Moose::UnitUtils::assertThrows(
        [&cart_lattice, &univ2]() { cart_lattice.setUniverseAtIndex(univ2, std::make_pair(0, 0)); },
        "Cannot set universe at location (0, 0) for lattice cartlat. "
        "Universe map has not been initialized.");
  }
}

/// tests CSGCartesianLattice different methods for retrieving universes or locations of universes
TEST(CSGLatticeTest, testGetMethods)
{
  // test get all and get by name (valid and invalid) and get at index (valid and invalid)
  // create initial lattice of all univ1 elements
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {
      {univ1, univ2, univ1}, {univ2, univ1, univ2}};
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0, univ_map);
  {
    // get universe indices by name - valid name
    auto loc_list = cart_lattice.getUniverseIndices("univ1");
    std::vector<std::pair<unsigned int, unsigned int>> exp_locs = {
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

/// test setName functionality
TEST(CSGLatticeTest, testSetName)
{
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0);
  cart_lattice.setName("new_name");
  ASSERT_EQ(cart_lattice.getName(), "new_name");
}

/// test the == and != overloaded operators for cartesian lattices
TEST(CSGLatticeTest, testCartLatticeEquality)
{
  // universe maps to use for different lattice comparisons
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  const auto out1 = CSGUniverse("outer1", false);
  const auto out2 = CSGUniverse("outer2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map1 = {{univ1, univ1},
                                                                                   {univ1, univ1}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map2 = {{univ2, univ2},
                                                                                   {univ2, univ2}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map3 = {{univ1, univ1}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map4 = {{univ1},
                                                                                   {univ1}};
  // identical lattices
  auto l1 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  auto l2 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  // lattice that differs by name only
  auto l3 = CSGCartesianLattice("cartlat1", 1.0, univ_map1);
  // lattice that differs by universe map items
  auto l4 = CSGCartesianLattice("cartlat", 1.0, univ_map2);
  // lattice that differs by pitch
  auto l5 = CSGCartesianLattice("cartlat", 2.0, univ_map1);
  // lattice that differs by nrow
  auto l6 = CSGCartesianLattice("cartlat", 1.0, univ_map3);
  // lattice that differs by ncol
  auto l7 = CSGCartesianLattice("cartlat", 1.0, univ_map4);
  // differs by outer type - material outer
  auto l8 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  l8.updateOuter("outer1");
  // differs by outer type - universe outer
  auto l9 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  l9.updateOuter(out1);
  // differs by outer object - universe outer
  auto l10 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  l10.updateOuter(out2);
  // differs by outer name - material outer
  auto l11 = CSGCartesianLattice("cartlat", 1.0, univ_map1);
  l11.updateOuter("outer2");

  // check equality
  {
    ASSERT_TRUE(l1 == l2);
  }
  // check inequality
  {
    // all lattices 2-7 should differ from each other in some way
    std::vector<CSGCartesianLattice> diff_compare = {l2, l3, l4, l5, l6, l7, l8, l9, l10, l11};
    for (std::size_t i = 0; i < diff_compare.size(); i++)
    {
      for (std::size_t j = i + 1; j < diff_compare.size(); ++j)
        ASSERT_TRUE(diff_compare[i] != diff_compare[j]);
    }
  }
}

/// test the == and != overloaded operators for hexagonal lattices
TEST(CSGLatticeTest, testHexLatticeEquality)
{
  // universe maps to use for different lattice comparisons
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map1 = {
      {univ1, univ1}, {univ1, univ1, univ1}, {univ1, univ1}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map2 = {
      {univ2, univ2}, {univ2, univ2, univ2}, {univ2, univ2}};
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map3 = {
      {univ1, univ1, univ1},
      {univ1, univ1, univ1, univ1},
      {univ1, univ1, univ1, univ1, univ1},
      {univ1, univ1, univ1, univ1},
      {univ1, univ1, univ1}};
  // identical lattices
  auto l1 = CSGHexagonalLattice("hexlat", 1.0, univ_map1);
  auto l2 = CSGHexagonalLattice("hexlat", 1.0, univ_map1);
  // lattice that differs by name only
  auto l3 = CSGHexagonalLattice("hexlat1", 1.0, univ_map1);
  // lattice that differs by universe map items
  auto l4 = CSGHexagonalLattice("hexlat", 1.0, univ_map2);
  // lattice that differs by pitch
  auto l5 = CSGHexagonalLattice("hexlat", 2.0, univ_map1);
  // lattice that differs by nrow/rings
  auto l6 = CSGHexagonalLattice("hexlat", 1.0, univ_map3);

  // check equality
  {
    ASSERT_TRUE(l1 == l2);
  }
  // check inequality
  {
    // all lattices 2-6 should differ from each other in some way
    std::vector<CSGHexagonalLattice> diff_compare = {l2, l3, l4, l5, l6};
    for (std::size_t i = 0; i < diff_compare.size(); i++)
    {
      for (std::size_t j = i + 1; j < diff_compare.size(); ++j)
        ASSERT_TRUE(diff_compare[i] != diff_compare[j]);
    }
  }
}

/// test CSGLattice::getUniqueUniverses
TEST(CSGLatticeTest, testGetUniqueUniverses)
{
  const auto univ1 = CSGUniverse("univ1", false);
  const auto univ2 = CSGUniverse("univ2", false);
  std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {{univ1, univ1},
                                                                                  {univ2, univ1}};
  auto lat = CSGCartesianLattice("cartlat", 1.0, univ_map);
  auto unique = lat.getUniqueUniverses();
  ASSERT_EQ(unique.size(), 2);
  ASSERT_EQ(unique[0].get(), univ1);
  ASSERT_EQ(unique[1].get(), univ2);
}

/// test CSG[Cartesian/Hexagonal]Lattice::setPitch
TEST(CSGLatticeTest, testSetPitch)
{
  {
    // cartesian lattice set pitch
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0);
    // set valid pitch
    cart_lattice.setPitch(2.5);
    ASSERT_EQ(cart_lattice.getPitch(), 2.5);
    // try to set invalid pitch (raise error)
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.setPitch(-0.5); },
                                   "must have pitch greater than 0.");
  }
  {
    // hexagonal lattice set pitch
    auto hex_lat = CSGHexagonalLattice("hex_lat", 1.0);
    // set valid pitch
    hex_lat.setPitch(2.5);
    ASSERT_EQ(hex_lat.getPitch(), 2.5);
    // try to set invalid pitch (raise error)
    Moose::UnitUtils::assertThrows([&hex_lat]() { hex_lat.setPitch(-0.5); },
                                   "must have pitch greater than 0.");
  }
}

/// test CSGHexagonalLattice::get[Ring/Row]IndexFrom[Row/Ring]Index
TEST(CSGLatticeTest, testHexConvertRowsRings)
{
  // test that conversion between ring and row form works on 2-, 3-, and 4-ring hex lattices
  const auto u = CSGUniverse("u", false);
  {
    // 2-ring lattice case
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> umap = {
        {u, u}, {u, u, u}, {u, u}};
    auto lat = CSGHexagonalLattice("lat", 1.0, umap);
    std::map<std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int>>
        exp_row_to_ring = {{{0, 0}, {0, 4}},
                           {{0, 1}, {0, 5}},
                           {{1, 0}, {0, 3}},
                           {{1, 1}, {1, 0}},
                           {{1, 2}, {0, 0}},
                           {{2, 0}, {0, 2}},
                           {{2, 1}, {0, 1}}};
    for (const auto & pair : exp_row_to_ring)
    {
      auto row_pair = lat.getRowIndexFromRingIndex(pair.second);
      ASSERT_EQ(row_pair, pair.first);
      auto ring_pair = lat.getRingIndexFromRowIndex(pair.first);
      ASSERT_EQ(ring_pair, pair.second);
    }
    // check that invalid ring index raises error
    Moose::UnitUtils::assertThrows([&lat]() { lat.getRowIndexFromRingIndex(std::make_pair(2, 0)); },
                                   "Ring 2 is not valid for hexagonal lattice lat");
    Moose::UnitUtils::assertThrows([&lat]() { lat.getRowIndexFromRingIndex(std::make_pair(1, 7)); },
                                   "Element 7 is not valid for ring 1 in hexagonal lattice lat");
    // check that invalid row-column index raises error
    Moose::UnitUtils::assertThrows([&lat]() { lat.getRingIndexFromRowIndex(std::make_pair(3, 0)); },
                                   "Index (3, 0) is not a valid index for hexagonal lattice lat");
  }
  {
    // 3-ring case
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> umap = {
        {u, u, u}, {u, u, u, u}, {u, u, u, u, u}, {u, u, u, u}, {u, u, u}};
    auto lat = CSGHexagonalLattice("lat", 1.0, umap);
    std::map<std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int>>
        exp_row_to_ring = {{{0, 0}, {0, 8}},
                           {{0, 1}, {0, 9}},
                           {{0, 2}, {0, 10}},
                           {{1, 0}, {0, 7}},
                           {{1, 1}, {1, 4}},
                           {{1, 2}, {1, 5}},
                           {{1, 3}, {0, 11}},
                           {{2, 0}, {0, 6}},
                           {{2, 1}, {1, 3}},
                           {{2, 2}, {2, 0}},
                           {{2, 3}, {1, 0}},
                           {{2, 4}, {0, 0}},
                           {{3, 0}, {0, 5}},
                           {{3, 1}, {1, 2}},
                           {{3, 2}, {1, 1}},
                           {{3, 3}, {0, 1}},
                           {{4, 0}, {0, 4}},
                           {{4, 1}, {0, 3}},
                           {{4, 2}, {0, 2}}};
    for (const auto & pair : exp_row_to_ring)
    {
      auto row_pair = lat.getRowIndexFromRingIndex(pair.second);
      ASSERT_EQ(row_pair, pair.first);
      auto ring_pair = lat.getRingIndexFromRowIndex(pair.first);
      ASSERT_EQ(ring_pair, pair.second);
    }
  }
  {
    // 4-ring case
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> umap = {
        {u, u, u, u},
        {u, u, u, u, u},
        {u, u, u, u, u, u},
        {u, u, u, u, u, u, u},
        {u, u, u, u, u, u},
        {u, u, u, u, u},
        {u, u, u, u}};
    auto lat = CSGHexagonalLattice("lat", 1.0, umap);
    std::map<std::pair<unsigned int, unsigned int>, std::pair<unsigned int, unsigned int>>
        exp_row_to_ring = {
            {{0, 0}, {0, 12}}, {{0, 1}, {0, 13}}, {{0, 2}, {0, 14}}, {{0, 3}, {0, 15}},
            {{1, 0}, {0, 11}}, {{1, 1}, {1, 8}},  {{1, 2}, {1, 9}},  {{1, 3}, {1, 10}},
            {{1, 4}, {0, 16}}, {{2, 0}, {0, 10}}, {{2, 1}, {1, 7}},  {{2, 2}, {2, 4}},
            {{2, 3}, {2, 5}},  {{2, 4}, {1, 11}}, {{2, 5}, {0, 17}}, {{3, 0}, {0, 9}},
            {{3, 1}, {1, 6}},  {{3, 2}, {2, 3}},  {{3, 3}, {3, 0}},  {{3, 4}, {2, 0}},
            {{3, 5}, {1, 0}},  {{3, 6}, {0, 0}},  {{4, 0}, {0, 8}},  {{4, 1}, {1, 5}},
            {{4, 2}, {2, 2}},  {{4, 3}, {2, 1}},  {{4, 4}, {1, 1}},  {{4, 5}, {0, 1}},
            {{5, 0}, {0, 7}},  {{5, 1}, {1, 4}},  {{5, 2}, {1, 3}},  {{5, 3}, {1, 2}},
            {{5, 4}, {0, 2}},  {{6, 0}, {0, 6}},  {{6, 1}, {0, 5}},  {{6, 2}, {0, 4}},
            {{6, 3}, {0, 3}}};
    for (const auto & pair : exp_row_to_ring)
    {
      auto row_pair = lat.getRowIndexFromRingIndex(pair.second);
      ASSERT_EQ(row_pair, pair.first);
      auto ring_pair = lat.getRingIndexFromRowIndex(pair.first);
      ASSERT_EQ(ring_pair, pair.second);
    }
  }
}

/// tests that a lattice initialized without a universe map can be filled later even after it is used to fill a cell
TEST(CSGLatticeTest, testEmptyToFilled)
{
  const auto univ1 = CSGUniverse("univ1", false);
  {
    // create lattice without any universes
    auto cart_lattice = CSGCartesianLattice("cartlat", 1.0);
    // create cell and fill with the empty lattice
    auto cell = CSGCell("cell1", &cart_lattice, CSG::CSGRegion());
    // now create universe map and set it on the lattice
    std::vector<std::vector<std::reference_wrapper<const CSGUniverse>>> univ_map = {{univ1, univ1},
                                                                                    {univ1, univ1}};
    ASSERT_NO_THROW(cart_lattice.setUniverses(univ_map));
    // verify that the lattice in the cell has the correct universe map now
    const auto & lat_in_cell = cell.getFillLattice();
    ASSERT_EQ(lat_in_cell.getUniverses().size(), 2);
    ASSERT_EQ(lat_in_cell.getUniverses()[0].size(), 2);
  }
}

TEST(CSGLatticeTest, testUpdateOuter)
{
  auto cart_lattice = CSGCartesianLattice("cartlat", 1.0);
  {
    // check outer universe type is set to VOID by default
    ASSERT_EQ(cart_lattice.getOuterType(), "VOID");
    // make sure trying to get material or universe outer raises error
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.getOuterMaterial(); },
                                   "Lattice 'cartlat' has VOID outer, not CSG_MATERIAL.");
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.getOuterUniverse(); },
                                   "Lattice 'cartlat' has VOID outer, not UNIVERSE.");
  }
  {
    // update outer to universe
    const auto univ = CSGUniverse("univ", false);
    cart_lattice.updateOuter(univ);
    ASSERT_EQ(cart_lattice.getOuterType(), "UNIVERSE");
    ASSERT_EQ(cart_lattice.getOuterUniverse(), univ);
    // try to get material outer - should raise error because type is UNIVERSE
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.getOuterMaterial(); },
                                   "Lattice 'cartlat' has UNIVERSE outer, not CSG_MATERIAL.");
  }
  {
    // change outer type to a material name
    cart_lattice.updateOuter("material");
    ASSERT_EQ(cart_lattice.getOuterType(), "CSG_MATERIAL");
    ASSERT_EQ(cart_lattice.getOuterMaterial(), "material");
    // try to get universe outer - should raise error because type is CSG_MATERIAL
    Moose::UnitUtils::assertThrows([&cart_lattice]() { cart_lattice.getOuterUniverse(); },
                                   "Lattice 'cartlat' has CSG_MATERIAL outer, not UNIVERSE.");
  }
  {
    // reset outer type - should change it back to VOID
    cart_lattice.resetOuter();
    ASSERT_EQ(cart_lattice.getOuterType(), "VOID");
  }
}
}
