//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneD3EqnEnergyFlux.h"

registerMooseObject("ThermalHydraulicsApp", OneD3EqnEnergyFlux);

InputParameters
OneD3EqnEnergyFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density material property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity material property");
  params.addRequiredParam<MaterialPropertyName>("e", "Specific internal energy material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure material property");
  params.addClassDescription("Energy flux for single phase flow");
  return params;
}

OneD3EqnEnergyFlux::OneD3EqnEnergyFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),
    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),
    _e(getMaterialProperty<Real>("e")),
    _de_darhoA(getMaterialPropertyDerivativeTHM<Real>("e", "arhoA")),
    _de_darhouA(getMaterialPropertyDerivativeTHM<Real>("e", "arhouA")),
    _de_darhoEA(getMaterialPropertyDerivativeTHM<Real>("e", "arhoEA")),
    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA"))
{
}

Real
OneD3EqnEnergyFlux::computeQpResidual()
{
  return -_vel[_qp] * _dir[_qp] * (_rho[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) + _p[_qp]) *
         _A[_qp] * _grad_test[_i][_qp];
}

Real
OneD3EqnEnergyFlux::computeQpJacobian()
{
  return -_vel[_qp] * _dir[_qp] * (_rho[_qp] * _de_darhoEA[_qp] + _dp_darhoEA[_qp]) * _A[_qp] *
         _phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
OneD3EqnEnergyFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -(_dvel_darhoA[_qp] * (_rho[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) + _p[_qp]) +
             _vel[_qp] * (_drho_darhoA[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) +
                          _rho[_qp] * (_de_darhoA[_qp] + _vel[_qp] * _dvel_darhoA[_qp]) +
                          _dp_darhoA[_qp])) *
           _dir[_qp] * _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    return -(_dvel_darhouA[_qp] * (_rho[_qp] * (_e[_qp] + 0.5 * _vel[_qp] * _vel[_qp]) + _p[_qp]) +
             _vel[_qp] * (_rho[_qp] * (_de_darhouA[_qp] + _vel[_qp] * _dvel_darhouA[_qp]) +
                          _dp_darhouA[_qp])) *
           _dir[_qp] * _A[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else
    return 0.;
}
