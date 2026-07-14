//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "ThermochimicaConfiguration.h"

/**
 * The ChemicalCompositionAction sets up user objects, aux kernels, and aux variables
 * for a thermochemistry calculation using Thermochimica.
 */
class ChemicalCompositionAction : public Action
{
public:
  static InputParameters validParams();
  ChemicalCompositionAction(const InputParameters & params);

  virtual void act() override;

protected:
  void readCSV();

  /// Initial conditions for each element: [element name] => initial condition value
  std::map<std::string, Real> _initial_conditions;

  /// Validated immutable configuration consumed by the runtime executor
  std::shared_ptr<ThermochimicaConfiguration> _configuration;
};
