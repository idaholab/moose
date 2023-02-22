//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenamedKernel.h"

registerMooseObjectRenamed("MooseTestApp", OldNamedKernel, "01/01/2050 00:00", RenamedKernel);
registerMooseObject("MooseTestApp", RenamedKernel);

InputParameters
RenamedKernel::validParams()
{
  InputParameters params = Reaction::validParams();
  params.addParam<Real>("coefficient", 1.0, "Coefficient of the term");
  params.renameParam("rate",
                     "base_coeff",
                     "The base coefficient multiplying the concentration in the Reaction kernel.");
  return params;
}

RenamedKernel::RenamedKernel(const InputParameters & parameters)
  : Reaction(parameters), _coef(getParam<Real>("coefficient"))
{
}

Real
RenamedKernel::computeQpResidual()
{
  return _coef * Reaction::computeQpResidual();
}

Real
RenamedKernel::computeQpJacobian()
{
  return _coef * Reaction::computeQpJacobian();
}
