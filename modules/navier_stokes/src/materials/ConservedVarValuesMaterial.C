//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "ConservedVarValuesMaterial.h"
#include "NS.h"
#include "NavierStokesMethods.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", ConservedVarValuesMaterial);

InputParameters
ConservedVarValuesMaterial::validParams()
{
  auto params = Material::validParams();
  params.addRequiredParam<UserObjectName>(NS::fluid, "fluid userobject");
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::total_energy_density, "total fluid energy");
  params.addRequiredCoupledVar(NS::momentum_x, "The x-momentum");
  params.addCoupledVar(NS::momentum_y, "The y-momentum");
  params.addCoupledVar(NS::momentum_z, "The z-momentum");
  params.addClassDescription("Provides access to variables for a conserved variable set "
                             "of density, total fluid energy, and momentum");
  return params;
}

ConservedVarValuesMaterial::ConservedVarValuesMaterial(const InputParameters & params)
  : Material(params),
    _fluid(UserObjectInterface::getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _var_rho(adCoupledValue(NS::density)),
    _var_rho_u(adCoupledValue(NS::momentum_x)),
    _var_rho_v(isCoupled(NS::momentum_y) ? adCoupledValue(NS::momentum_y) : _ad_zero),
    _var_rho_w(isCoupled(NS::momentum_z) ? adCoupledValue(NS::momentum_z) : _ad_zero),
    _var_total_energy_density(adCoupledValue(NS::total_energy_density)),
    _rho(declareADProperty<Real>(NS::density)),
    _mass_flux(declareADProperty<RealVectorValue>(NS::mass_flux)),
    _momentum(declareADProperty<RealVectorValue>(NS::momentum)),
    _total_energy_density(declareADProperty<Real>(NS::total_energy_density)),
    _velocity(declareADProperty<RealVectorValue>(NS::velocity)),
    _speed(declareADProperty<Real>(NS::speed)),
    _vel_x(declareADProperty<Real>(NS::velocity_x)),
    _vel_y(declareADProperty<Real>(NS::velocity_y)),
    _vel_z(declareADProperty<Real>(NS::velocity_z)),
    _rhou(declareADProperty<Real>(NS::momentum_x)),
    _rhov(declareADProperty<Real>(NS::momentum_y)),
    _rhow(declareADProperty<Real>(NS::momentum_z)),
    _v(declareADProperty<Real>(NS::v)),
    _specific_internal_energy(declareADProperty<Real>(NS::specific_internal_energy)),
    _pressure(declareADProperty<Real>(NS::pressure)),
    _specific_total_enthalpy(declareADProperty<Real>(NS::specific_total_enthalpy)),
    _rho_ht(declareADProperty<Real>(NS::total_enthalpy_density)),
    _T_fluid(declareADProperty<Real>(NS::T_fluid))
{
}

void
ConservedVarValuesMaterial::computeQpProperties()
{
  _rho[_qp] = _var_rho[_qp];
  _mass_flux[_qp] = {_var_rho_u[_qp], _var_rho_v[_qp], _var_rho_w[_qp]};
  _momentum[_qp] = _mass_flux[_qp];
  _total_energy_density[_qp] = _var_total_energy_density[_qp];

  _velocity[_qp] = _mass_flux[_qp] / _rho[_qp];
  _speed[_qp] = NS::computeSpeed(_velocity[_qp]);
  _vel_x[_qp] = _velocity[_qp](0);
  _vel_y[_qp] = _velocity[_qp](1);
  _vel_z[_qp] = _velocity[_qp](2);
  _rhou[_qp] = _vel_x[_qp] * _rho[_qp];
  _rhov[_qp] = _vel_y[_qp] * _rho[_qp];
  _rhow[_qp] = _vel_z[_qp] * _rho[_qp];

  _v[_qp] = 1 / _rho[_qp];

  _specific_internal_energy[_qp] =
      _total_energy_density[_qp] / _rho[_qp] - (_velocity[_qp] * _velocity[_qp]) / 2;

  _pressure[_qp] = _fluid.p_from_v_e(_v[_qp], _specific_internal_energy[_qp]);

  _specific_total_enthalpy[_qp] = (_total_energy_density[_qp] + _pressure[_qp]) / _rho[_qp];
  _rho_ht[_qp] = _specific_total_enthalpy[_qp] * _rho[_qp];
  _T_fluid[_qp] = _fluid.T_from_v_e(_v[_qp], _specific_internal_energy[_qp]);
}
