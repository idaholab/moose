//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFEFluidVelocityTimeDerivative.h"

registerMooseObject("NavierStokesApp", PINSFEFluidVelocityTimeDerivative);
registerMooseObjectRenamed("NavierStokesApp",
                           PMFluidVelocityTimeDerivative,
                           "02/01/2024 00:00",
                           PINSFEFluidVelocityTimeDerivative);

InputParameters
PINSFEFluidVelocityTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addParam<bool>("conservative_form", false, "if conservative form is used");
  params.addRequiredCoupledVar("pressure", "coupled pressure");
  params.addRequiredCoupledVar("temperature", "coupled temperature");
  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");
  params.addClassDescription("Add the transient term for one component of the porous media "
                             "momentum conservation equation");

  return params;
}

PINSFEFluidVelocityTimeDerivative::PINSFEFluidVelocityTimeDerivative(
    const InputParameters & parameters)
  : TimeDerivative(parameters),
    _conservative_form(getParam<bool>("conservative_form")),
    _pressure(coupledValue("pressure")),
    _temperature(coupledValue("temperature")),
    _temperature_dot(coupledDot("temperature")),
    _pressure_dot(coupledDot("pressure")),
    _rho(getMaterialProperty<Real>("rho_fluid")),
    _eos(getUserObject<SinglePhaseFluidProperties>("eos"))
{
}

Real
PINSFEFluidVelocityTimeDerivative::computeQpResidual()
{
  Real res = _rho[_qp] * TimeDerivative::computeQpResidual();
  if (_conservative_form)
  {
    Real rho, drho_dp, drho_dT;
    _eos.rho_from_p_T(_pressure[_qp], _temperature[_qp], rho, drho_dp, drho_dT);
    Real drho_dt = drho_dT * _temperature_dot[_qp] + drho_dp * _pressure_dot[_qp];
    res += _u[_qp] * drho_dt * _test[_i][_qp];
  }

  return res;
}

Real
PINSFEFluidVelocityTimeDerivative::computeQpJacobian()
{
  Real jac = _rho[_qp] * TimeDerivative::computeQpJacobian();
  if (_conservative_form)
  {
    Real rho, drho_dp, drho_dT;
    _eos.rho_from_p_T(_pressure[_qp], _temperature[_qp], rho, drho_dp, drho_dT);
    Real drho_dt = drho_dT * _temperature_dot[_qp] + drho_dp * _pressure_dot[_qp];
    jac += _phi[_j][_qp] * drho_dt * _test[_i][_qp];
  }

  return jac;
}
