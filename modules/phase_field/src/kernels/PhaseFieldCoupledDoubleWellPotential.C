//* This file is part of the MOOSE framework
//* https://www.mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldCoupledDoubleWellPotential.h"

registerMooseObject("PhaseFieldApp", PhaseFieldCoupledDoubleWellPotential);

InputParameters
PhaseFieldCoupledDoubleWellPotential::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addRequiredCoupledVar("c", "phase field variable");
  params.addRequiredParam<Real>("prefactor", "prefactor for double well potential");
  return params;
}

PhaseFieldCoupledDoubleWellPotential::PhaseFieldCoupledDoubleWellPotential(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _c(adCoupledValue("c")),
    _prefactor(getParam<Real>("prefactor"))


{
}

ADReal
PhaseFieldCoupledDoubleWellPotential::precomputeQpResidual()
{
  return _prefactor*_c[_qp]*(_c[_qp]*_c[_qp] - 1);
}
 