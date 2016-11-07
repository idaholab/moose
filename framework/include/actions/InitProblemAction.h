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

#ifndef INITPROBLEMACTION_H
#define INITPROBLEMACTION_H

#include "Action.h"

class InitProblemAction;

template <>
InputParameters validParams<InitProblemAction>();

class InitProblemAction : public Action
{
public:
  InitProblemAction(InputParameters params);

  virtual void act() override;
};

#endif // INITPROBLEMACTION_H
