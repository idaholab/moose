//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "PorousConservedVarMaterial.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", PorousConservedVarMaterial);

InputParameters
PorousConservedVarMaterial::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::total_energy_density, "total fluid energy");
  params.addRequiredCoupledVar(NS::superficial_momentum_x, "The x-momentum times the porosity");
  params.addCoupledVar(NS::superficial_momentum_y, "The y-momentum times the porosity");
  params.addCoupledVar(NS::superficial_momentum_z, "The z-momentum times the porosity");
  params.addClassDescription("Provides access to variables for a conserved variable set "
                             "of density, total fluid energy, and momentum");
  params.addRequiredParam<MaterialPropertyName>(NS::porosity, "the porosity");
  return params;
}

PorousConservedVarMaterial::PorousConservedVarMaterial(const InputParameters & params)
  : Material(params),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _var_rho(adCoupledValue(NS::density)),
    _var_grad_rho(adCoupledGradient(NS::density)),
    _var_rho_ud(adCoupledValue(NS::superficial_momentum_x)),
    _var_rho_vd(isCoupled(NS::superficial_momentum_y) ? adCoupledValue(NS::superficial_momentum_y)
                                                      : _ad_zero),
    _var_rho_wd(isCoupled(NS::superficial_momentum_z) ? adCoupledValue(NS::superficial_momentum_z)
                                                      : _ad_zero),
    _var_grad_rho_ud(adCoupledGradient(NS::superficial_momentum_x)),
    _var_grad_rho_vd(isCoupled(NS::superficial_momentum_y)
                         ? adCoupledGradient(NS::superficial_momentum_y)
                         : _ad_grad_zero),
    _var_grad_rho_wd(isCoupled(NS::superficial_momentum_z)
                         ? adCoupledGradient(NS::superficial_momentum_z)
                         : _ad_grad_zero),
    _var_total_energy_density(adCoupledValue(NS::total_energy_density)),
    _var_grad_rho_et(adCoupledGradient(NS::total_energy_density)),
    _epsilon(getMaterialProperty<Real>(NS::porosity)),
    _rho(declareADProperty<Real>(NS::density)),
    _superficial_rho(declareADProperty<Real>(NS::superficial_density)),
    _mass_flux(declareADProperty<RealVectorValue>(NS::mass_flux)),
    _momentum(declareADProperty<RealVectorValue>(NS::momentum)),
    _total_energy_density(declareADProperty<Real>(NS::total_energy_density)),
    _velocity(declareADProperty<RealVectorValue>(NS::velocity)),
    _speed(declareADProperty<Real>(NS::speed)),
    _superficial_velocity(declareADProperty<RealVectorValue>(NS::superficial_velocity)),
    _sup_vel_x(declareADProperty<Real>(NS::superficial_velocity_x)),
    _sup_vel_y(declareADProperty<Real>(NS::superficial_velocity_y)),
    _sup_vel_z(declareADProperty<Real>(NS::superficial_velocity_z)),
    _grad_sup_vel_x(declareADProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_x))),
    _grad_sup_vel_y(declareADProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_y))),
    _grad_sup_vel_z(declareADProperty<RealVectorValue>(NS::grad(NS::superficial_velocity_z))),
    _vel_x(declareADProperty<Real>(NS::velocity_x)),
    _vel_y(declareADProperty<Real>(NS::velocity_y)),
    _vel_z(declareADProperty<Real>(NS::velocity_z)),
    _rhou(declareADProperty<Real>(NS::momentum_x)),
    _rhov(declareADProperty<Real>(NS::momentum_y)),
    _rhow(declareADProperty<Real>(NS::momentum_z)),
    _v(declareADProperty<Real>(NS::v)),
    _specific_internal_energy(declareADProperty<Real>(NS::specific_internal_energy)),
    _pressure(declareADProperty<Real>(NS::pressure)),
    _grad_pressure(declareADProperty<RealVectorValue>(NS::grad(NS::pressure))),
    _specific_total_enthalpy(declareADProperty<Real>(NS::specific_total_enthalpy)),
    _rho_ht(declareADProperty<Real>(NS::total_enthalpy_density)),
    _superficial_rho_et(declareADProperty<Real>(NS::superficial_total_energy_density)),
    _superficial_rho_ht(declareADProperty<Real>(NS::superficial_total_enthalpy_density)),
    _T_fluid(declareADProperty<Real>(NS::T_fluid)),
    _grad_T_fluid(declareADProperty<RealVectorValue>(NS::grad(NS::T_fluid)))
{
}

