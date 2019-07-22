//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CouplingFunctorCheckAction.h"
#include "Kernel.h"
#include "MooseApp.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

#include "libmesh/system.h"

registerMooseAction("MooseApp", CouplingFunctorCheckAction, "coupling_functor_check");

template <>
InputParameters
validParams<CouplingFunctorCheckAction>()
{
  return validParams<Action>();
}

CouplingFunctorCheckAction::CouplingFunctorCheckAction(InputParameters parameters)
  : Action(parameters)
{
}

void
CouplingFunctorCheckAction::act()
{
  auto & nl = _problem->getNonlinearSystemBase();
  auto & kernels = nl.getKernelWarehouse();
  auto & nbcs = nl.getNodalBCWarehouse();
  auto & ibcs = nl.getIntegratedBCWarehouse();

  // If we have any of these, we need default coupling (e.g. element intra-dof coupling)
  if (kernels.size() || nbcs.size() || ibcs.size())
  {
    auto original_size = _app.relationshipManagers().size();

    // It doesn't really matter what T is in validParams<T> as long as it will add our default
    // coupling functor object (ElementSideNeighborLayers with n_levels of 0)
    addRelationshipManagers(Moose::RelationshipManagerType::COUPLING, validParams<Kernel>());

    // See whether we've actually added anything new. If we have, then unfortunately we need to
    // reinit our libMesh::ImplicitSystem in order to update the sparsity
    if (original_size != _app.relationshipManagers().size())
    {
      // There's no concern of duplicating functors previously added here since functors are stored
      // as std::sets
      _app.attachRelationshipManagers(Moose::RelationshipManagerType::COUPLING);

      nl.system().reinit();
    }
  }
}
