//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionScalarIC.h"

// MOOSE includes
#include "Function.h"
#include "MooseVariableScalar.h"

registerMooseObject("MooseApp", FunctionScalarIC);

InputParameters
FunctionScalarIC::validParams()
{
  InputParameters params = ScalarInitialCondition::validParams();
  params.addClassDescription("Initializes a scalar variable using a function.");
  params.addRequiredParam<std::vector<FunctionName>>("function", "The initial condition function.");
  return params;
}

FunctionScalarIC::FunctionScalarIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters), _ncomp(_var.order())
{
  std::vector<FunctionName> funcs = getParam<std::vector<FunctionName>>("function");
  if (funcs.size() != _ncomp)
    mooseError("number of functions must be equal to the scalar variable order");

  for (const auto & func_name : funcs)
    _func.push_back(&getFunctionByName(func_name));
}

Real
FunctionScalarIC::value()
{
  return _func[_i]->value(_t, _point_zero);
}
