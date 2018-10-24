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

registerMooseAction("HeatConductionApp", AddSlaveFluxVectorAction, "add_slave_flux_vector");

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

  // It is risky to apply this optimization to contact problems
  // since the problem configuration may be changed during Jacobian
  // evaluation. We therefore turn it off for all contact problems so that
  // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3 or older.
  if (!_problem->isSNESMFReuseBaseSetbyUser())
    _problem->setSNESMFReuseBase(false, false);
}
