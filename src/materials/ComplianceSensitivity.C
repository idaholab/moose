//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComplianceSensitivity.h"

registerMooseObject("troutApp", ComplianceSensitivity);

InputParameters
ComplianceSensitivity::validParams()
{
  InputParameters params = StrainEnergyDensity::validParams();
  params.addClassDescription("Computes compliance sensitivity needed for SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredParam<int>("power", "Penalty power for SIMP method.");
  params.addRequiredParam<Real>("E", "Young's modulus for the material.");
  params.addRequiredParam<Real>("Emin", "Minimum value of Young's modulus for the material.");

  return params;
}

ComplianceSensitivity::ComplianceSensitivity(const InputParameters & parameters)
  : StrainEnergyDensity(parameters),
    _sensitivity(declareProperty<Real>(_base_name + "sensitivity")),
    _design_density(coupledValue("design_density")),
    _power(getParam<int>("power")),
    _E(getParam<Real>("E")),
    _Emin(getParam<Real>("Emin"))
{
}

void
ComplianceSensitivity::computeQpProperties()
{
  // Call the parent class's method to compute the strain energy density
  StrainEnergyDensity::computeQpProperties();

  // Compute the compliance as the strain energy density at the quadrature point
  Real compliance = _strain_energy_density[_qp];

  // Compute the derivative of the compliance with respect to the design density
  // _power-2 because StrainEnergyDensity needed to be divided by the _design_density
  Real derivative =
      -_power * (_E - _Emin) * MathUtils::pow(_design_density[_qp], _power - 2) * compliance;

  // This makes the sensitivity mesh size independent
  _sensitivity[_qp] = derivative;
}
