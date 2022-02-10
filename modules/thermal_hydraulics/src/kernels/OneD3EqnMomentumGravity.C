//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneD3EqnMomentumGravity.h"

registerMooseObject("ThermalHydraulicsApp", OneD3EqnMomentumGravity);

InputParameters
OneD3EqnMomentumGravity::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");
  params.addClassDescription("Computes gravity term for the momentum equation for 1-phase flow");
  return params;
}

OneD3EqnMomentumGravity::OneD3EqnMomentumGravity(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),
    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector")),
    _arhoA_var_number(coupled("arhoA"))
{
}

Real
OneD3EqnMomentumGravity::computeQpResidual()
{
  return -_rho[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _test[_i][_qp];
}

Real
OneD3EqnMomentumGravity::computeQpJacobian()
{
  return 0;
}

Real
OneD3EqnMomentumGravity::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -_drho_darhoA[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else
    return 0;
}
