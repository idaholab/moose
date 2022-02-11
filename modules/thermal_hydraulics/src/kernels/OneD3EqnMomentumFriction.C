//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneD3EqnMomentumFriction.h"

registerMooseObject("ThermalHydraulicsApp", OneD3EqnMomentumFriction);

InputParameters
OneD3EqnMomentumFriction::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>("D_h", "Hydraulic diameter");
  params.addRequiredCoupledVar("arhoA", "Solution variable alpha*rho*A");
  params.addRequiredCoupledVar("arhouA", "Solution variable alpha*rho*u*A");
  params.addRequiredCoupledVar("arhoEA", "Solution variable alpha*rho*E*A");
  params.addRequiredParam<MaterialPropertyName>("rho", "Density property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity property");
  params.addRequiredParam<MaterialPropertyName>("f_D",
                                                "Darcy friction factor coefficient property");
  params.addClassDescription("Computes wall friction term for single phase flow.");
  return params;
}

OneD3EqnMomentumFriction::OneD3EqnMomentumFriction(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _A(coupledValue("A")),
    _D_h(getMaterialProperty<Real>("D_h")),
    _rho(getMaterialProperty<Real>("rho")),
    _drho_darhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "arhoA")),
    _vel(getMaterialProperty<Real>("vel")),
    _dvel_darhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhoA")),
    _dvel_darhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "arhouA")),
    _f_D(getMaterialProperty<Real>("f_D")),
    _df_D_darhoA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhoA")),
    _df_D_darhouA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhouA")),
    _df_D_darhoEA(getMaterialPropertyDerivativeTHM<Real>("f_D", "arhoEA")),
    _arhoA_var_number(coupled("arhoA")),
    _arhouA_var_number(coupled("arhouA")),
    _arhoEA_var_number(coupled("arhoEA"))
{
}

Real
OneD3EqnMomentumFriction::computeQpResidual()
{
  return 0.5 * _f_D[_qp] * _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] / _D_h[_qp] *
         _test[_i][_qp];
}

Real
OneD3EqnMomentumFriction::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
OneD3EqnMomentumFriction::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    const Real vel2 = _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel2_darhoA = 2 * std::abs(_vel[_qp]) * _dvel_darhoA[_qp];
    return (_df_D_darhoA[_qp] * _rho[_qp] * vel2 + _f_D[_qp] * _drho_darhoA[_qp] * vel2 +
            _f_D[_qp] * _rho[_qp] * dvel2_darhoA) *
           0.5 * _A[_qp] / _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhouA_var_number)
  {
    const Real vel2 = _vel[_qp] * std::abs(_vel[_qp]);
    const Real dvel2_darhouA = 2 * std::abs(_vel[_qp]) * _dvel_darhouA[_qp];
    return (_df_D_darhouA[_qp] * vel2 + _f_D[_qp] * dvel2_darhouA) * 0.5 * _rho[_qp] * _A[_qp] /
           _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoEA_var_number)
  {
    return _df_D_darhoEA[_qp] * 0.5 * _rho[_qp] * _vel[_qp] * std::abs(_vel[_qp]) * _A[_qp] /
           _D_h[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
