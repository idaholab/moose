//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MathEBFreeEnergy.h"

registerMooseObject("PhaseFieldApp", MathEBFreeEnergy);

InputParameters
MathEBFreeEnergy::validParams()
{
  InputParameters params = DerivativeParsedMaterialHelper::validParams();
  params.addClassDescription("Material that implements the math free energy using the expression "
                             "builder and automatic differentiation");
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
