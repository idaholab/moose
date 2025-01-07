//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "SolutionScalarAux.h"
#include "SolutionUserObjectBase.h"

registerMooseObject("MooseApp", SolutionScalarAux);

InputParameters
SolutionScalarAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addClassDescription(
      "Sets scalar variable by using information from a SolutionUserObject.");
  params.addRequiredParam<UserObjectName>("solution", "The name of the SolutionUserObject");
  params.addParam<std::string>("from_variable",
                               "The name of the variable to extract from the file");
  params.addParam<Real>(
      "scale_factor",
      1.0,
      "Scale factor (a)  to be applied to the solution (x): ax+b, where b is the 'add_factor'");
  params.addParam<Real>(
      "add_factor",
      0.0,
      "Add this value (b) to the solution (x): ax+b, where a is the 'scale_factor'");
  return params;
}

SolutionScalarAux::SolutionScalarAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _solution_object(getUserObject<SolutionUserObjectBase>("solution")),
    _scale_factor(getParam<Real>("scale_factor")),
    _add_factor(getParam<Real>("add_factor"))
{
}

void
SolutionScalarAux::initialSetup()
{
  if (isParamValid("from_variable"))
    _var_name = getParam<std::string>("from_variable");
  else
  {
    const std::vector<std::string> & vars = _solution_object.variableNames();
    if (vars.size() > 1)
      mooseError(name(),
                 ": The SolutionUserObject contains multiple variables, please specifiy the "
                 "desired variable in the input file using the 'from_variable' parameter.");

    _var_name = vars[0];
  }
}

Real
SolutionScalarAux::computeValue()
{
  Real value = _solution_object.scalarValue(_t, _var_name);
  return _scale_factor * value + _add_factor;
}
