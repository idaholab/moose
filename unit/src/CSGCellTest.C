//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGCell.h"
#include "CSGUniverse.h"
#include "CSGRegion.h"
#include "CSGSphere.h"

#include "MooseUnitUtils.h"

namespace CSG
{

// helper function to convert infix JSON object to string representation
std::string
infixJSONToString(nlohmann::json infix_json)
{
  auto json_string = infix_json.dump();
  // Remove quotation marks from string
  json_string.erase(std::remove(json_string.begin(), json_string.end(), '\"'), json_string.end());
  // Replace square brackets with parentheses
  std::replace(json_string.begin(), json_string.end(), '[', '(');
  std::replace(json_string.begin(), json_string.end(), ']', ')');
  return json_string;
}

// helper function to setup the regions used for the cells in all tests
std::tuple<CSGRegion, CSGRegion, std::string, std::vector<std::string>>
setupRegions()
{
  static CSG::CSGSphere sphere("sphere_surf", 1.0);
  auto region1 = -sphere;
  auto region2 = +sphere;
  std::string reg_str_infix = "(-sphere_surf)";
  std::vector<std::string> reg_str_postfix{"sphere_surf", "-"};
  return std::make_tuple(region1, region2, reg_str_infix, reg_str_postfix);
}

// Test each type of CSGCell constructor
TEST(CSGCellTest, testVoidCell)
{
  auto [region1, region2, reg_str_infix, reg_str_postfix] = setupRegions();
  std::string name = "void_cell";
  CSGCell cell(name, region1);

  // check expected attributes
  ASSERT_EQ(name, cell.getName());
  ASSERT_EQ("VOID", cell.getFillType());
  ASSERT_EQ("", cell.getFillName());
  ASSERT_EQ(region1, cell.getRegion());
  auto region_json = cell.getRegion().toInfixJSON();
  ASSERT_EQ(reg_str_infix, infixJSONToString(region_json));
  ASSERT_EQ(reg_str_postfix, cell.getRegion().toPostfixStringList());
}

TEST(CSGCellTest, testMaterialCell)
{
  auto [region1, region2, reg_str_infix, reg_str_postfix] = setupRegions();
  std::string name = "mat_cell";
  std::string matname = "matname";
  CSGCell cell(name, matname, region1);

  // check expected attributes
  ASSERT_EQ(name, cell.getName());
  ASSERT_EQ("CSG_MATERIAL", cell.getFillType());
  ASSERT_EQ(matname, cell.getFillName());
  ASSERT_EQ(region1, cell.getRegion());
  auto region_json = cell.getRegion().toInfixJSON();
  ASSERT_EQ(reg_str_infix, infixJSONToString(region_json));
  ASSERT_EQ(reg_str_postfix, cell.getRegion().toPostfixStringList());
}

TEST(CSGCellTest, testUniverseCell)
{
  auto [region1, region2, reg_str_infix, reg_str_postfix] = setupRegions();
  std::string name = "univ_cell";
  std::string uname = "new_univ";
  const CSGUniverse univ(uname);
  CSGCell cell(name, &univ, region1);

  // check expected attributes
  ASSERT_EQ(name, cell.getName());
  ASSERT_EQ("UNIVERSE", cell.getFillType());
  ASSERT_EQ(uname, cell.getFillName());
  ASSERT_EQ(region1, cell.getRegion());
  auto region_json = cell.getRegion().toInfixJSON();
  ASSERT_EQ(reg_str_infix, infixJSONToString(region_json));
  ASSERT_EQ(reg_str_postfix, cell.getRegion().toPostfixStringList());
}

// tests CSGCell::getFillUniverse and CSGCell::getFillMaterial
TEST(CSGCellTest, testGetFill)
{
  auto [region1, region2, reg_str_infix, reg_str_postfix] = setupRegions();
  std::string cellname = "cellname";

  // get fill for void cell - expect errors for both methods
  {
    CSGCell cell(cellname, region1);
    Moose::UnitUtils::assertThrows([&cell]() { cell.getFillUniverse(); },
                                   "Cell '" + cellname + "' has VOID fill, not UNIVERSE.");
    Moose::UnitUtils::assertThrows([&cell]() { cell.getFillMaterial(); },
                                   "Cell '" + cellname + "' has VOID fill, not CSG_MATERIAL.");
  }
  // get fill for material cell - expect error for universe method
  {
    std::string matname = "matname";
    CSGCell cell(cellname, matname, region1);

    ASSERT_EQ(matname, cell.getFillMaterial());

    // expect error if trying to get universe fill from a material cell
    Moose::UnitUtils::assertThrows([&cell]() { cell.getFillUniverse(); },
                                   "Cell '" + cellname + "' has CSG_MATERIAL fill, not UNIVERSE.");
  }
  // get fill for universe cell - expect error for material method
  {
    const CSGUniverse univ("new_univ");
    CSGCell cell(cellname, &univ, region1);
    ASSERT_EQ(univ, cell.getFillUniverse());

    // expect error if trying to get universe fill from a material cell
    Moose::UnitUtils::assertThrows([&cell]() { cell.getFillMaterial(); },
                                   "Cell '" + cellname + "' has UNIVERSE fill, not CSG_MATERIAL.");
  }
}

// Test equality operators
TEST(CSGCellTest, testCellEquality)
{
  auto [region1, region2, reg_str_infix, reg_str_postfix] = setupRegions();

  // identical void cells
  CSGCell vcell1("void_cell", region1);
  CSGCell vcell2("void_cell", region1);
  // void cells that differ by one attribute
  CSGCell vcdiff1("void_cell", region2);
  CSGCell vcdiff2("void_cell2", region1);

  // identical material cells
  CSGCell mcell1("mat_cell", "matname", region1);
  CSGCell mcell2("mat_cell", "matname", region1);
  // material cells that differ by 1 attribute
  CSGCell mcdiff1("mat_cell", "matname", region2);
  CSGCell mcdiff2("mat_cell2", "matname", region1);
  CSGCell mcdiff3("mat_cell", "matname2", region1);

  // identical universe cells
  const CSGUniverse univ("new_univ");
  CSGCell ucell1("univ_cell", &univ, region1);
  CSGCell ucell2("univ_cell", &univ, region1);
  // universe cells that differ by one attribute
  CSGCell ucdiff1("univ_cell1", &univ, region1);
  CSGCell ucdiff2("univ_cell", &univ, region2);
  const CSGUniverse univ2("new_univ2");
  CSGCell ucdiff3("univ_cell", &univ2, region1);

  // check equality
  {
    ASSERT_TRUE(vcell1 == vcell2);
    ASSERT_TRUE(mcell1 == mcell2);
    ASSERT_TRUE(ucell1 == ucell2);
  }
  // check inequality
  {
    // all cells in this list differ by some way
    std::vector<CSGCell> diff_compare = {vcell1,
                                         vcdiff1,
                                         vcdiff2,
                                         mcell1,
                                         mcdiff1,
                                         mcdiff2,
                                         mcdiff3,
                                         ucell1,
                                         ucdiff1,
                                         ucdiff2,
                                         ucdiff3};
    for (std::size_t i = 0; i < diff_compare.size(); i++)
    {
      for (std::size_t j = i + 1; j < diff_compare.size(); ++j)
        ASSERT_TRUE(diff_compare[i] != diff_compare[j]);
    }
  }
}

/// CSGCell::setName
TEST(CSGCellTest, testSetName)
{
  auto [region1, region2, reg_str_infix, reg_str_postfix] = setupRegions();
  CSGCell cell("first_name", region1);
  cell.setName("new_name");
  ASSERT_EQ("new_name", cell.getName());
}

/// CSGCell::updateRegion
TEST(CSGCellTest, testUpdateRegion)
{
  auto [region1, region2, reg_str_infix, reg_str_postfix] = setupRegions();
  CSGCell cell("cellname", region1);
  cell.updateRegion(region2);
  ASSERT_EQ(region2, cell.getRegion());
}

} // namespace CSG
