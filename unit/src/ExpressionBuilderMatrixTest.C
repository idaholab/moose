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

class ExpressionBuilderMatrixTest : public ::testing::Test, public ExpressionBuilder
{
};

TEST_F(ExpressionBuilderMatrixTest, test)
{

  EBTerm a("a");
  EBTerm new_term = a - a + a + a- a - a + a;
  std::cout << new_term << std::endl;
  new_term.simplify();
  std::cout << new_term << std::endl;
}
