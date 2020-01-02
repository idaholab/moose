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
  : DerivativeParsedMaterialHelper(parameters), ExpressionBuilder(parameters)
{
  EBTermList coupled_terms(getEBTermList("v"));
  EBTerm mat(getEBMaterial<Real>("easy", coupled_terms));
  EBTerm gradmat(grad(mat));
  EBTerm curlgradmat(curl(gradmat));
  EBTerm gradgradmat(grad(gradmat));

  for (unsigned int i = 0; i < _mat_prop_names.size(); ++i)
    std::cout << _mat_prop_names[i] << std::endl;

  std::cout << std::string(mat) << std::endl;
  for (unsigned int i = 0; i < 3; ++i)
    std::cout << gradmat[{i}] << std::endl;

  for (unsigned int i = 0; i < 3; ++i)
    std::cout << curlgradmat[{i}] << std::endl;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      std::cout << gradgradmat[{i, j}] << std::endl;
}
