//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMPreconditioningIntegrityCheckAction.h"
#include "THMProblem.h"
#include "ThermalHydraulicsApp.h"

registerMooseAction("ThermalHydraulicsApp",
                    THMPreconditioningIntegrityCheckAction,
                    "THM:preconditioning_integrity_check");

InputParameters
THMPreconditioningIntegrityCheckAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

THMPreconditioningIntegrityCheckAction::THMPreconditioningIntegrityCheckAction(
    const InputParameters & parameters)
  : Action(parameters)
{
}

void
THMPreconditioningIntegrityCheckAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->couplingMatrixIntegrityCheck();
}
