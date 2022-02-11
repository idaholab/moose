//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDMomentumFreeBC.h"

registerMooseObject("ThermalHydraulicsApp", OneDMomentumFreeBC);

InputParameters
OneDMomentumFreeBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();
  params.addRequiredCoupledVar("arhoA", "alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "alpha*rho*E*A");
  params.addRequiredCoupledVar("vel", "x-component of velocity");
  params.addRequiredCoupledVar("A", "Area");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure property name");
  return params;
}

OneDMomentumFreeBC::OneDMomentumFreeBC(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<OneDIntegratedBC>(parameters),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA")),
    _vel(coupledValue("vel")),
    _area(coupledValue("A")),
    _p(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA"))
{
}

Real
OneDMomentumFreeBC::computeQpResidual()
{
  Real F2 = _u[_qp] * _vel[_qp] + _p[_qp] * _area[_qp];
  return F2 * _normal * _test[_i][_qp];
}

Real
OneDMomentumFreeBC::computeQpJacobian()
{
  return (2. * _vel[_qp] + _dp_darhouA[_qp] * _area[_qp]) * _normal * _phi[_j][_qp] *
         _test[_i][_qp];
}

Real
OneDMomentumFreeBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _arhoA_var_number)
    return (-_vel[_qp] * _vel[_qp] + _dp_darhoA[_qp] * _area[_qp]) * _normal * _phi[_j][_qp] *
           _test[_i][_qp];

  else if (jvar == _arhoEA_var_number)
    return _dp_darhoEA[_qp] * _area[_qp] * _normal * _phi[_j][_qp] * _test[_i][_qp];

  else
    return 0.;
}
