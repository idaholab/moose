//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CouplingFunctorCheckAction.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "NonlinearSystemBase.h"
#include "InputParameters.h"
#include "RelationshipManager.h"

#include "libmesh/system.h"
#include "libmesh/communicator.h"

registerMooseAction("MooseApp", CouplingFunctorCheckAction, "coupling_functor_check");

InputParameters
CouplingFunctorCheckAction::validParams()
{
  return Action::validParams();
}

CouplingFunctorCheckAction::CouplingFunctorCheckAction(const InputParameters & parameters)
  : Action(parameters)
{
  _name = "coupling_functor_check";
}

void
CouplingFunctorCheckAction::act()
{
  // If we're doing Jacobian-free, then we have no matrix and we can just return
  if (_problem->solverParams()._type == Moose::ST_JFNK)
    return;

  auto & nl = _problem->getNonlinearSystemBase();
  auto & dgs = nl.getDGKernelWarehouse();
  auto & iks = nl.getInterfaceKernelWarehouse();

  // to reduce typing
  auto algebraic = Moose::RelationshipManagerType::ALGEBRAIC;
  auto coupling = Moose::RelationshipManagerType::COUPLING;

  // If we have any DGKernels or InterfaceKernels we need one layer of sparsity
  if (dgs.size() || iks.size())
  {
    // We are going to add the algebraic ghosting and coupling functors one at a time because then
    // we can keep track of whether we need to redistribute the dofs or not

    // Add the algebraic ghosting functors if we are running in parallel
    if (_communicator.size() > 1 &&
        addRelationshipManagers(algebraic, RelationshipManager::oneLayerGhosting(algebraic)))
      _app.attachRelationshipManagers(algebraic);

    // Add the coupling functor. This plays a role regardless of whether we are running serially or
    // in parallel
    if (addRelationshipManagers(coupling, RelationshipManager::oneLayerGhosting(coupling)))
      _app.attachRelationshipManagers(coupling);
  }
}
