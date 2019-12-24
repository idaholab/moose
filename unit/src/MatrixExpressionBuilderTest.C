//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ExpressionBuilder.h"

class MatrixExpressionBuilderTest : public ::testing::Test, public ExpressionBuilder
{
};

TEST_F(MatrixExpressionBuilderTest, test)
{

  EBTerm new_test({1, 2, 3, 4, 5, 6, 7, 8, 9}, {3, 3});

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      std::cout << new_test[{i, j}] << " ";
}
