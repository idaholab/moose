//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalizedHeatStructureResidualNorm.h"
#include "ThermalSolidProperties.h"

registerMooseObject("ThermalHydraulicsApp", NormalizedHeatStructureResidualNorm);

InputParameters
NormalizedHeatStructureResidualNorm::validParams()
{
  InputParameters params = DiscreteVariableResidualNorm::validParams();

  params.addClassDescription("Computes a normalized residual norm for a heat structure.");

  params.addRequiredParam<Real>("T_ref", "Reference temperature [K]");
  params.addRequiredParam<UserObjectName>("solid_properties", "Solid properties object");
  params.addRequiredParam<PostprocessorName>("ref_elem_size",
                                             "Reference element size [units vary]");

  return params;
}

NormalizedHeatStructureResidualNorm::NormalizedHeatStructureResidualNorm(
    const InputParameters & parameters)
  : DiscreteVariableResidualNorm(parameters),
    _ref_elem_size(getPostprocessorValue("ref_elem_size")),
    _initialized(false)
{
}

void
NormalizedHeatStructureResidualNorm::initialize()
{
  DiscreteVariableResidualNorm::initialize();

  // This cannot be done in constructor or initialSetup() due to some solid
  // properties not being initialized yet.
  if (!_initialized)
  {
    _normalization = computeNormalization();
    _initialized = true;
  }
}

Real
NormalizedHeatStructureResidualNorm::computeNormalization() const
{
  const auto & sp = getUserObject<ThermalSolidProperties>("solid_properties");
  const auto T_ref = getParam<Real>("T_ref");
  const auto rho_ref = sp.rho_from_T(T_ref);
  const auto cp_ref = sp.cp_from_T(T_ref);

  return rho_ref * cp_ref * T_ref * _ref_elem_size;
}

Real
NormalizedHeatStructureResidualNorm::getValue() const
{
  return DiscreteVariableResidualNorm::getValue() / _normalization;
}
