//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
