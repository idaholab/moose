//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "ConservedVarMaterial.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

// MOOSE includes
#include "AuxiliarySystem.h"

registerMooseObject("NavierStokesApp", ConservedVarMaterial);

InputParameters
ConservedVarMaterial::validParams()
{
  auto params = VarMaterialBase::validParams();
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::total_energy_density, "total fluid energy");
  params.addRequiredCoupledVar(NS::momentum_x, "rhou");
  params.addCoupledVar(NS::momentum_y, "rhov");
  params.addCoupledVar(NS::momentum_z, "rhow");
  params.addCoupledVar(
      NS::T_fluid,
      "optionally use for approximating second spatial derivatives for stabilization");
  params.addClassDescription("Provides access to variables for a conserved variable set "
    "of density, total fluid energy, and momentum");
  params.addParam<MaterialPropertyName>(NS::porosity, "the porosity");
  return params;
}

using MetaPhysicL::raw_value;

ConservedVarMaterial::ConservedVarMaterial(const InputParameters & params)
  : VarMaterialBase(params),
    _var_rho(adCoupledValue(NS::density)),
    _var_total_energy_density(adCoupledValue(NS::total_energy_density)),
    _var_rho_u(adCoupledValue(NS::momentum_x)),
    _var_rho_v(isCoupled(NS::momentum_y) ? adCoupledValue(NS::momentum_y) : _ad_zero),
    _var_rho_w(isCoupled(NS::momentum_z) ? adCoupledValue(NS::momentum_z) : _ad_zero),
    _var_grad_rho(adCoupledGradient(NS::density)),
    _var_grad_rho_et(adCoupledGradient(NS::total_energy_density)),
    _var_grad_rho_u(adCoupledGradient(NS::momentum_x)),
    _var_grad_rho_v(isCoupled(NS::momentum_y) ? adCoupledGradient(NS::momentum_y) : _ad_grad_zero),
    _var_grad_rho_w(isCoupled(NS::momentum_z) ? adCoupledGradient(NS::momentum_z) : _ad_grad_zero),
    _var_grad_grad_rho(adCoupledSecond(NS::density)),
    _var_grad_grad_rho_u(adCoupledSecond(NS::momentum_x)),
    _var_grad_grad_rho_v(isCoupled(NS::momentum_y) ? adCoupledSecond(NS::momentum_y) : _ad_second_zero),
    _var_grad_grad_rho_w(isCoupled(NS::momentum_z) ? adCoupledSecond(NS::momentum_z) : _ad_second_zero),
    _var_rho_dot(_is_transient ? adCoupledDot(NS::density) : _ad_zero),
    _var_rho_et_dot(_is_transient ? adCoupledDot(NS::total_energy_density) : _ad_zero),
    _var_rho_u_dot(_is_transient ? adCoupledDot(NS::momentum_x) : _ad_zero),
    _var_rho_v_dot(isCoupled(NS::momentum_y) && _is_transient ? adCoupledDot(NS::momentum_y)
                                                          : _ad_zero),
    _var_rho_w_dot(isCoupled(NS::momentum_z) && _is_transient ? adCoupledDot(NS::momentum_z)
                                                          : _ad_zero),
    _var_grad_grad_T_fluid(isCoupled(nms::T_fluid) ? adCoupledSecond(nms::T_fluid)
                                                   : _ad_second_zero)
{
  warnAuxiliaryVariables();
}

bool
ConservedVarMaterial::coupledAuxiliaryVariables() const
{
  auto & sys = _fe_problem.getAuxiliarySystem();
  return sys.hasVariable(NS::density) || sys.hasVariable(NS::total_energy_density) || sys.hasVariable(NS::momentum_x) ||
      sys.hasVariable(NS::momentum_y) || sys.hasVariable(NS::momentum_z);
}

void
ConservedVarMaterial::setNonlinearProperties()
{
  _rho[_qp] = _var_rho[_qp];
  _total_energy_density[_qp] = _var_total_energy_density[_qp];
  _momentum[_qp] = {_var_rho_u[_qp], _var_rho_v[_qp], _var_rho_w[_qp]};

  _grad_rho[_qp] = _var_grad_rho[_qp];
  _grad_rho_et[_qp] = _var_grad_rho_et[_qp];
  _grad_rho_u[_qp] = _var_grad_rho_u[_qp];
  _grad_rho_v[_qp] = _var_grad_rho_v[_qp];
  _grad_rho_w[_qp] = _var_grad_rho_w[_qp];

  _drho_dt[_qp] = _var_rho_dot[_qp];
  _drho_et_dt[_qp] = _var_rho_et_dot[_qp];
  _drho_u_dt[_qp] = _var_rho_u_dot[_qp];
  _drho_v_dt[_qp] = _var_rho_v_dot[_qp];
  _drho_w_dt[_qp] = _var_rho_w_dot[_qp];

  // TODO: Figure out how to approximate second spatial derivatives better
  //
  // The Quasi-linear (strong) residual part of the stabilization term has effectively a laplace
  // operator acting on T_fluid - so we need the second spatial derivatives of temperature. But if
  // the nonlinear variable is not T_fluid (e.g. energy instead) we get temperature via the fluid
  // properties system which doesn't support second derivatives. So with the current state of
  // things, we will not be able to get the full accuracy for the stabilization term - unless you
  // are using PrimitiveVarMaterial or MixedVarMaterial. However, when using linear
  // elements, the aux variables would have been giving us zero for the second derivatives anyway
  // when using regular cartesian grids.  So I would say that the loss of accuracy here is fine
  // for now. And the solution in the future will be to support some sort of approximation or
  // figure out how to get second derivs of fluid properties.
  _grad_grad_T_fluid[_qp] = _var_grad_grad_T_fluid[_qp];
}

