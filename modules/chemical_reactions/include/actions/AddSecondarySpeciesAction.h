//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AddAuxVariableAction.h"

class AddSecondarySpeciesAction : public AddAuxVariableAction
{
public:
  static InputParameters validParams();

  AddSecondarySpeciesAction(const InputParameters & params);

  virtual void act() override;

private:
  /// Secondary species to add
  const std::vector<AuxVariableName> _secondary_species;
};
