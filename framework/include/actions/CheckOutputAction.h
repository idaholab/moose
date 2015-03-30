/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CHECKOUTPUTACTION_H
#define CHECKOUTPUTACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class CheckOutputAction;

template<>
InputParameters validParams<CheckOutputAction>();

/**
 * Action for checking that "outputs" is properly populated for Materials
 */
class CheckOutputAction : public Action
{
public:

  /**
   * Class constructor
   * @param name Name of this action
   * @param params Input parameters for this object
   */
  CheckOutputAction(const std::string & name, InputParameters params);

  /**
   * Class destructor
   */
  virtual ~CheckOutputAction();

  /**
   * Preforms a set of checks on various Output objects
   */
  virtual void act();

private:

  /**
   * Performs check for "outputs" option for Variables and AuxVariables blocks
   * @param task The name of the task to extract names from (add_variable or add_aux_variable)
   */
  void checkVariableOutput(const std::string & task);

  /**
   * Preforms a set of checks on each of the Material objects that the "outputs" parameters has valid values
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

  /**
   * Checks for --output-input command line parameter
   */
  void checkInputOutput();
};

#endif //CHECKOUTPUTACTION_H
