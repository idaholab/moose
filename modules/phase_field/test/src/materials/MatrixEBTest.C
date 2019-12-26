//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatrixEBTest.h"

registerMooseObject("PhaseFieldApp", MatrixEBTest);

template <>
InputParameters
validParams<MatrixEBTest>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Test to see if calculus components of coupled variables can be used "
                             "(gradient & second derivative terms) in ExpressionBuilder");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of other coupled order parameters");

  return params;
}

MatrixEBTest::MatrixEBTest(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters)
{
  EBTerm new_test({1, 2, 3, 4, 5, 6, 7, 8, 9}, {3, 3});
  EBTerm newer_test(new_test * new_test);

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      std::cout << newer_test[{i, j}] << std::endl;
}
