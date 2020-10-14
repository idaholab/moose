//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EBCoupledVarTest.h"

registerMooseObject("PhaseFieldTestApp", EBCoupledVarTest);

InputParameters
EBCoupledVarTest::validParams()
{
  InputParameters params = DerivativeParsedMaterialHelper::validParams();
  params.addClassDescription(
      "Test to see if vector of coupled variables works with ExpressionBuilder");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");

  return params;
}

EBCoupledVarTest::EBCoupledVarTest(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters), _op_num(coupledComponents("v")), _vals(_op_num)
{
  EBTerm newest("v");
  std::string new_name;
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    new_name = "v" + std::to_string(i);
    EBTerm new_term(new_name.c_str());
    _vals[i] = new_term;
  }
  EBFunction tester;
  tester(_vals) = _vals[0] + 2 * _vals[1];

  functionParse(tester);
}
