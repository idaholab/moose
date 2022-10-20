//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "PorousMixedVarMaterial.h"
#include "NS.h"
#include "NavierStokesMethods.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", PorousMixedVarMaterial);

InputParameters
PorousMixedVarMaterial::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addRequiredCoupledVar(NS::pressure, "The pressure");
  params.addRequiredCoupledVar(NS::T_fluid, "The fluid temperature");
  params.addRequiredCoupledVar(NS::superficial_momentum_x, "The x-momentum times the porosity");
  params.addCoupledVar(NS::superficial_momentum_y, "The y-momentum times the porosity");
  params.addCoupledVar(NS::superficial_momentum_z, "The z-momentum times the porosity");
  params.addClassDescription("Provides access to variables for a primitive variable set "
                             "of pressure, temperature, and superficial velocity");
  params.addRequiredParam<MaterialPropertyName>(NS::porosity, "the porosity");
  return params;
}

PorousMixedVarMaterial::PorousMixedVarMaterial(const InputParameters & params)
  : Material(params),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    // mixed variables
    _var_pressure(adCoupledValue(NS::pressure)),
    _grad_var_pressure(adCoupledGradient(NS::pressure)),
    _pressure_dot(_is_transient ? adCoupledDot(NS::pressure) : _ad_zero),
    _var_T_fluid(adCoupledValue(NS::T_fluid)),
    _grad_var_T_fluid(adCoupledGradient(NS::T_fluid)),
    _T_fluid_dot(_is_transient ? adCoupledDot(NS::T_fluid) : _ad_zero),
    _var_sup_mom_x(adCoupledValue(NS::superficial_momentum_x)),
    _grad_var_sup_mom_x(adCoupledGradient(NS::superficial_momentum_x)),
    _var_sup_mom_y(isCoupled(NS::superficial_momentum_y)
                       ? adCoupledValue(NS::superficial_momentum_y)
                       : _ad_zero),
    _grad_var_sup_mom_y(isCoupled(NS::superficial_momentum_y)
                            ? adCoupledGradient(NS::superficial_momentum_y)
                            : _ad_grad_zero),
    _var_sup_mom_z(isCoupled(NS::superficial_momentum_z)
                       ? adCoupledValue(NS::superficial_momentum_z)
                       : _ad_zero),
    _grad_var_sup_mom_z(isCoupled(NS::superficial_momentum_z)
                            ? adCoupledGradient(NS::superficial_momentum_z)
                            : _ad_grad_zero),
    _var_sup_mom_x_dot(_is_transient ? adCoupledDot(NS::superficial_momentum_x) : _ad_zero),
    _var_sup_mom_y_dot((isCoupled(NS::superficial_momentum_y) && _is_transient)
                           ? adCoupledDot(NS::superficial_momentum_y)
                           : _ad_zero),
    _var_sup_mom_z_dot((isCoupled(NS::superficial_momentum_z) && _is_transient)
                           ? adCoupledDot(NS::superficial_momentum_z)
                           : _ad_zero),
    // porosity
    _epsilon(getMaterialProperty<Real>(NS::porosity)),
    // properties: primitives
    _pressure(declareADProperty<Real>(NS::pressure)),
    _grad_pressure(declareADProperty<RealVectorValue>(NS::grad(NS::pressure))),
    _T_fluid(declareADProperty<Real>(NS::T_fluid)),
    _grad_T_fluid(declareADProperty<RealVectorValue>(NS::grad(NS::T_fluid))),
    _sup_vel(declareADProperty<RealVectorValue>(NS::superficial_velocity)),
    _sup_vel_x(declareADProperty<Real>(NS::superficial_velocity_x)),
    _grad_sup_vel_x(declareADProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_x))),
    _sup_vel_y(declareADProperty<Real>(NS::superficial_velocity_y)),
    _grad_sup_vel_y(declareADProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_y))),
    _sup_vel_z(declareADProperty<Real>(NS::superficial_velocity_z)),
    _grad_sup_vel_z(declareADProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_z))),
    // properties: for viz
    _rho(declareADProperty<Real>(NS::density)),
    _sup_rho_dot(declareADProperty<Real>(NS::time_deriv(NS::superficial_density))),
    _velocity(declareADProperty<RealVectorValue>(NS::velocity)),
    _vel_x(declareADProperty<Real>(NS::velocity_x)),
    _vel_y(declareADProperty<Real>(NS::velocity_y)),
    _vel_z(declareADProperty<Real>(NS::velocity_z)),
    _sup_mom_x(declareADProperty<Real>(NS::superficial_momentum_x)),
    _sup_mom_y(declareADProperty<Real>(NS::superficial_momentum_y)),
    _sup_mom_z(declareADProperty<Real>(NS::superficial_momentum_z)),
    _sup_mom_x_dot(declareADProperty<Real>(NS::time_deriv(NS::superficial_momentum_x))),
    _sup_mom_y_dot(declareADProperty<Real>(NS::time_deriv(NS::superficial_momentum_y))),
    _sup_mom_z_dot(declareADProperty<Real>(NS::time_deriv(NS::superficial_momentum_z))),
    _sup_rho_et_dot(declareADProperty<Real>(NS::time_deriv(NS::superficial_total_energy_density))),
    _mom(declareADProperty<RealVectorValue>(NS::momentum)),
    _mom_x(declareADProperty<Real>(NS::momentum_x)),
    _mom_y(declareADProperty<Real>(NS::momentum_y)),
    _mom_z(declareADProperty<Real>(NS::momentum_z)),
    _speed(declareADProperty<Real>(NS::speed)),
    _rho_et(declareADProperty<Real>(NS::total_energy_density)),
    _e(declareADProperty<Real>(NS::specific_internal_energy)),
    _ht(declareADProperty<Real>(NS::specific_total_enthalpy))
{
  if (_mesh.dimension() >= 2 && !isCoupled(NS::superficial_momentum_y))
    mooseError("You must couple in a superficial y-momentum when solving 2D or 3D problems.");

  if (_mesh.dimension() >= 3 && !isCoupled(NS::superficial_momentum_z))
    mooseError("You must couple in a superficial z-momentum when solving 3D problems.");
}

