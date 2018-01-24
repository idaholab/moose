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

#include "InitProblemAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<InitProblemAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

InitProblemAction::InitProblemAction(InputParameters params) : Action(params) {}

void
InitProblemAction::act()
{
  if (_problem.get())
    _problem->init();
  else
    mooseError("Problem doesn't exist in InitProblemAction!");
}
