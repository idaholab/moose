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
#include "CoupledExecutioner.h"

template<>
InputParameters validParams<InitProblemAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}


InitProblemAction::InitProblemAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
InitProblemAction::act()
{
  if (_problem != NULL)
    _problem->init();
  else
  {
    // init_problem is a mandatory action, we have an executioner at this point and all FEProblems are
    // already added, we build the coupled system
    CoupledExecutioner * ex = dynamic_cast<CoupledExecutioner *>(_executioner);
    if (ex != NULL)
      ex->build();
  }
}
