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

  std::string _tunit;
  std::string _punit;
  std::string _munit;
  std::vector<std::string> _phases;
  std::vector<std::string> _species;
  std::vector<std::string> _element_potentials;
};