void
ConservedVarMaterial::computeQpProperties()
{
  setNonlinearProperties();

  _velocity[_qp] = _momentum[_qp] / _rho[_qp];
  _speed[_qp] = computeSpeed();

  _v[_qp] = 1 / _rho[_qp];
  auto grad_vol = -1 / (_rho[_qp] * _rho[_qp]) * _grad_rho[_qp];
  _grad_vel_x[_qp] = _v[_qp] * _grad_rho_u[_qp] + _momentum[_qp](0) * grad_vol;
  _grad_vel_y[_qp] = _v[_qp] * _grad_rho_v[_qp] + _momentum[_qp](1) * grad_vol;
  _grad_vel_z[_qp] = _v[_qp] * _grad_rho_w[_qp] + _momentum[_qp](2) * grad_vol;

  _grad_grad_vel_x[_qp] = (_var_grad_grad_rho_u[_qp] - outer_product(_grad_rho[_qp], _grad_vel_x[_qp]) -
    outer_product(_grad_vel_x[_qp], _grad_rho[_qp]) - _velocity[_qp](0) * _var_grad_grad_rho[_qp]) / _rho[_qp];
  _grad_grad_vel_y[_qp] = (_var_grad_grad_rho_v[_qp] - outer_product(_grad_rho[_qp], _grad_vel_y[_qp]) -
    outer_product(_grad_vel_y[_qp], _grad_rho[_qp]) - _velocity[_qp](1) * _var_grad_grad_rho[_qp]) / _rho[_qp];
  _grad_grad_vel_z[_qp] = (_var_grad_grad_rho_w[_qp] - outer_product(_grad_rho[_qp], _grad_vel_z[_qp]) -
    outer_product(_grad_vel_z[_qp], _grad_rho[_qp]) - _velocity[_qp](2) * _var_grad_grad_rho[_qp]) / _rho[_qp];

  _specific_internal_energy[_qp] = _total_energy_density[_qp] / _rho[_qp] - (_velocity[_qp] * _velocity[_qp]) / 2;

  Real dTdvol = 0;
  Real dTde = 0;
  ADReal dpdvol = 0;
  ADReal dpde = 0;
  _T_fluid[_qp] = _fluid.T_from_v_e(_v[_qp], _specific_internal_energy[_qp]);
  _fluid.p_from_v_e(_v[_qp], _specific_internal_energy[_qp], _pressure[_qp], dpdvol, dpde);

  _specific_total_enthalpy[_qp] = (_total_energy_density[_qp] + _pressure[_qp]) / _rho[_qp];
  _total_enthalpy_density[_qp] = _rho[_qp] * _specific_total_enthalpy[_qp];

  auto grad_e = _grad_rho_et[_qp] * _v[_qp] + _total_energy_density[_qp] * grad_vol - _velocity[_qp](0) * _grad_vel_x[_qp] -
                _velocity[_qp](1) * _grad_vel_y[_qp] - _velocity[_qp](2) * _grad_vel_z[_qp];

  _grad_T_fluid[_qp] = grad_vol * dTdvol + grad_e * dTde;
  _grad_pressure[_qp] = grad_vol * dpdvol + grad_e * dpde;

  auto dvelx_dt = (_drho_u_dt[_qp] - _velocity[_qp](0) * _drho_dt[_qp]) / _rho[_qp];
  auto dvely_dt = (_drho_v_dt[_qp] - _velocity[_qp](1) * _drho_dt[_qp]) / _rho[_qp];
  auto dvelz_dt = (_drho_w_dt[_qp] - _velocity[_qp](2) * _drho_dt[_qp]) / _rho[_qp];
  auto de_dt = _drho_et_dt[_qp] / _rho[_qp] - _total_energy_density[_qp] / (_rho[_qp] * _rho[_qp]) * _drho_dt[_qp] -
    (_velocity[_qp](0) * dvelx_dt + _velocity[_qp](1) * dvely_dt + _velocity[_qp](2) * dvelz_dt);
  auto dvol_dt = -1 / (_rho[_qp] * _rho[_qp]) * _drho_dt[_qp];
  _dT_dt[_qp] = dTde * de_dt + dTdvol * dvol_dt;
}
