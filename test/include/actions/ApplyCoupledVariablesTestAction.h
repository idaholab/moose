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
