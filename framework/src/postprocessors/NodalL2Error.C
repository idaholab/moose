//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalL2Error.h"
#include "Function.h"

registerMooseObject("MooseApp", NodalL2Error);

InputParameters
NodalL2Error::validParams()
{
  InputParameters params = NodalVariablePostprocessor::validParams();
  params.addClassDescription(
      "The L2-norm of the difference between a variable and a function computed at nodes.");
  params.addRequiredParam<FunctionName>("function", "The analytic solution to compare against");

  return params;
}

NodalL2Error::NodalL2Error(const InputParameters & parameters)
  : NodalVariablePostprocessor(parameters), _func(getFunction("function"))
{
}

void
NodalL2Error::initialize()
{
  _integral_value = 0.;
}

void
NodalL2Error::execute()
{
  Real diff = _u[0] - _func.value(_t, *_current_node);
  _integral_value += diff * diff;
}

Real
NodalL2Error::getValue()
{
  return std::sqrt(_integral_value);
}

void
NodalL2Error::threadJoin(const UserObject & y)
{
  const NodalL2Error & pps = static_cast<const NodalL2Error &>(y);
  _integral_value += pps._integral_value;
}

void
NodalL2Error::finalize()
{
  gatherSum(_integral_value);
}
