//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelPressureDependentConduction.h"

registerMooseObject("HeatConductionApp", GapFluxModelPressureDependentConduction);

InputParameters
GapFluxModelPressureDependentConduction::validParams()
{
  InputParameters params = GapFluxModelBase::validParams();

  params.addClassDescription("Heat flux model across a closed gap to calculate the conductance "
                             "between two solid materials");
  params.addRequiredCoupledVar("temperature", "The name of the temperature variable");
  params.addRequiredCoupledVar("contact_pressure", "The name of the contact pressure variable");

  params.addParam<Real>(
      "scaling_coefficient",
      1.0,
      "The leading coefficient for the closed gap conductance value; used for tuning");
  params.addRequiredParam<MaterialPropertyName>(
      "primary_conductivity", "The thermal conductivity of the primary surface solid material");
  params.addRequiredParam<MaterialPropertyName>(
      "secondary_conductivity", "The thermal conductivity of the secondary surface solid material");
  params.addRequiredParam<MaterialPropertyName>(
      "primary_hardness", "The hardness value of the primary surface material");
  params.addRequiredParam<MaterialPropertyName>("secondary_hardness",
                                                "The hardness of the secondary surface material");

  return params;
}

GapFluxModelPressureDependentConduction::GapFluxModelPressureDependentConduction(
    const InputParameters & parameters)
  : GapFluxModelBase(parameters),
    _primary_T(adCoupledNeighborValue("temperature")),
    _secondary_T(adCoupledValue("temperature")),
    _contact_pressure(adCoupledLowerValue("contact_pressure")),
    _scaling(getParam<Real>("scaling_coefficient")),
    _primary_conductivity(getNeighborADMaterialProperty<Real>("primary_conductivity")),
    _secondary_conductivity(getADMaterialProperty<Real>("secondary_conductivity")),
    _primary_hardness(getNeighborADMaterialProperty<Real>("primary_hardness")),
    _secondary_hardness(getADMaterialProperty<Real>("secondary_hardness"))
{
}

ADReal
GapFluxModelPressureDependentConduction::computeFlux() const
{
  // Check that the surfaces are in actual contact with the pressure:
  if (_contact_pressure[_qp] <= 0.0)
    return 0.0;

  // calculate the harmonic means of the two material properties
  const ADReal k_sum = _primary_conductivity[_qp] + _secondary_conductivity[_qp];
  const ADReal k_harmonic = 2 * _primary_conductivity[_qp] * _secondary_conductivity[_qp] / k_sum;

  const ADReal h_sum = _primary_hardness[_qp] + _secondary_hardness[_qp];
  const ADReal h_harmonic = 2 * _primary_hardness[_qp] * _secondary_hardness[_qp] / h_sum;

  return _scaling * k_harmonic * (_primary_T[_qp] - _secondary_T[_qp]) * _contact_pressure[_qp] /
         h_harmonic;
}
