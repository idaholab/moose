//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckFVBCAction.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "FVFluxBC.h"
#include "FVDirichletBCBase.h"

registerMooseAction("MooseApp", CheckFVBCAction, "check_integrity");

InputParameters
CheckFVBCAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Check that boundary conditions are defined correctly for finite volume problems.");
  return params;
}

CheckFVBCAction::CheckFVBCAction(const InputParameters & params) : Action(params) {}

void
CheckFVBCAction::act()
{
  if (_current_task == "check_integrity" && _problem->fvBCsIntegrityCheck())
  {
    // check that boundary conditions follow these rules:
    // 1. One variable cannot define Dirichlet & Flux BCs
    //    on the same sideset
    // 2. One variable cannot define more than one Dirichlet BCs
    //    on the same sideset
    TheWarehouse & the_warehouse = _problem->theWarehouse();
    const std::vector<MooseVariableFEBase *> & variables =
        _problem->getNonlinearSystemBase().getVariables(0);

    for (auto & var : variables)
    {
      if (!var->isFV())
        continue;

      unsigned int var_num = var->number();
      std::vector<FVFluxBC *> flux_bcs;
      std::vector<FVDirichletBCBase *> dirichlet_bcs;
      the_warehouse.query()
          .template condition<AttribSystem>("FVFluxBC")
          .template condition<AttribVar>(var_num)
          .template condition<AttribThread>(0)
          .template condition<AttribSysNum>(var->sys().number())
          .queryInto(flux_bcs);

      the_warehouse.query()
          .template condition<AttribSystem>("FVDirichletBC")
          .template condition<AttribVar>(var_num)
          .template condition<AttribThread>(0)
          .template condition<AttribSysNum>(var->sys().number())
          .queryInto(dirichlet_bcs);

      std::set<BoundaryID> all_flux_side_ids;
      for (auto & fbc : flux_bcs)
      {
        const std::set<BoundaryID> & t = fbc->boundaryIDs();
        all_flux_side_ids.insert(t.begin(), t.end());
      }

      std::set<BoundaryID> all_dirichlet_side_ids;
      for (auto & dbc : dirichlet_bcs)
      {
        const std::set<BoundaryID> & temp = dbc->boundaryIDs();
        // check rule #2
        for (auto & t : temp)
          if (all_dirichlet_side_ids.find(t) != all_dirichlet_side_ids.end())
            mooseError(
                "Sideset ", t, " defines at least two DirichletBCs for variable ", var->name());
        all_dirichlet_side_ids.insert(temp.begin(), temp.end());
      }

      // check rule #1
      for (auto & t : all_flux_side_ids)
        if (all_dirichlet_side_ids.find(t) != all_dirichlet_side_ids.end())
          mooseError(
              "Sideset ", t, " defines a FluxBC and a DirichletBC for variable ", var->name());
    }
  }
}
