//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SetupPreconditionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "MoosePreconditioner.h"
#include "FEProblem.h"
#include "CreateExecutionerAction.h"
#include "NonlinearSystemBase.h"

unsigned int SetupPreconditionerAction::_count = 0;

template <>
InputParameters
validParams<SetupPreconditionerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();

  return params;
}

SetupPreconditionerAction::SetupPreconditionerAction(InputParameters params)
  : MooseObjectAction(params)
{
}

void
SetupPreconditionerAction::act()
{
  if (_problem.get() != NULL)
  {
    // build the preconditioner
    _moose_object_pars.set<FEProblemBase *>("_fe_problem_base") = _problem.get();
    std::shared_ptr<MoosePreconditioner> pc =
        _factory.create<MoosePreconditioner>(_type, _name, _moose_object_pars);

    _problem->getNonlinearSystemBase().setPreconditioner(pc);

/**
 * Go ahead and set common precondition options here.  The child classes will still be called
 * through the action warehouse
 */
#if LIBMESH_HAVE_PETSC
    Moose::PetscSupport::storePetscOptions(*_problem, _moose_object_pars);
#endif
  }
}
