/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SwitchingFunction3PhaseMaterial.h"

template <>
InputParameters
validParams<SwitchingFunction3PhaseMaterial>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Material for switching function that prevents formation of a third "
                             "phase at a two-phase interface: h_i = eta_i^2/4 * [15 (1-eta_i) [1 + "
                             "eta_i - (eta_k - eta_j)^2] + eta_i * (9eta_i^2 - 5)]");
  params.addRequiredCoupledVar("eta_i", "Order parameter i");
  params.addRequiredCoupledVar("eta_j", "Order parameter j");
  params.addRequiredCoupledVar("eta_k", "Order parameter k");
  return params;
}

SwitchingFunction3PhaseMaterial::SwitchingFunction3PhaseMaterial(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters), _eta_i("eta_i"), _eta_j("eta_j"), _eta_k("eta_k")
{
  EBFunction h_i;
  // Definition of the switching function for the expression builder
  h_i(_eta_i, _eta_j, _eta_k) =
      _eta_i * _eta_i / 4.0 *
      (15.0 * (1.0 - _eta_i) * (1.0 + _eta_i - (_eta_k - _eta_j) * (_eta_k - _eta_j)) +
       _eta_i * (9.0 * _eta_i * _eta_i - 5.0));

  // Parse function for automatic differentiation
  functionParse(h_i);
}
