//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef APPLYCOUPLEDVARIABLESTESTACTION_H
#define APPLYCOUPLEDVARIABLESTESTACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class ApplyCoupledVariablesTestAction;

template <>
InputParameters validParams<ApplyCoupledVariablesTestAction>();

/**
 * An action for testing InputParameters::applyParameters
 */
class ApplyCoupledVariablesTestAction : public Action
{
public:
  /**
   * Class constructor
   */
  ApplyCoupledVariablesTestAction(const InputParameters & params);

  /**
   * Class destructor
   */
  virtual ~ApplyCoupledVariablesTestAction();

  /**
   * Creates an action for adding a Kernel
   */
  virtual void act();
};

#endif // APPLYCOUPLEDVARIABLESTESTACTION_H
