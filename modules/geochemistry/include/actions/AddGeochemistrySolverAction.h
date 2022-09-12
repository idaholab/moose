//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * Action that sets up GeochemistryConsoleOutput and various AuxVariables
 */
class AddGeochemistrySolverAction : public Action
{
public:
  static InputParameters validParams();

  AddGeochemistrySolverAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /**
   * Adds AuxVariable and AuxKernel that will record species concentrations
   * @param var_name AuxVariable name
   * @param species_name Species name
   * @param unit Unit choice: defined in GeochemistryQuantityAux
   */
  void addAuxSpecies(const std::string & var_name,
                     const std::string & species_name,
                     const std::string & unit);
};
