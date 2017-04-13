/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AddSlaveFluxVectorAction.h"
#include "Parser.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<AddSlaveFluxVectorAction>()
{
  return validParams<Action>();
}

AddSlaveFluxVectorAction::AddSlaveFluxVectorAction(const InputParameters & params) : Action(params)
{
}

void
AddSlaveFluxVectorAction::act()
{
  _problem->getNonlinearSystemBase().addVector("slave_flux", false, GHOSTED);
  _problem->getNonlinearSystemBase().zeroVectorForResidual("slave_flux");
}
