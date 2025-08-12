//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HexagonalLatticeUtils.h"
#include "MooseObjectUnitTest.h"

// Macro for computing absolute error
#define ABS_DOUBLE_TEST(value, ref_value)                                                          \
  EXPECT_LE(std::abs(((MetaPhysicL::raw_value(value)) - (MetaPhysicL::raw_value(ref_value)))),     \
            libMesh::TOLERANCE * libMesh::TOLERANCE)

class HexagonalLatticeTest : public MooseObjectUnitTest
{
public:
  HexagonalLatticeTest() : MooseObjectUnitTest("ReactorUnitApp") {}
};
