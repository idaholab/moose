//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPeriodicBCAction.h"

#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "RelationshipManager.h"

#include "libmesh/periodic_boundary.h"

registerMooseAction("MooseApp", AddPeriodicBCAction, "add_periodic_bc");
registerMooseAction("MooseApp", AddPeriodicBCAction, "add_geometric_rm");
registerMooseAction("MooseApp", AddPeriodicBCAction, "add_algebraic_rm");

InputParameters
AddPeriodicBCAction::validParams()
{
  InputParameters params = Action::validParams();
  params += Moose::PeriodicBCHelper::validParams();

  params.addParam<std::vector<VariableName>>("variable",
                                             "Variable(s) for the periodic boundary condition; if "
                                             "not provided, apply to all variables.");

  params.addClassDescription("Action that adds periodic boundary conditions");

  return params;
}

AddPeriodicBCAction::AddPeriodicBCAction(const InputParameters & params)
  : Action(params),
    Moose::PeriodicBCHelper(static_cast<const Action &>(*this), /* algebraic = */ true)
{
  checkPeriodicParams();
}

void
AddPeriodicBCAction::onSetupPeriodicBoundary(libMesh::PeriodicBoundaryBase & p)
{
  // TODO: multi-system
  if (_problem->numSolverSystems() > 1)
    mooseError("Multiple solver systems currently not supported");

  NonlinearSystemBase & nl = _problem->getNonlinearSystemBase(/*nl_sys_num=*/0);

  // If var_names is empty - then apply this periodic condition to all variables in the system
  const auto variable_ptr = queryParam<std::vector<VariableName>>("variable");
  const auto & var_names = variable_ptr ? *variable_ptr : nl.getVariableNames();

  // Helper function to apply periodic BC for a given variable number
  auto apply_periodic_bc = [&](unsigned int var_num, const std::string & var_name)
  {
    p.set_variable(var_num);
    if (_mesh->isRegularOrthogonal())
      _mesh->addPeriodicVariable(var_num, p.myboundary, p.pairedboundary);
    else
      mooseInfoRepeated("Periodicity information for variable '" + var_name +
                        "' will only be stored in the system's DoF map, not on the MooseMesh");
  };

  // If is an array variable, loop over all of components
  for (const auto & var_name : var_names)
  {
    // Exclude scalar variables for which periodic boundary conditions dont make sense
    if (!nl.hasScalarVariable(var_name))
    {
      const auto & var = nl.getVariable(0, var_name);
      const auto var_num = var.number();

      if (var.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
      {
        for (const auto component : make_range(var.count()))
          apply_periodic_bc(var_num + component, var_name + "_" + std::to_string(component));
      }
      else
        apply_periodic_bc(var_num, var_name);
    }
  }
}

void
AddPeriodicBCAction::act()
{
  if (_current_task == "add_geometric_rm")
    // Tell the mesh to hold off on deleting remote elements because we need to wait for our
    // periodic boundaries to be added
    Action::_mesh->allowRemoteElementRemoval(false);

  if (_current_task == "add_algebraic_rm")
  {
    auto rm_params = _factory.getValidParams("ElementSideNeighborLayers");

    rm_params.set<std::string>("for_whom") = "PeriodicBCs";
    if (!_mesh)
      mooseError("We should have added periodic boundaries and consequently we should have set the "
                 "_mesh by now");

    rm_params.set<MooseMesh *>("mesh") = _mesh;
    // The default GhostPointNeighbors ghosting functor in libMesh handles the geometric ghosting
    // of periodic boundaries for us, so we only need to handle the algebraic ghosting here
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::ALGEBRAIC;

    if (rm_params.areAllRequiredParamsValid())
    {
      auto rm_obj = _factory.create<RelationshipManager>(
          "ElementSideNeighborLayers", "periodic_bc_ghosting_" + name(), rm_params);

      if (!_app.addRelationshipManager(rm_obj))
        _factory.releaseSharedObjects(*rm_obj);
    }
    else
      mooseError("Invalid initialization of ElementSideNeighborLayers");
  }

  if (_current_task == "add_periodic_bc")
  {
    _mesh = &_problem->mesh();
    setupPeriodicBoundaries(*_problem);
  }
}
