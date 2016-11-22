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

#include "SetupExecutionerProblemParamsAction.h"
#include "FEProblem.h"

template<>
InputParameters validParams<SetupExecutionerProblemParamsAction>()
{
  InputParameters params = FEProblem::executionerValidParams();
  // This action's params need validParams<...> defaults for Action in
  // addition to MooseObject - particularly for in-code app setup using the
  // action warehouse which assumes Action params have been set.
  params += validParams<Action>();
  return params;
}

SetupExecutionerProblemParamsAction::SetupExecutionerProblemParamsAction(InputParameters params) :
    Action(params)
{
}

void
SetupExecutionerProblemParamsAction::act()
{
  _problem->executionerInitParams(_pars);
}
