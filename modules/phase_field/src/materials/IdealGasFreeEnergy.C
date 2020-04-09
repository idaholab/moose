//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdealGasFreeEnergy.h"

registerMooseObject("PhaseFieldApp", IdealGasFreeEnergy);

InputParameters
IdealGasFreeEnergy::validParams()
{
  InputParameters params = GasFreeEnergyBase::validParams();
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
