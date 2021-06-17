//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledValueFunctionIC.h"
#include "Function.h"

registerMooseObject("PhaseFieldApp", CoupledValueFunctionIC);

InputParameters
CoupledValueFunctionIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("Initialize the variable from a lookup function");
  params.addRequiredParam<FunctionName>("function",
                                        "Coupled function to evaluate with values from v");
  params.addCoupledVar("v",
                       "List of up to four coupled variables that are substituted for x,y,z, and t "
                       "in the coupled function");
  return params;
}

CoupledValueFunctionIC::CoupledValueFunctionIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _func(getFunction("function")),
    _var_num(coupledComponents("v")),
    _vals(coupledValues("v"))
{
  if (_var_num > 4)
    paramError("v", "You can couple at most four variables.");
}

Real
CoupledValueFunctionIC::value(const Point & /*p*/)
{
  Point p;
  Real t = 0.0;

  for (unsigned int i = 0; i < 3 && i < _var_num; ++i)
    p(i) = (*_vals[i])[_qp];
  if (_var_num == 4)
    t = (*_vals[3])[_qp];

  return _func.value(t, p);
}
