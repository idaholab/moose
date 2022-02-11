//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneD3EqnMomentumAreaGradient.h"

registerMooseObject("ThermalHydraulicsApp", OneD3EqnMomentumAreaGradient);

InputParameters
OneD3EqnMomentumAreaGradient::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("arhoA", "The density of the kth phase");
  params.addRequiredCoupledVar("arhouA", "The momentum of the kth phase");
  params.addRequiredCoupledVar("arhoEA", "The total energy of the kth phase");
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredParam<MaterialPropertyName>(
      "direction", "The direction of the flow channel material property");
  params.addRequiredParam<MaterialPropertyName>("p", "Pressure");
  params.addClassDescription(
      "Computes the area gradient term in the momentum equation for single phase flow.");
  return params;
}

OneD3EqnMomentumAreaGradient::OneD3EqnMomentumAreaGradient(const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<Kernel>(parameters),
    _area_grad(coupledGradient("A")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _pressure(getMaterialProperty<Real>("p")),
    _dp_darhoA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoA")),
    _dp_darhouA(getMaterialPropertyDerivativeTHM<Real>("p", "arhouA")),
    _dp_darhoEA(getMaterialPropertyDerivativeTHM<Real>("p", "arhoEA")),
    _arhoA_var_number(coupled("arhoA")),
    _arhoE_var_number(coupled("arhoEA"))
{
}

Real
OneD3EqnMomentumAreaGradient::computeQpResidual()
{
  return -_pressure[_qp] * _area_grad[_qp] * _dir[_qp] * _test[_i][_qp];
}

Real
OneD3EqnMomentumAreaGradient::computeQpJacobian()
{
  return -_dp_darhouA[_qp] * _area_grad[_qp] * _dir[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

Real
OneD3EqnMomentumAreaGradient::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _arhoA_var_number)
  {
    return -_dp_darhoA[_qp] * _area_grad[_qp] * _dir[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else if (jvar == _arhoE_var_number)
  {
    return -_dp_darhoEA[_qp] * _area_grad[_qp] * _dir[_qp] * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0;
}
