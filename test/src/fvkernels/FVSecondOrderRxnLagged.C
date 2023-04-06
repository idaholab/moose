//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVSecondOrderRxnLagged.h"

registerMooseObject("MooseTestApp", FVSecondOrderRxnLagged);

InputParameters
FVSecondOrderRxnLagged::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addRequiredParam<bool>(
      "lag", "Whether to lag one of the variable's evaluations by a nonlinear iteration.");
  return params;
}

FVSecondOrderRxnLagged::FVSecondOrderRxnLagged(const InputParameters & params)
  : FVElementalKernel(params), _lag(getParam<bool>("lag"))
{
}

ADReal
FVSecondOrderRxnLagged::computeQpResidual()
{
  const auto elem = makeElemArg(_current_elem);
  const auto lag_time = _lag ? Moose::previousNonlinearState() : Moose::currentState();
  return _u_functor(elem, Moose::currentState()) * _u_functor(elem, lag_time);
}
