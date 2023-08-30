//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CostSensitivity.h"

registerMooseObject("troutApp", ComplianceSensitivity);

InputParameters
CostSensitivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes cost sensitivity needed for multimaterial SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredRangeCheckedParam<int>("power", "power>=1", "Penalty power for SIMP method.");
  params.addRequiredRangeCheckedParam<Real>("E", "E>0", "Young's modulus for the material.");
  params.addRequiredRangeCheckedParam<Real>(
      "Emin", "Emin>0", "Minimum value of Young's modulus for the material.");

  return params;
}

CostSensitivity::CostSensitivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _sensitivity(declareProperty<Real>(_base_name + "sensitivity")),
    _design_density(coupledValue("design_density")),
    _power(getParam<int>("power")),
    _E(getParam<Real>("E")),
    _Emin(getParam<Real>("Emin"))
{
}

void
CostSensitivity::computeQpProperties()
{

  // Compute the derivative of the compliance with respect to the design density
  // _power-2 because StrainEnergyDensity needed to be divided by the _design_density
  Real derivative = -_power * (_E - _Emin) * MathUtils::pow(_design_density[_qp], _power - 1) * 1.0;

  // This makes the sensitivity mesh size independent
  _sensitivity[_qp] = derivative;

  _design_density[_qp];
  _current_elem->volume();

  // C_e (rho_e) = A_c rho^{1/p} + B_c; A_c = (C_i - C_{i+1})/(rho_i^{1/p} - rho_{i+1}^{1/p}); B_c = C_i - A_c rho_i^{1/p}
}
