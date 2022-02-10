//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneD3EqnMomentumFlux.h"

registerMooseObject("ThermalHydraulicsApp", OneD3EqnMomentumFlux);

InputParameters
OneD3EqnMomentumFlux::validParams()
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
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure material property");
  params.addClassDescription("Momentum flux for 1-phase flow");
  return params;
}

OneD3EqnMomentumFlux::OneD3EqnMomentumFlux(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),

    _A(coupledValue("A")),

    _dir(getMaterialProperty<RealVectorValue>("direction")),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),

    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),

    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

Real
OneD3EqnMomentumFlux::computeQpResidual()
{
  return -(_rho[_qp] * _vel[_qp] * _vel[_qp] + _p[_qp]) * _A[_qp] * _dir[_qp] * _grad_test[_i][_qp];
}

Real
OneD3EqnMomentumFlux::computeQpJacobian()
{
  return -(_rho[_qp] * 2.0 * _vel[_qp] * _dvel_darhouA[_qp] + _dp_darhouA[_qp]) * _A[_qp] *
         _dir[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
OneD3EqnMomentumFlux::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -(_drho_darhoA[_qp] * _vel[_qp] * _vel[_qp] +
             _rho[_qp] * 2.0 * _vel[_qp] * _dvel_darhoA[_qp] + _dp_darhoA[_qp]) *
           _A[_qp] * _dir[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return -_dp_darhoEA[_qp] * _A[_qp] * _dir[_qp] * _phi[_j][_qp] * _grad_test[_i][_qp];
  }
  else
    return 0.;
}
