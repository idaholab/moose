//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelRadiative.h"
#include "libmesh/utility.h"

registerMooseObject("HeatConductionApp", GapFluxModelRadiative);

InputParameters
GapFluxModelRadiative::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();
  params.addClassDescription("Gap flux demonstration model for radiative heat conductance");
  params.addRequiredCoupledVar("temperature", "The name of the temperature variable");
  params.addParam<Real>("sigma", 5.670373e-8, "Stefan-Boltzmann constant");
  params.addRequiredRangeCheckedParam<MaterialPropertyName>(
      "primary_emissivity",
      "primary_emissivity>0 && primary_emissivity<=1",
      "Primary surface emissivity");
  params.addRequiredRangeCheckedParam<MaterialPropertyName>(
      "secondary_emissivity",
      "secondary_emissivity>0 && secondary_emissivity<=1",
      "Secondary surface emissivity");
  return params;
}

GapFluxModelRadiative::GapFluxModelRadiative(const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _primary_T(adCoupledNeighborValue("temperature")),
    _secondary_T(adCoupledValue("temperature")),
    _sigma(getParam<Real>("sigma")),
    _primary_emissivity(getNeighborADMaterialProperty<Real>("primary_emissivity")),
    _secondary_emissivity(getADMaterialProperty<Real>("secondary_emissivity"))
{
}

ADReal
GapFluxModelRadiative::computeFlux() const
{
  const auto Fe = 1.0 / (1.0 / _primary_emissivity[_qp] + 1.0 / _secondary_emissivity[_qp] - 1.0);
  return _sigma * Fe * (Utility::pow<4>(_primary_T[_qp]) - Utility::pow<4>(_secondary_T[_qp]));
}
