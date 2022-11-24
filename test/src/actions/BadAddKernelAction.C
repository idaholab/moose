//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BadAddKernelAction.h"
#include "FEProblem.h"

registerMooseAction("MooseTestApp", BadAddKernelAction, "add_kernel");

InputParameters
BadAddKernelAction::validParams()
{
  return MooseObjectAction::validParams();
}

BadAddKernelAction::BadAddKernelAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
BadAddKernelAction::act()
{
  // Wrong method being called for adding *Kernel* object.
  // Note: we chose addIndicator() for this Action so that a specific
  // Factory error is triggered (and not some other error check on
  // e.g. variable types).
  _problem->addBoundaryCondition(_type, _name, _moose_object_pars);
}
