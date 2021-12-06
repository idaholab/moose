//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IntegralPreservingFunctionIC.h"
#include "Function.h"
#include "UserObject.h"

registerMooseObject("MooseApp", IntegralPreservingFunctionIC);

InputParameters
IntegralPreservingFunctionIC::validParams()
{
  InputParameters params = FunctionIC::validParams();
  params.addRequiredParam<PostprocessorName>(
      "integral", "Postprocessor providing the integral of the function, for normalization");
  params.addRequiredParam<Real>("magnitude",
                                "Desired magnitude of the initial condition upon integration");
  params.addClassDescription("Function initial condition that preserves an integral");
  return params;
}

IntegralPreservingFunctionIC::IntegralPreservingFunctionIC(const InputParameters & parameters)
  : FunctionIC(parameters),
    _pp_name(getParam<PostprocessorName>("integral")),
    _integral(getPostprocessorValue("integral")),
    _magnitude(getParam<Real>("magnitude"))
{
}

void
IntegralPreservingFunctionIC::initialSetup()
{
  const UserObject & pp = _fe_problem.getUserObject<UserObject>(_pp_name);
  if (!pp.getExecuteOnEnum().contains(EXEC_INITIAL))
    mooseError("The 'execute_on' parameter for the '" + _pp_name +
               "' postprocessor must include 'initial'!");
}

Real
IntegralPreservingFunctionIC::value(const Point & p)
{
  if (std::abs(_integral) < libMesh::TOLERANCE)
    mooseError("The integral of '" + _pp_name + "' cannot be zero!");

  return magnitude() * _func.value(_t, p) / _integral;
}
