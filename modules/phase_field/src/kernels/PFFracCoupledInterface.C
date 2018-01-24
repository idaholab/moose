//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFFracCoupledInterface.h"

template <>
InputParameters
validParams<PFFracCoupledInterface>()
{
  InputParameters params = validParams<KernelGrad>();
  params.addClassDescription("Phase-field fracture residual for beta variable: Contribution from "
                             "gradient of damage order parameter");
  params.addRequiredCoupledVar("c", "Order parameter for damage");
  return params;
}

PFFracCoupledInterface::PFFracCoupledInterface(const InputParameters & parameters)
  : KernelGrad(parameters),
    _c(coupledValue("c")),
    _grad_c(coupledGradient("c")),
    _c_var(coupled("c"))
{
}

RealGradient
PFFracCoupledInterface::precomputeQpResidual()
{
  return _grad_c[_qp];
}

RealGradient
PFFracCoupledInterface::precomputeQpJacobian()
{
  return 0.0;
}

Real
PFFracCoupledInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _c_var)
    return _grad_test[_i][_qp] * _grad_phi[_j][_qp];
  else
    return 0.0;
}
