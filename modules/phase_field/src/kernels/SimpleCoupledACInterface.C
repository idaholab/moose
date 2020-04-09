//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SimpleCoupledACInterface.h"

registerMooseObject("PhaseFieldApp", SimpleCoupledACInterface);

InputParameters
SimpleCoupledACInterface::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Gradient energy for Allen-Cahn Kernel with constant Mobility and "
                             "Interfacial parameter for a coupled order parameter variable.");
  params.addRequiredCoupledVar("v", "Coupled variable that the Laplacian is taken of");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  params.addParam<MaterialPropertyName>("kappa_name", "kappa_op", "The kappa used with the kernel");
  return params;
}

SimpleCoupledACInterface::SimpleCoupledACInterface(const InputParameters & parameters)
  : Kernel(parameters),
    _L(getMaterialProperty<Real>("mob_name")),
    _kappa(getMaterialProperty<Real>("kappa_name")),
    _grad_v(coupledGradient("v")),
    _v_var(coupled("v", 0))
{
}

Real
SimpleCoupledACInterface::computeQpResidual()
{
  return _grad_v[_qp] * _kappa[_qp] * _L[_qp] * _grad_test[_i][_qp];
}

Real
SimpleCoupledACInterface::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _v_var)
    return _grad_phi[_j][_qp] * _kappa[_qp] * _L[_qp] * _grad_test[_i][_qp];

  return 0.0;
}
