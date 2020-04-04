//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PoroFullSatTimeDerivative.h"

registerMooseObject("RichardsApp", PoroFullSatTimeDerivative);

InputParameters
PoroFullSatTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addClassDescription("Kernel = biot_coefficient*d(volumetric_strain)/dt + "
                             "(1/biot_modulus)*d(porepressure)/dt.  This is the time-derivative "
                             "for poromechanics for a single-phase, fully-saturated fluid with "
                             "constant bulk modulus");
  return params;
}

PoroFullSatTimeDerivative::PoroFullSatTimeDerivative(const InputParameters & parameters)
  : DerivativeMaterialInterface<TimeDerivative>(parameters),
    _u_old(valueOld()),
    _volstrain(getMaterialProperty<Real>("volumetric_strain")),
    _volstrain_old(getMaterialPropertyOld<Real>("volumetric_strain")),

    _ndisp(coupledComponents("displacements")),
    _disp_var_num(_ndisp),

    _alpha(getMaterialProperty<Real>("biot_coefficient")),

    _one_over_biot_modulus(getMaterialProperty<Real>("one_over_biot_modulus")),
    _done_over_biot_modulus_dP(
        getMaterialPropertyDerivative<Real>("one_over_biot_modulus", _var.name())),
    _done_over_biot_modulus_dep(
        getMaterialPropertyDerivative<Real>("one_over_biot_modulus", "volumetric_strain"))
{
  for (unsigned i = 0; i < _ndisp; ++i)
    _disp_var_num[i] = coupled("displacements", i);
}

Real
PoroFullSatTimeDerivative::computeQpResidual()
{
  // here, "_u" is the porepressure
  Real res = _one_over_biot_modulus[_qp] * (_u[_qp] - _u_old[_qp]);
  res += _alpha[_qp] * (_volstrain[_qp] - _volstrain_old[_qp]);
  return _test[_i][_qp] * res / _dt;
}

Real
PoroFullSatTimeDerivative::computeQpJacobian()
{
  Real jac = _one_over_biot_modulus[_qp] * _phi[_j][_qp];
  jac += _done_over_biot_modulus_dP[_qp] * _phi[_j][_qp] * (_u[_qp] - _u_old[_qp]);
  return _test[_i][_qp] * jac / _dt;
}

Real
PoroFullSatTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real jac = 0;
  for (unsigned i = 0; i < _ndisp; ++i)
    if (jvar == _disp_var_num[i])
      jac = _grad_phi[_j][_qp](i);

  jac *= _done_over_biot_modulus_dep[_qp] * (_u[_qp] - _u_old[_qp]) + _alpha[_qp];

  return _test[_i][_qp] * jac / _dt;
}
