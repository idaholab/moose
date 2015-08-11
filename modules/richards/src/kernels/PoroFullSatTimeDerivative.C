/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PoroFullSatTimeDerivative.h"


template<>
InputParameters validParams<PoroFullSatTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addRequiredCoupledVar("disp_z", "The z displacement");
  params.addClassDescription("Kernel = biot_coefficient*d(volumetric_strain)/dt + (1/biot_modulus)*d(porepressure)/dt.  This is the time-derivative for poromechanics for a single-phase, fully-saturated fluid with constant bulk modulus");
  return params;
}

PoroFullSatTimeDerivative::PoroFullSatTimeDerivative(const InputParameters & parameters) :
    DerivativeMaterialInterface<TimeDerivative>(parameters),
    _u_old(valueOld()),
    _volstrain(getMaterialProperty<Real>("volumetric_strain")),
    _volstrain_old(getMaterialPropertyOld<Real>("volumetric_strain")),

    _disp_x_var(coupled("disp_x")),
    _disp_y_var(coupled("disp_y")),
    _disp_z_var(coupled("disp_z")),

    _alpha(getMaterialProperty<Real>("biot_coefficient")),

    _one_over_biot_modulus(getMaterialProperty<Real>("one_over_biot_modulus")),
    _done_over_biot_modulus_dP(getMaterialPropertyDerivative<Real>("one_over_biot_modulus", _var.name())),
    _done_over_biot_modulus_dep(getMaterialPropertyDerivative<Real>("one_over_biot_modulus", "volumetric_strain"))

{
}


Real
PoroFullSatTimeDerivative::computeQpResidual()
{
  // here, "_u" is the porepressure
  Real res = _one_over_biot_modulus[_qp]*(_u[_qp] - _u_old[_qp]);
  res += _alpha[_qp]*(_volstrain[_qp] - _volstrain_old[_qp]);
  return _test[_i][_qp]*res/_dt;
}


Real
PoroFullSatTimeDerivative::computeQpJacobian()
{
  Real jac = _one_over_biot_modulus[_qp]*_phi[_j][_qp];
  jac += _done_over_biot_modulus_dP[_qp]*_phi[_j][_qp]*(_u[_qp] - _u_old[_qp]);
  return _test[_i][_qp]*jac/_dt;
}

Real
PoroFullSatTimeDerivative::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (!(jvar == _disp_x_var || jvar == _disp_y_var || jvar == _disp_z_var))
    return 0.0;

  Real jac = _done_over_biot_modulus_dep[_qp]*(_u[_qp] - _u_old[_qp]);
  jac += _alpha[_qp];

  if (jvar == _disp_x_var)
    jac *= _grad_phi[_j][_qp](0);
  else if (jvar == _disp_y_var)
    jac *= _grad_phi[_j][_qp](1);
  else
    jac *= _grad_phi[_j][_qp](2);

  return _test[_i][_qp]*jac/_dt;
}

