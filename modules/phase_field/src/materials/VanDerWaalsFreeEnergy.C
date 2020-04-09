//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VanDerWaalsFreeEnergy.h"

registerMooseObject("PhaseFieldApp", VanDerWaalsFreeEnergy);

InputParameters
VanDerWaalsFreeEnergy::validParams()
{
  InputParameters params = GasFreeEnergyBase::validParams();
  params.addClassDescription("Free energy of a Van der Waals gas.");
  params.addRequiredParam<Real>("a",
                                "Van der Waals coefficient a (default mass_unit_conversion "
                                "requires this to be in [eV*Ang^3])");
  params.addRequiredParam<Real>("b",
                                "Van der Waals molecular exclusion volume b (default "
                                "mass_unit_conversion requires this to be in [Ang^3])");
  params.addParam<Real>("log_tol",
                        0.1,
                        "The logarithm in the free energy is evaluated using a Taylor expansion "
                        "below this value. This allows formulating free energies for systems where "
                        "the molecular volume is smaller than the exclusion volume b.");
  return params;
}

VanDerWaalsFreeEnergy::VanDerWaalsFreeEnergy(const InputParameters & parameters)
  : GasFreeEnergyBase(parameters),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _log_tol(getParam<Real>("log_tol"))
{
  // Definition of the free energy for the expression builder
  EBFunction free_energy;
  free_energy(_c, _T) =
      -_n * _kB * _T * (plog(_nq * (1.0 / _n - _b), _log_tol) + 1.0) - _n * _n * _a;

  // Parse function for automatic differentiation
  functionParse(free_energy);
}