void
PorousMixedVarMaterial::computeQpProperties()
{
  // Our primitive variable set
  _pressure[_qp] = _var_pressure[_qp];
  _grad_pressure[_qp] = _grad_var_pressure[_qp];
  _T_fluid[_qp] = _var_T_fluid[_qp];
  _grad_T_fluid[_qp] = _grad_var_T_fluid[_qp];
  const VectorValue<ADReal> superficial_momentum = {
      _var_sup_mom_x[_qp], _var_sup_mom_y[_qp], _var_sup_mom_z[_qp]};
  _sup_mom_x[_qp] = superficial_momentum(0);
  _sup_mom_y[_qp] = superficial_momentum(1);
  _sup_mom_z[_qp] = superficial_momentum(2);
  _sup_mom_x_dot[_qp] = _var_sup_mom_x_dot[_qp];
  _sup_mom_y_dot[_qp] = _var_sup_mom_y_dot[_qp];
  _sup_mom_z_dot[_qp] = _var_sup_mom_z_dot[_qp];

  ADReal drho_dp, drho_dT;
  _fluid.rho_from_p_T(_pressure[_qp], _T_fluid[_qp], _rho[_qp], drho_dp, drho_dT);
  const auto rho_dot = drho_dp * _pressure_dot[_qp] + drho_dT * _T_fluid_dot[_qp];
  const auto grad_rho = drho_dp * _grad_pressure[_qp] + drho_dT * _grad_T_fluid[_qp];
  _sup_rho_dot[_qp] = _epsilon[_qp] * rho_dot;

  _sup_vel[_qp] = superficial_momentum / _rho[_qp];
  _sup_vel_x[_qp] = _sup_vel[_qp](0);
  _sup_vel_y[_qp] = _sup_vel[_qp](1);
  _sup_vel_z[_qp] = _sup_vel[_qp](2);
  _grad_sup_vel_x[_qp] = _grad_var_sup_mom_x[_qp] / _rho[_qp] -
                         superficial_momentum(0) / (_rho[_qp] * _rho[_qp]) * grad_rho;
  _grad_sup_vel_y[_qp] = _grad_var_sup_mom_y[_qp] / _rho[_qp] -
                         superficial_momentum(1) / (_rho[_qp] * _rho[_qp]) * grad_rho;
  _grad_sup_vel_z[_qp] = _grad_var_sup_mom_z[_qp] / _rho[_qp] -
                         superficial_momentum(2) / (_rho[_qp] * _rho[_qp]) * grad_rho;
  const auto sup_vel_x_dot = _var_sup_mom_x_dot[_qp] / _rho[_qp] -
                             superficial_momentum(0) / (_rho[_qp] * _rho[_qp]) * rho_dot;
  const auto sup_vel_y_dot = _var_sup_mom_y_dot[_qp] / _rho[_qp] -
                             superficial_momentum(1) / (_rho[_qp] * _rho[_qp]) * rho_dot;
  const auto sup_vel_z_dot = _var_sup_mom_z_dot[_qp] / _rho[_qp] -
                             superficial_momentum(2) / (_rho[_qp] * _rho[_qp]) * rho_dot;

  _velocity[_qp] = _sup_vel[_qp] / _epsilon[_qp];
  _vel_x[_qp] = _velocity[_qp](0);
  _vel_y[_qp] = _velocity[_qp](1);
  _vel_z[_qp] = _velocity[_qp](2);

  const auto v = 1. / _rho[_qp];
  const auto v_dot = -rho_dot / (_rho[_qp] * _rho[_qp]);
  ADReal de_dT, de_dv;
  _fluid.e_from_T_v(_T_fluid[_qp], v, _e[_qp], de_dT, de_dv);
  const auto e_dot = de_dT * _T_fluid_dot[_qp] + de_dv * v_dot;
  const auto et = _e[_qp] + _velocity[_qp] * _velocity[_qp] / 2.;
  const auto velocity_dot =
      VectorValue<ADReal>(sup_vel_x_dot, sup_vel_y_dot, sup_vel_z_dot) / _epsilon[_qp];
  const auto et_dot = e_dot + _velocity[_qp] * velocity_dot;
  _sup_rho_et_dot[_qp] = _epsilon[_qp] * (rho_dot * et + et_dot * _rho[_qp]);

  _mom_x[_qp] = _sup_mom_x[_qp] / _epsilon[_qp];
  _mom_y[_qp] = _sup_mom_y[_qp] / _epsilon[_qp];
  _mom_z[_qp] = _sup_mom_z[_qp] / _epsilon[_qp];
  _mom[_qp] = {_mom_x[_qp], _mom_y[_qp], _mom_z[_qp]};

  _speed[_qp] = NS::computeSpeed(_velocity[_qp]);

  _rho_et[_qp] = _rho[_qp] * et;
  _ht[_qp] = et + _pressure[_qp] / _rho[_qp];
}
