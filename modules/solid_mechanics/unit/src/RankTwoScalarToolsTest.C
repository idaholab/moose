//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "RankTwoTensor.h"
#include "RankTwoScalarTools.h"

TEST(RankTwoScalarToolsTest, ErrorComputingEV)
{
  // generate tensor that does not allow computation of eigenvectors
  // and fails because of nan entries

  RankTwoTensor a;
  a(0, 0) = a(0, 1) = a(0, 2) = std::numeric_limits<double>::quiet_NaN();
  a(1, 0) = a(1, 1) = a(1, 2) = a(0, 0);
  a(2, 0) = a(2, 1) = a(2, 2) = a(0, 0);

  try
  {
    Point p(1, 0, 0);
    RankTwoScalarTools::calcEigenValuesEigenVectors(a, 0, p);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("In computing the eigenvalues and eigenvectors");
    ASSERT_TRUE(pos != std::string::npos);
  }
}
