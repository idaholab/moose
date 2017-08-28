/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "IdealGasFreeEnergy.h"

template <>
InputParameters
validParams<IdealGasFreeEnergy>()
{
  InputParameters params = validParams<GasFreeEnergyBase>();
  params.addClassDescription("Free energy of an ideal gas.");
  return params;
}

IdealGasFreeEnergy::IdealGasFreeEnergy(const InputParameters & parameters)
  : GasFreeEnergyBase(parameters)
{
  // Definition of the free energy for the expression builder
  EBFunction free_energy;
  free_energy(_c, _T) = -_n * _kB * _T * (log(_nq / _n) + 1.0);

  // Parse function for automatic differentiation
  functionParse(free_energy);
}
