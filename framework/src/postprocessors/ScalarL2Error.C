//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarL2Error.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", ScalarL2Error);

InputParameters
ScalarL2Error::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Compute L2 error of a scalar variable using analytic function.");
  params.addRequiredParam<VariableName>("variable", "The name of the scalar variable");
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");
  return params;
}

ScalarL2Error::ScalarL2Error(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var(_subproblem.getScalarVariable(_tid, getParam<VariableName>("variable"))),
    _func(getFunction("function"))
{
}

void
ScalarL2Error::initialize()
{
}

void
ScalarL2Error::execute()
{
}

Real
ScalarL2Error::getValue()
{
  _var.reinit();
  Real diff = (_var.sln()[0] - _func.value(_t));
  return std::sqrt(diff * diff);
}
