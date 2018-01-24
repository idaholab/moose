//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CHECKOUTPUTACTION_H
#define CHECKOUTPUTACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class CheckOutputAction;

template <>
InputParameters validParams<CheckOutputAction>();

/**
 * Action for checking that "outputs" is properly populated for Materials
 */
class CheckOutputAction : public Action
{
public:
  /**
   * Class constructor
   * @param params Input parameters for this object
   */
  CheckOutputAction(InputParameters params);

  virtual void act() override;

private:
  /**
   * Performs check for "outputs" option for Variables and AuxVariables blocks
   * @param task The name of the task to extract names from (add_variable or add_aux_variable)
   */
  void checkVariableOutput(const std::string & task);

  /**
   * Preforms a set of checks on each of the Material objects that the "outputs" parameters has
   * valid values
   */
  void checkMaterialOutput();

  /**
   * Performs Console Output object related checks
   */
  void checkConsoleOutput();

  /**
   * Performs PerfLog output settings
   */
  void checkPerfLogOutput();
};

#endif // CHECKOUTPUTACTION_H
