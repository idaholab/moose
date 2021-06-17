//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunction3PhaseMaterial.h"

registerMooseObject("PhaseFieldApp", SwitchingFunction3PhaseMaterial);

InputParameters
SwitchingFunction3PhaseMaterial::validParams()
{
  InputParameters params = DerivativeParsedMaterialHelper::validParams();
  params.addClassDescription(
      "Material for switching function that prevents formation of a third "
      "phase at a two-phase interface: $h_i = \\eta_i^2/4 [15 (1-\\eta_i) [1 + "
      "\\eta_i - (\\eta_k - \\eta_j)^2] + \\eta_i (9\\eta_i^2 - 5)]$");
  params.addRequiredCoupledVar("eta_i", "Order parameter i");
  params.addRequiredCoupledVar("eta_j", "Order parameter j");
  params.addRequiredCoupledVar("eta_k", "Order parameter k");
  params.addParam<bool>(
      "constrain_range",
      false,
      "Use a formulation that constrains the switching function values to [0:1]. This requires the "
      "Lagrange multiplier to constrain the sum of the switching function, rather than the etas.");
  return params;
}

SwitchingFunction3PhaseMaterial::SwitchingFunction3PhaseMaterial(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters), _eta_i("eta_i"), _eta_j("eta_j"), _eta_k("eta_k")
{
  EBTerm h_i;

  if (getParam<bool>("constrain_range"))
  {
    EBFunction h;
    EBTerm eta("eta");

    h(eta) =
        conditional(eta < 0, 0, conditional(eta > 1, 1, 3.0 * pow(eta, 2) - 2.0 * pow(eta, 3)));

    h_i = pow(h(_eta_i), 2) / 4.0 *
          (15.0 * (1.0 - h(_eta_i)) * (1.0 + h(_eta_i) - pow(h(_eta_k) - h(_eta_j), 2)) +
           h(_eta_i) * (9.0 * pow(h(_eta_i), 2) - 5.0));
  }
  else
    h_i = pow(_eta_i, 2) / 4.0 *
          (15.0 * (1.0 - _eta_i) * (1.0 + _eta_i - pow(_eta_k - _eta_j, 2)) +
           _eta_i * (9.0 * pow(_eta_i, 2) - 5.0));

  // Parse function for automatic differentiation
  functionParse(h_i);
}
