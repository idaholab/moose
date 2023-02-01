//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFEFluidTemperatureTimeDerivative.h"

registerMooseObject("NavierStokesApp", PINSFEFluidTemperatureTimeDerivative);
registerMooseObjectRenamed("NavierStokesApp",
                           PMFluidTemperatureTimeDerivative,
                           "02/01/2024 00:00",
                           PINSFEFluidTemperatureTimeDerivative);

InputParameters
PINSFEFluidTemperatureTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addParam<bool>("conservative_form", false, "if conservative form is used");
  params.addRequiredCoupledVar("pressure", "coupled pressure");
  params.addRequiredCoupledVar("porosity", "porosity");
  params.addRequiredParam<UserObjectName>("eos", "The name of equation of state object to use.");

  return params;
}

PINSFEFluidTemperatureTimeDerivative::PINSFEFluidTemperatureTimeDerivative(
    const InputParameters & parameters)
  : TimeDerivative(parameters),
    _conservative_form(getParam<bool>("conservative_form")),
    _pressure(coupledValue("pressure")),
    _pressure_dot(coupledDot("pressure")),
    _porosity(coupledValue("porosity")),
    _rho(getMaterialProperty<Real>("rho_fluid")),
    _cp(getMaterialProperty<Real>("cp_fluid")),
    _eos(getUserObject<SinglePhaseFluidProperties>("eos"))
{
}

Real
PINSFEFluidTemperatureTimeDerivative::computeQpResidual()
{
  Real res = _porosity[_qp] * _rho[_qp] * _cp[_qp] * _u_dot[_qp] * _test[_i][_qp];
  if (_conservative_form)
  {
    Real rho, drho_dp, drho_dT;
    _eos.rho_from_p_T(_pressure[_qp], _u[_qp], rho, drho_dp, drho_dT);
    Real drho_dt = drho_dT * _u_dot[_qp] + drho_dp * _pressure_dot[_qp];
    res += _porosity[_qp] * _cp[_qp] * _u[_qp] * drho_dt * _test[_i][_qp];
  }

  return res;
}

Real
PINSFEFluidTemperatureTimeDerivative::computeQpJacobian()
{
  Real jac = _porosity[_qp] * _rho[_qp] * _cp[_qp] * TimeDerivative::computeQpJacobian();
  if (_conservative_form)
  {
    Real rho, drho_dp, drho_dT;
    _eos.rho_from_p_T(_pressure[_qp], _u[_qp], rho, drho_dp, drho_dT);
    Real drho_dt = drho_dT * _u_dot[_qp] + drho_dp * _pressure_dot[_qp];
    jac += _porosity[_qp] * _cp[_qp] * _phi[_j][_qp] * drho_dt * _test[_i][_qp];
  }

  return jac;
}
