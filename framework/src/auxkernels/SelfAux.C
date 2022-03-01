//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SelfAux.h"

registerMooseObject("MooseApp", SelfAux);

InputParameters
SelfAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Returns the specified variable as an auxiliary variable with the same value.");
  params.addCoupledVar("v",
                       "Optional variable to take the value of. If omitted the value of the "
                       "`variable` itself is returned.");
  return params;
}

SelfAux::SelfAux(const InputParameters & parameters)
  : AuxKernel(parameters), _v(isCoupled("v") ? coupledValue("v") : _u)
{
}

Real
SelfAux::computeValue()
{
  return _v[_qp];
}
