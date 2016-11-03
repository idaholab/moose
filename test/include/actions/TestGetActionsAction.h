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

#ifndef TESTGETACTIONSACTION_H
#define TESTGETACTIONSACTION_H

#include "Action.h"

class TestGetActionsAction;

template<>
InputParameters validParams<TestGetActionsAction>();

/**
 * This action tests two functions provided by action warehouse: getAction and getActions.
 * It uses AddMaterialAction as the example for calling these two functions.
 * These two functions allow an action to interfact with other actions.
 */
class TestGetActionsAction : public Action
{
public:
  TestGetActionsAction(const InputParameters & params);

  virtual void act() override;
};

#endif // TESTGETACTIONSACTION_H