ADReal
PorousConservedVarMaterial::computeSpeed() const
{
  // if the velocity is zero, then the norm function call fails because AD tries to calculate the
  // derivatives which causes a divide by zero - because d/dx(sqrt(f(x))) = 1/2/sqrt(f(x))*df/dx.
  // So add a bit of noise to avoid this failure mode.
  if ((MooseUtils::absoluteFuzzyEqual(_velocity[_qp](0), 0)) &&
      (MooseUtils::absoluteFuzzyEqual(_velocity[_qp](1), 0)) &&
      (MooseUtils::absoluteFuzzyEqual(_velocity[_qp](2), 0)))
    return 1e-42;

  return _velocity[_qp].norm();
}

void
PorousConservedVarMaterial::computeQpProperties()
{
  _rho[_qp] = _var_rho[_qp];
  _superficial_rho[_qp] = _rho[_qp] * _epsilon[_qp];
  _mass_flux[_qp] = {_var_rho_ud[_qp], _var_rho_vd[_qp], _var_rho_wd[_qp]};
  _momentum[_qp] = _mass_flux[_qp] / _epsilon[_qp];
  _total_energy_density[_qp] = _var_total_energy_density[_qp];
  _superficial_rho_et[_qp] = _epsilon[_qp] * _total_energy_density[_qp];

  _superficial_velocity[_qp] = _mass_flux[_qp] / _rho[_qp];
  _sup_vel_x[_qp] = _superficial_velocity[_qp](0);
  _sup_vel_y[_qp] = _superficial_velocity[_qp](1);
  _sup_vel_z[_qp] = _superficial_velocity[_qp](2);
  _grad_sup_vel_x[_qp] = _var_grad_rho_ud[_qp] / _rho[_qp] -
                         _var_rho_ud[_qp] / (_rho[_qp] * _rho[_qp]) * _var_grad_rho[_qp];
  _grad_sup_vel_y[_qp] = _var_grad_rho_vd[_qp] / _rho[_qp] -
                         _var_rho_vd[_qp] / (_rho[_qp] * _rho[_qp]) * _var_grad_rho[_qp];
  _grad_sup_vel_z[_qp] = _var_grad_rho_wd[_qp] / _rho[_qp] -
                         _var_rho_wd[_qp] / (_rho[_qp] * _rho[_qp]) * _var_grad_rho[_qp];

  _velocity[_qp] = _superficial_velocity[_qp] / _epsilon[_qp];
  _speed[_qp] = computeSpeed();
  _vel_x[_qp] = _velocity[_qp](0);
  _vel_y[_qp] = _velocity[_qp](1);
  _vel_z[_qp] = _velocity[_qp](2);
  const auto grad_vel_x = _grad_sup_vel_x[_qp] / _epsilon[_qp];
  const auto grad_vel_y = _grad_sup_vel_y[_qp] / _epsilon[_qp];
  const auto grad_vel_z = _grad_sup_vel_z[_qp] / _epsilon[_qp];
  _rhou[_qp] = _vel_x[_qp] * _rho[_qp];
  _rhov[_qp] = _vel_y[_qp] * _rho[_qp];
  _rhow[_qp] = _vel_z[_qp] * _rho[_qp];

  _v[_qp] = 1 / _rho[_qp];
  const auto grad_v = (-1. / (_rho[_qp] * _rho[_qp])) * _var_grad_rho[_qp];

  _specific_internal_energy[_qp] =
      _total_energy_density[_qp] / _rho[_qp] - (_velocity[_qp] * _velocity[_qp]) / 2;
  const auto grad_e =
      _var_grad_rho_et[_qp] / _rho[_qp] -
      _total_energy_density[_qp] / (_rho[_qp] * _rho[_qp]) * _var_grad_rho[_qp] -
      (_vel_x[_qp] * grad_vel_x + _vel_y[_qp] * grad_vel_y + _vel_z[_qp] * grad_vel_z);

  Real dp_dv, dp_de, dummy_p;
  _pressure[_qp] = _fluid.p_from_v_e(_v[_qp], _specific_internal_energy[_qp]);
  _fluid.p_from_v_e(_v[_qp].value(), _specific_internal_energy[_qp].value(), dummy_p, dp_dv, dp_de);
  _grad_pressure[_qp] = dp_dv * grad_v + dp_de * grad_e;

  _specific_total_enthalpy[_qp] = (_total_energy_density[_qp] + _pressure[_qp]) / _rho[_qp];
  _rho_ht[_qp] = _specific_total_enthalpy[_qp] * _rho[_qp];
  _superficial_rho_ht[_qp] = _rho_ht[_qp] * _epsilon[_qp];

  Real dT_dv, dT_de, dummy_T;
  _T_fluid[_qp] = _fluid.T_from_v_e(_v[_qp], _specific_internal_energy[_qp]);
  _fluid.T_from_v_e(_v[_qp].value(), _specific_internal_energy[_qp].value(), dummy_T, dT_dv, dT_de);
  _grad_T_fluid[_qp] = dT_dv * grad_v + dT_de * grad_e;
}
