//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AddFunctionAction.h"

/**
 * The ChemicalCompositionAction sets up user objects, aux kernels, and aux variables
 * for a thermochemistry calculation using Thermochimica.
 */
class ChemicalCompositionAction : public Action
{
public:
  static InputParameters validParams();
  ChemicalCompositionAction(const InputParameters & params);

  virtual void act();

protected:
  void readCSV();

  /// Element names
  std::vector<std::string> _elements;

  /// Initial conditions for each element: [element name] => initial condition value
  std::map<std::string, Real> _initial_conditions;

  /// Temperature unit
  std::string _tunit;

  /// Pressure unit
  std::string _punit;

  /// Mass/amount unit
  std::string _munit;

  /// List of phases tracked by Thermochimica
  std::vector<std::string> _phases;

  /// List of species tracked by Thermochimica
  std::vector<std::string> _species;

  /// List of element chemical potentials to be extracted from Thermochimica
  std::vector<std::string> _element_potentials;

  /// List of gas phase species to extract vapor pressures from Thermochimica
  std::vector<std::string> _vapor_pressures;
};
