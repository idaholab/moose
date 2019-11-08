//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WrongJacobianDiffusion.h"

registerMooseObject("MooseTestApp", WrongJacobianDiffusion);

InputParameters
WrongJacobianDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<Real>("rfactor", 1.0, "Prefactor on the Residual");
  params.addParam<Real>("jfactor", 1.0, "Prefactor on the Jacobian");
  params.addCoupledVar("coupled", "optionally couple variables");
  return params;
}

WrongJacobianDiffusion::WrongJacobianDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _rfactor(getParam<Real>("rfactor")), _jfactor(getParam<Real>("jfactor"))
{
}

Real
WrongJacobianDiffusion::computeQpResidual()
{
  return _rfactor * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
WrongJacobianDiffusion::computeQpJacobian()
{
  return _jfactor * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}

Real
WrongJacobianDiffusion::computeQpOffDiagJacobian(unsigned int)
{
  return 1.0;
}
