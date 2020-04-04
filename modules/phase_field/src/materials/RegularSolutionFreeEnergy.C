//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RegularSolutionFreeEnergy.h"

registerMooseObject("PhaseFieldApp", RegularSolutionFreeEnergy);

InputParameters
RegularSolutionFreeEnergy::validParams()
{
  InputParameters params = DerivativeParsedMaterialHelper::validParams();
  params.addClassDescription("Material that implements the free energy of a regular solution");
  params.addRequiredCoupledVar("c", "Concentration variable");
  params.addCoupledVar("T", 300, "Temperature variable");
  params.addParam<Real>("omega", 0.1, "Regular solution parameter");
  params.addParam<Real>("kB", 8.6173324e-5, "Boltzmann constant");
  params.addParam<Real>(
      "log_tol", "If specified logarithms are evaluated using a Taylor expansion below this value");
  return params;
}

RegularSolutionFreeEnergy::RegularSolutionFreeEnergy(const InputParameters & parameters)
  : DerivativeParsedMaterialHelper(parameters),
    _c("c"),
    _T("T"),
    _omega(getParam<Real>("omega")),
    _kB(getParam<Real>("kB"))
{
  EBFunction free_energy;
  // Definition of the free energy for the expression builder
  free_energy(_c) =
      _omega * _c * (1.0 - _c) + _kB * _T * (_c * log(_c) + (1.0 - _c) * log(1.0 - _c));

  // Use Taylor expanded logarithm?
  if (isParamValid("log_tol"))
    free_energy.substitute(EBLogPlogSubstitution(getParam<Real>("log_tol")));

  // Parse function for automatic differentiation
  functionParse(free_energy);
}
