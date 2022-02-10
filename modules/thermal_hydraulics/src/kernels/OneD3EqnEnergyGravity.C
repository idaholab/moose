//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneD3EqnEnergyGravity.h"

registerMooseObject("ThermalHydraulicsApp", OneD3EqnEnergyGravity);

InputParameters
OneD3EqnEnergyGravity::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");
  params.addRequiredParam<RealVectorValue>("gravity_vector", "Gravitational acceleration vector");
  params.addClassDescription("Computes gravity term for the energy equation in 1-phase flow");
  return params;
}

OneD3EqnEnergyGravity::OneD3EqnEnergyGravity(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),
    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),
    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _gravity_vector(getParam<RealVectorValue>("gravity_vector")),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA"))
{
}

Real
OneD3EqnEnergyGravity::computeQpResidual()
{
  return -_rho[_qp] * _vel[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _test[_i][_qp];
}

Real
OneD3EqnEnergyGravity::computeQpJacobian()
{
  return 0;
}

Real
OneD3EqnEnergyGravity::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -(_drho_darhoA[_qp] * _vel[_qp] + _rho[_qp] * _dvel_darhoA[_qp]) * _A[_qp] *
           _gravity_vector * _dir[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    return -_rho[_qp] * _dvel_darhouA[_qp] * _A[_qp] * _gravity_vector * _dir[_qp] * _phi[_j][_qp] *
           _test[_i][_qp];
  }
  else
    return 0;
}
