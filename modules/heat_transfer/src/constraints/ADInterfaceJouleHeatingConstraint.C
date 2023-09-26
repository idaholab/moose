//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADInterfaceJouleHeatingConstraint.h"

registerMooseObject("HeatConductionApp", ADInterfaceJouleHeatingConstraint);

InputParameters
ADInterfaceJouleHeatingConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "Joule heating model, for the case of a closed gap interface, to calculate the heat flux "
      "contribution created when an electric potential difference occurs across that interface.");
  params.addRequiredCoupledVar(
      "potential_lagrange_multiplier",
      "The name of the lagrange multiplier variable used in the calculation of the electrical "
      "potential mortar constrain calculation");
  params.addRequiredParam<MaterialPropertyName>(
      "primary_electrical_conductivity",
      "The electrical conductivity of the primary surface solid material");
  params.addRequiredParam<MaterialPropertyName>(
      "secondary_electrical_conductivity",
      "The electrical conductivity of the secondary surface solid material");
  params.addParam<Real>("weighting_factor",
                        0.5,
                        "Weight applied to divide the heat flux from Joule heating at the "
                        "interface between the primary and secondary surfaces.");
  return params;
}

ADInterfaceJouleHeatingConstraint::ADInterfaceJouleHeatingConstraint(
    const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _lm_electrical_potential(adCoupledLowerValue("potential_lagrange_multiplier")),
    _primary_conductivity(getNeighborADMaterialProperty<Real>("primary_electrical_conductivity")),
    _secondary_conductivity(getADMaterialProperty<Real>("secondary_electrical_conductivity")),
    _weight_factor(getParam<Real>("weighting_factor"))
{
}

ADReal
ADInterfaceJouleHeatingConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  // calculate the harmonic means of the two material properties
  const ADReal C_sum = _primary_conductivity[_qp] + _secondary_conductivity[_qp];
  const ADReal C_harmonic = 2.0 * _primary_conductivity[_qp] * _secondary_conductivity[_qp] / C_sum;

  ADReal potential_flux_sq = _lm_electrical_potential[_qp] * _lm_electrical_potential[_qp];
  ADReal q_electric = potential_flux_sq / C_harmonic;

  switch (mortar_type)
  {
    case Moose::MortarType::Primary:
    {
      auto source = -q_electric * _weight_factor * _test_primary[_i][_qp];
      return source;
    }

    case Moose::MortarType::Secondary:
    {
      auto source = -q_electric * (1.0 - _weight_factor) * _test_secondary[_i][_qp];
      return source;
    }

    default:
      return 0;
  }
}
