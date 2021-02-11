//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PressureEqualsDensityMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesTestApp", PressureEqualsDensityMaterial);

InputParameters
PressureEqualsDensityMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredCoupledVar(NS::density, "The density");
  params.addRequiredCoupledVar(NS::momentum_x, "The momentum in the x direction");
  params.addCoupledVar(NS::momentum_y, 0, "The momentum in the y direction");
  params.addCoupledVar(NS::momentum_z, 0, "The momentum in the z direction");
  params.addRequiredCoupledVar(NS::total_energy_density,
                               "The total energy density, e.g. Et or rho_et");
  return params;
}

PressureEqualsDensityMaterial::PressureEqualsDensityMaterial(const InputParameters & parameters)
  : Material(parameters),
    _rho(adCoupledValue(NS::density)),
    _grad_rho(adCoupledGradient(NS::density)),
    _rho_u(adCoupledValue(NS::momentum_x)),
    _rho_v(adCoupledValue(NS::momentum_y)),
    _rho_w(adCoupledValue(NS::momentum_z)),
    _rho_et(adCoupledValue(NS::total_energy_density)),
    _velocity(declareADProperty<RealVectorValue>(NS::velocity)),
    _grad_p(declareADProperty<RealVectorValue>(NS::grad(NS::pressure))),
    _rho_ht(declareADProperty<Real>(NS::total_enthalpy_density))
{
}

void
PressureEqualsDensityMaterial::computeQpProperties()
{
  _velocity[_qp] =
      ADRealVectorValue(_rho_u[_qp] / _rho[_qp], _rho_v[_qp] / _rho[_qp], _rho_w[_qp] / _rho[_qp]);

  // This is our dummy relationship between rho and p
  const auto p = _rho[_qp];
  _grad_p[_qp] = _grad_rho[_qp];

  _rho_ht[_qp] = _rho_et[_qp] + p;
}
