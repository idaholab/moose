/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SimpleCoupledACInterface.h"

template <>
InputParameters
validParams<SimpleCoupledACInterface>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription(
      "Gradient energy for Allen-Cahn Kernel with constant Mobility and Interfacial parameter");
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
