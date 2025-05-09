//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  auto params = Action::validParams();
  params.set<std::string>("_action_name") = "coupling_functor_check";
  return Action::validParams();
}

CouplingFunctorCheckAction::CouplingFunctorCheckAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
CouplingFunctorCheckAction::act()
{
  for (const auto i : make_range(_problem->numNonlinearSystems()))
  {
    if (_problem->solverParams(i)._type == Moose::ST_JFNK)
      continue;

    auto & nl = _problem->getNonlinearSystemBase(i);
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

      // Add the coupling functor. This plays a role regardless of whether we are running serially
      // or in parallel
      if (addRelationshipManagers(coupling, RelationshipManager::oneLayerGhosting(coupling)))
        _app.attachRelationshipManagers(coupling);

      // If any nonlinear system added ghosting, then in our current MOOSE ghosting strategy we
      // added it for all systems so we can safely exit this loop
      break;
    }
  }
}
