//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DisplayGhostingAction.h"
#include "FEProblemBase.h"

registerMooseAction("MooseApp", DisplayGhostingAction, "add_aux_variable");

registerMooseAction("MooseApp", DisplayGhostingAction, "add_aux_kernel");

registerMooseAction("MooseApp", DisplayGhostingAction, "add_user_object");

InputParameters
DisplayGhostingAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>("output_ghosting", false, "Boolean to turn on ghosting auxiliary fields");
  params.addParam<bool>("include_local_in_ghosting",
                        false,
                        "Boolean used to toggle on the inclusion of local elements along with the "
                        "ghost elements for a complete partition map");

  params.addClassDescription(
      "Action to setup AuxVariables and AuxKernels to display ghosting when running in parallel");

  return params;
}

DisplayGhostingAction::DisplayGhostingAction(const InputParameters & params)
  : Action(params),
    _display_ghosting(getParam<bool>("output_ghosting")),
    _include_local(getParam<bool>("include_local_in_ghosting"))
{
}

void
DisplayGhostingAction::act()
{
  if (_display_ghosting == false)
    return;

  auto n_procs = _app.n_processors();

  if (_current_task == "add_aux_variable")
  {
    auto params = _factory.getValidParams("MooseVariableConstMonomial");
    params.set<MooseEnum>("order") = "CONSTANT";
    params.set<MooseEnum>("family") = "MONOMIAL";

    for (unsigned int i = 0; i < 2; ++i)
    {
      std::string var_name_base = (i == 0 ? "geometric" : "algebraic");
      for (decltype(n_procs) proc_id = 0; proc_id < n_procs; ++proc_id)
        _problem->addAuxVariable(
            "MooseVariableConstMonomial", var_name_base + std::to_string(proc_id), params);
    }
  }
  else if (_current_task == "add_aux_kernel")
  {
    for (unsigned int i = 0; i < 2; ++i)
    {
      std::string aux_kernel_name_base = i == 0 ? "geometric" : "algebraic";
      for (decltype(n_procs) proc_id = 0; proc_id < n_procs; ++proc_id)
      {
        std::string name = aux_kernel_name_base + std::to_string(proc_id);

        auto params = _factory.getValidParams("GhostingAux");
        params.set<processor_id_type>("pid") = proc_id;
        params.set<MooseEnum>("functor_type") = aux_kernel_name_base;
        params.set<UserObjectName>("ghost_uo") = "ghost_uo";
        params.set<AuxVariableName>("variable") = name;
        params.set<bool>("include_local_elements") = _include_local;

        _problem->addAuxKernel("GhostingAux", name, params);
      }
    }
  }
  else if (_current_task == "add_user_object")
  {
    auto params = _factory.getValidParams("GhostingUserObject");
    _problem->addUserObject("GhostingUserObject", "ghost_uo", params);
  }
}
