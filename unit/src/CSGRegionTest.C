//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGPlane.h"
#include "CSGRegion.h"

#include "MooseUnitUtils.h"

using namespace CSG;

// Test each type of CSGRegion operator
TEST(CSGRegionTest, testRegionOperators)
{
  // surfaces to use for regions:
  CSGPlane surf1("s1", 1.0, 0.0, 0.0, 1.0);
  CSGPlane surf2("s2", 1.0, 0.0, 0.0, 2.0);

  // positive (+) and negative (-) halfspaces
  auto pos_half = +surf1;
  auto neg_half = -surf2;
  // intersection of two halspaces (&)
  auto inter = pos_half & neg_half;
  // intersection with existing region (&=)
  auto new_reg_int = +surf1;
  new_reg_int &= neg_half;
  // union of two halfspaces (|)
  auto uni = pos_half | neg_half;
  // union with existing region (|=)
  auto new_reg_uni = +surf1;
  new_reg_uni |= neg_half;
  // empty region (initialized only)
  CSGRegion empty;
  // complement (~)
  auto complement = ~inter;

  // assert each region is of the correct type and string matches

  // halfspaces
  {
    std::string pos_string = "+s1";
    std::string neg_string = "-s2";
    ASSERT_EQ(CSGRegion::RegionType::HALFSPACE, pos_half.getRegionType());
    ASSERT_EQ(CSGRegion::RegionType::HALFSPACE, neg_half.getRegionType());
    ASSERT_EQ(pos_string, pos_half.toString());
    ASSERT_EQ(neg_string, neg_half.toString());
  }
  // intersections
  {
    std::string inter_string = "(+s1 & -s2)";
    ASSERT_EQ(CSGRegion::RegionType::INTERSECTION, inter.getRegionType());
    ASSERT_EQ(CSGRegion::RegionType::INTERSECTION, new_reg_int.getRegionType());
    ASSERT_EQ(inter_string, inter.toString());
    ASSERT_EQ(inter_string, new_reg_int.toString());
  }
  // unions
  {
    std::string union_string = "(+s1 | -s2)";
    ASSERT_EQ(CSGRegion::RegionType::UNION, uni.getRegionType());
    ASSERT_EQ(CSGRegion::RegionType::UNION, new_reg_uni.getRegionType());
    ASSERT_EQ(union_string, uni.toString());
    ASSERT_EQ(union_string, new_reg_uni.toString());
  }
  // initialized but empty
  {
    ASSERT_EQ(CSGRegion::RegionType::EMPTY, empty.getRegionType());
  }
  // complement
  {
    std::string comp_string = "~(+s1 & -s2)";
    ASSERT_EQ(CSGRegion::RegionType::COMPLEMENT, complement.getRegionType());
    ASSERT_EQ(comp_string, complement.toString());
  }
}

// Tests errors are caught with using wrong region types
TEST(CSGRegionTest, testRegionErrors)
{
  CSGPlane surf1("s1", 1.0, 0.0, 0.0, 1.0);
  CSGPlane surf2("s2", 1.0, 0.0, 0.0, 2.0);
  auto reg1 = +surf1;
  auto reg2 = -surf2;
  CSGRegion empty;

  // expect error if trying to union or intersect empty region(s)
  {
    Moose::UnitUtils::assertThrows(
        [&empty, &reg1]() { empty & reg1; },
        "Region operation INTERSECTION cannot be performed on an empty region.");
    Moose::UnitUtils::assertThrows(
        [&empty, &reg1]() { reg1 | empty; },
        "Region operation UNION cannot be performed on an empty region.");
  }
  // expect error if wrong number of regions are called with the different region types
  {
    Moose::UnitUtils::assertThrows([&reg1, &reg2]() { CSGRegion(reg1, reg2, "HALFSPACE"); },
                                   "Region type HALFSPACE is not supported for two regions.");
    Moose::UnitUtils::assertThrows([&reg1, &reg2]() { CSGRegion(reg1, reg2, "COMPLEMENT"); },
                                   "Region type COMPLEMENT is not supported for two regions.");
    Moose::UnitUtils::assertThrows([&reg1, &reg2]() { CSGRegion(reg1, reg2, "EMPTY"); },
                                   "Region type EMPTY is not supported for two regions.");
    Moose::UnitUtils::assertThrows([&reg1]() { CSGRegion(reg1, "HALFSPACE"); },
                                   "Region type HALFSPACE is not supported for a single region.");
    Moose::UnitUtils::assertThrows([&reg1]() { CSGRegion(reg1, "UNION"); },
                                   "Region type UNION is not supported for a single region.");
    Moose::UnitUtils::assertThrows(
        [&reg1]() { CSGRegion(reg1, "INTERSECTION"); },
        "Region type INTERSECTION is not supported for a single region.");
  }
}

// Tests == and != operators
TEST(CSGRegionTest, testEquality)
{
  CSGPlane surf("s1", 1.0, 0.0, 0.0, 1.0);

  auto reg1 = +surf;         // halfspace type
  auto reg2 = +surf;         // identical type and region
  auto reg3 = -surf;         // identical type, different region
  auto reg4 = -surf | +surf; // different region and type

  // == operator
  {
    ASSERT_TRUE(reg1 == reg2);
  }
  // != operator
  {
    // different region string
    ASSERT_TRUE(reg1 != reg3);
    // different type and string
    ASSERT_TRUE(reg1 != reg4);
  }
}
