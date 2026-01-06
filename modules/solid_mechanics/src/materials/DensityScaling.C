//* This file is part of the MOOSE framework
//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DensityScaling.h"
#include "libmesh/utility.h"

registerMooseObject("SolidMechanicsApp", DensityScaling);

InputParameters
DensityScaling::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Computes the inertial density needed to enable stable explicit time-stepping using the "
      "desired_time_step in solid-mechanics problems.  Note that if this inertial density is used "
      "in input files (for instance, in the mass matrix) it will impact the dynamics of the "
      "system, largely eliminating high-frequency oscillations.  Hence, use with caution.");
  params.addRequiredParam<MaterialPropertyName>(
      "true_density",
      "Name of Material Property defining the true inertial density of the material.");
  params.addRequiredParam<MaterialPropertyName>(
      "scaled_density", "Name of the Property that this Material will compute.");
  params.addRequiredParam<Real>("desired_time_step", "The desired time step.");
  params.addParam<bool>(
      "additive_contribution",
      false,
      "When false, this Material computes the inertial density needed to enable time-stepping with "
      "given desired_time_step.  When true, the Material computes max(inertial_density_needed - "
      "true_density, 0), ie, just the portion that needs to be added to the true density to enable "
      "the desired time stepping.");
  params.addParamNamesToGroup("additive_contribution", "Advanced");
  params.addRangeCheckedParam<Real>(
      "safety_factor",
      0.7,
      "(safety_factor>0) & (safety_factor<=1)",
      "The scaled density that this Material produces will potentially allow stable time-step "
      "sizes of desired_time_step / safety_factor.  In practice, however, using such a time step "
      "might result in instabilities, because of time-step lagging and the approximate critical "
      "time-step formula used by this Material.  Hence, safety_factor allows for a safety margin.");
  return params;
}

DensityScaling::DensityScaling(const InputParameters & parameters)
  : Material(parameters),
    _desired_time_step(getParam<Real>("desired_time_step")),
    _additive_contribution(getParam<bool>("additive_contribution")),
    _density_scaled(declareProperty<Real>(getParam<MaterialPropertyName>("scaled_density"))),
    _material_density(getMaterialProperty<Real>("true_density")),
    _sqrt_effective_stiffness(getMaterialPropertyByName<Real>("effective_stiffness")),
    _safety_factor(getParam<Real>("safety_factor"))
{
}

void
DensityScaling::computeQpProperties()
{
  const Real stable_density = Utility::pow<2>(_sqrt_effective_stiffness[_qp] * _desired_time_step /
                                              _safety_factor / _current_elem->hmin());

  if (_additive_contribution)
    _density_scaled[_qp] = std::max(stable_density - _material_density[_qp], 0.0);
  else
    _density_scaled[_qp] = std::max(stable_density, _material_density[_qp]);
}
