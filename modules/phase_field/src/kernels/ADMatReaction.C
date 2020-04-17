//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMatReaction.h"

registerMooseObject("PhaseFieldApp", ADMatReaction);

InputParameters
ADMatReaction::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addCoupledVar("v",
                       "Set this to make v a coupled variable, otherwise it will use the "
                       "kernel's nonlinear variable for v");
  params.addClassDescription("Kernel to add -L*v, where L=reaction rate, v=variable");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The reaction rate used with the kernel");
  return params;
}

ADMatReaction::ADMatReaction(const InputParameters & parameters)
  : ADKernel(parameters),
    _v(isCoupled("v") ? adCoupledValue("v") : _u),
    _mob(getADMaterialProperty<Real>("mob_name"))
{
}

ADReal
ADMatReaction::computeQpResidual()
{
  return -_mob[_qp] * _test[_i][_qp] * _v[_qp];
}
