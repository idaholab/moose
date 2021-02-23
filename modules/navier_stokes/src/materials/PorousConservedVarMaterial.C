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
    _var_rho_ud(adCoupledValue(NS::superficial_momentum_x)),
    _var_rho_vd(isCoupled(NS::superficial_momentum_y) ? adCoupledValue(NS::superficial_momentum_y)
                                                      : _ad_zero),
    _var_rho_wd(isCoupled(NS::superficial_momentum_z) ? adCoupledValue(NS::superficial_momentum_z)
                                                      : _ad_zero),
    _var_total_energy_density(adCoupledValue(NS::total_energy_density)),
    _epsilon(getMaterialProperty<Real>(NS::porosity)),
    _rho(declareADProperty<Real>(NS::density)),
    _mass_flux(declareADProperty<RealVectorValue>(NS::mass_flux)),
    _total_energy_density(declareADProperty<Real>(NS::total_energy_density)),
    _velocity(declareADProperty<RealVectorValue>(NS::velocity)),
    _vel_x(declareADProperty<Real>(NS::velocity_x)),
    _vel_y(declareADProperty<Real>(NS::velocity_y)),
    _vel_z(declareADProperty<Real>(NS::velocity_z)),
    _v(declareADProperty<Real>(NS::v)),
    _specific_internal_energy(declareADProperty<Real>(NS::specific_internal_energy)),
    _pressure(declareADProperty<Real>(NS::pressure)),
    _specific_total_enthalpy(declareADProperty<Real>(NS::specific_total_enthalpy))
{
}

void
PorousConservedVarMaterial::computeQpProperties()
{
  _rho[_qp] = _var_rho[_qp];
  _mass_flux[_qp] = {_var_rho_ud[_qp], _var_rho_vd[_qp], _var_rho_wd[_qp]};
  _total_energy_density[_qp] = _var_total_energy_density[_qp];

  _velocity[_qp] = _mass_flux[_qp] / _rho[_qp] / _epsilon[_qp];
  _vel_x[_qp] = _velocity[_qp](0);
  _vel_y[_qp] = _velocity[_qp](1);
  _vel_z[_qp] = _velocity[_qp](2);

  _v[_qp] = 1 / _rho[_qp];

  _specific_internal_energy[_qp] =
      _total_energy_density[_qp] / _rho[_qp] - (_velocity[_qp] * _velocity[_qp]) / 2;

  _pressure[_qp] = _fluid.p_from_v_e(_v[_qp], _specific_internal_energy[_qp]);

  _specific_total_enthalpy[_qp] = (_total_energy_density[_qp] + _pressure[_qp]) / _rho[_qp];
}
