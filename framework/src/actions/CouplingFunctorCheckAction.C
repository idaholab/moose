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
#include "TimedPrint.h"

#include "libmesh/system.h"
#include "libmesh/communicator.h"

registerMooseAction("MooseApp", CouplingFunctorCheckAction, "coupling_functor_check");

defineLegacyParams(CouplingFunctorCheckAction);

InputParameters
CouplingFunctorCheckAction::validParams()
{
  return Action::validParams();
}

CouplingFunctorCheckAction::CouplingFunctorCheckAction(InputParameters parameters)
  : Action(parameters)
{
  _name = "coupling_functor_check";
}

void
redistributeDofs(System & system)
{
  // Localize the vectors
  system.re_update();

  // Determine what dofs should be ghosted
  system.get_dof_map().distribute_dofs(system.get_mesh());

  // Recreate any constraints
  system.reinit_constraints();

  // Reinitialize the vectors with the new send_lists. I know the method name below is not perfectly
  // indicative of that...
  system.prolong_vectors();
}

void
redistributeDofs(SubProblem & problem)
{
  redistributeDofs(problem.systemBaseNonlinear().system());
  redistributeDofs(problem.systemBaseAuxiliary().system());
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
    if (_communicator.size() > 1)
    {
      const bool added =
          addRelationshipManagers(algebraic, RelationshipManager::oneLayerGhosting(algebraic));

      if (added)
      {
        // If you didn't do the ghosting with your own actions, you're going to pay the price now.
        // We have to reinit all the DofMaps so we can be sure that we've ghosted the necessary
        // vector entries
        CONTROLLED_CONSOLE_TIMED_PRINT(
            0, 1, "Reinitializing vectors because of late algebraic ghosting");

        // Attach the algebraic ghosting functors to the DofMaps
        _app.attachRelationshipManagers(algebraic);

        redistributeDofs(*_problem);
        if (auto displaced_problem = _problem->getDisplacedProblem())
          redistributeDofs(*displaced_problem);
      }
    }

    // Add the coupling functor. This plays a role regardless of whether we are running serially or
    // in parallel
    const bool added =
        addRelationshipManagers(coupling, RelationshipManager::oneLayerGhosting(coupling));
    if (added)
    {
      CONTROLLED_CONSOLE_TIMED_PRINT(
          0, 1, "Reinitializing sparsity pattern because of late coupling addition");

      // Along with attaching this coupling functor the the DofMap, we will make sure that the
      // DofMap coupling matrix gets attached to the coupling functor. The latter operation happens
      // when we call RelationshipManager::init
      _app.attachRelationshipManagers(coupling);

      // Reinit the libMesh (Implicit)System. This re-computes the sparsity pattern and then
      // applies it to the ImplicitSystem's matrices. Note that does NOT make a call to
      // DofMap::reinit, hence we have to call GhostingFunctor::dofmap_reinit ourselves in the
      // call above
      nl.system().reinit();
    }
  }
}
