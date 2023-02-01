//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFEFluidEnergyKernel.h"

registerMooseObject("NavierStokesApp", INSFEFluidEnergyKernel);
registerMooseObjectRenamed("NavierStokesApp",
                           MDFluidEnergyKernel,
                           "02/01/2024 00:00",
                           INSFEFluidEnergyKernel);

InputParameters
INSFEFluidEnergyKernel::validParams()
{
  InputParameters params = INSFEFluidKernelStabilization::validParams();
  params.addClassDescription("Adds advection, diffusion, and heat source terms to energy equation, "
                             "potentially with stabilization");
  params.addParam<bool>("conservative_form", false, "if conservative form is used");
  params.addCoupledVar("porosity_elem", "Element averaged porosity");
  params.addCoupledVar("power_density", "volumetric heat source");
  params.addCoupledVar("pke_power_var", "Normalized power from PKE");
  params.addParam<FunctionName>("power_shape_function", "Function that defines power profile");

  return params;
}

INSFEFluidEnergyKernel::INSFEFluidEnergyKernel(const InputParameters & parameters)
  : INSFEFluidKernelStabilization(parameters),
    _conservative_form(getParam<bool>("conservative_form")),
    _k_elem(getMaterialProperty<Real>("k_fluid_elem")),
    _cp(getMaterialProperty<Real>("cp_fluid")),
    _has_porosity_elem(isParamValid("porosity_elem")),
    _porosity_elem(_has_porosity_elem ? coupledValue("porosity_elem")
                                      : (_has_porosity ? coupledValue("porosity") : _zero)),
    _has_qv(isParamValid("power_density")),
    _qv(_has_qv ? coupledValue("power_density") : _zero),
    _has_pke(isParamValid("pke_power_var")),
    _pke_qv(_has_pke ? coupledScalarValue("pke_power_var") : _zero),
    _power_shape_function(
        isParamValid("power_shape_function") ? &getFunction("power_shape_function") : NULL)
{
  // Input sanity check: cannot have both 'power_density' and 'pke_power_var'
  if (_has_qv && _has_pke)
    mooseError(
        "'power_density' and 'pke_power_var' cannot be both provided in 'INSFEFluidEnergyKernel'.");

  if (_has_pke && (_power_shape_function == NULL))
    mooseError("'power_shape_function' is required if 'pke_power_var' is provided in "
               "'INSFEFluidEnergyKernel'.");
}

Real
INSFEFluidEnergyKernel::computeQpResidual()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  Real porosity_elem = (_has_porosity_elem || _has_porosity) ? _porosity_elem[_qp] : 1.0;
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real enthalpy = _eos.h_from_p_T(_pressure[_qp], _u[_qp]);

  // Residual weak form terms: convection + diffusion + volumetric heat
  Real convective_part = _conservative_form
                             ? -_rho[_qp] * enthalpy * vec_vel * _grad_test[_i][_qp]
                             : _rho[_qp] * _cp[_qp] * vec_vel * _grad_u[_qp] * _test[_i][_qp];

  // heat source, one of the two terms, or both two terms will be zero
  Real heat_source_part = -_qv[_qp] * _test[_i][_qp];
  if (_has_pke)
    heat_source_part +=
        -_pke_qv[0] * _power_shape_function->value(_t, _q_point[_qp]) * _test[_i][_qp];

  Real diffusion_part = porosity_elem * _k_elem[_qp] * _grad_u[_qp] * _grad_test[_i][_qp];

  // Residual strong form terms for SUPG contributions
  Real transient_supg = _bTransient ? porosity * _rho[_qp] * _cp[_qp] * _u_dot[_qp] : 0;
  Real convection_supg = _rho[_qp] * _cp[_qp] * vec_vel * _grad_u[_qp];
  Real heat_source_supg = -_qv[_qp];
  if (_has_pke)
    heat_source_supg += -_pke_qv[0] * _power_shape_function->value(_t, _q_point[_qp]);

  Real diffusion_supg = -porosity_elem * _k_elem[_qp] * _second_u[_qp].tr();

  // assemble residuals
  Real normal_part = convective_part + heat_source_part + diffusion_part;
  Real supg_part = (transient_supg + convection_supg + heat_source_supg + diffusion_supg) *
                   _taue[_qp] * _vel_elem * _grad_test[_i][_qp];

  return normal_part + supg_part;
}

Real
INSFEFluidEnergyKernel::computeQpJacobian()
{
  Real porosity = _has_porosity ? _porosity[_qp] : 1.0;
  Real porosity_elem = (_has_porosity_elem || _has_porosity) ? _porosity_elem[_qp] : 1.0;
  Real rho, drho_dp, drho_dT;
  _eos.rho_from_p_T(_pressure[_qp], _u[_qp], rho, drho_dp, drho_dT);
  Real enthalpy = _eos.h_from_p_T(_pressure[_qp], _u[_qp]);
  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // convection part
  Real convection_part = _conservative_form
                             ? -(_rho[_qp] * _cp[_qp] + enthalpy * drho_dT) * vec_vel *
                                   _phi[_j][_qp] * _grad_test[_i][_qp]
                             : _cp[_qp] * vec_vel *
                                   (drho_dT * _grad_u[_qp] + _rho[_qp] * _grad_phi[_j][_qp]) *
                                   _test[_i][_qp];

  // diffusion part
  Real diffusion_part = porosity_elem * _k_elem[_qp] * _grad_phi[_j][_qp] * _grad_test[_i][_qp];

  Real normal_part = convection_part + diffusion_part;

  // SUPG contributions (only transient and convection part, ignore diffusion part)
  Real transient_supg =
      _bTransient ? porosity * _rho[_qp] * _cp[_qp] * _du_dot_du[_qp] * _phi[_j][_qp] : 0;
  Real convection_supg = _rho[_qp] * _cp[_qp] * vec_vel * _grad_phi[_j][_qp];
  Real supg_part =
      _taue[_qp] * _vel_elem * _grad_test[_i][_qp] * (transient_supg + convection_supg);

  // Assembly total residuals
  return normal_part + supg_part;
}

Real
INSFEFluidEnergyKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  unsigned m = this->mapVarNumber(jvar);

  RealVectorValue vec_vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  Real enthalpy = _eos.h_from_p_T(_pressure[_qp], _u[_qp]);

  Real jac = 0.;
  switch (m)
  {
    case 1:
    case 2:
    case 3:
      // convection weak form part
      if (_conservative_form)
        jac -= _rho[_qp] * _phi[_j][_qp] * enthalpy * _grad_test[_i][_qp](m - 1);
      else
        jac += _rho[_qp] * _cp[_qp] * _phi[_j][_qp] * _grad_u[_qp](m - 1) * _test[_i][_qp];
      // convection supg part
      jac += _rho[_qp] * _cp[_qp] * _phi[_j][_qp] * _grad_u[_qp](m - 1) * _taue[_qp] * _vel_elem *
             _grad_test[_i][_qp];
      break;

    default:
      jac = 0.0;
  }

  return jac;
}
