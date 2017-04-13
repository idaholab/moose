/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MathEBFreeEnergy.h"

template <>
InputParameters
validParams<MathEBFreeEnergy>()
{
  InputParameters params = validParams<DerivativeParsedMaterialHelper>();
  params.addClassDescription("Material that implements the math free energy using the expression "
                             "builder and automatric differentiation");
  params.addRequiredCoupledVar("c", "Concentration variable");
  return params;
}

MathEBFreeEnergy::MathEBFreeEnergy(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters), _c("c")
{
  EBFunction free_energy;
  // Definition of the free energy for the expression builder
  free_energy(_c) = 1.0 / 4.0 * (1.0 + _c) * (1.0 + _c) * (1.0 - _c) * (1.0 - _c);

  // Parse function for automatic differentiation
  functionParse(free_energy);
}
