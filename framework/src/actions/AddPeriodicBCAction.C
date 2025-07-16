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
#include "NonlinearSystemBase.h"
#include "RelationshipManager.h"

#include "libmesh/periodic_boundary.h"

#include <optional>

registerMooseAction("MooseApp", AddPeriodicBCAction, "add_periodic_bc");
registerMooseAction("MooseApp", AddPeriodicBCAction, "add_geometric_rm");
registerMooseAction("MooseApp", AddPeriodicBCAction, "add_algebraic_rm");

InputParameters
AddPeriodicBCAction::validParams()
{
  InputParameters params = Action::validParams();
  params += Moose::PeriodicBCHelper::validParams();

  params.addParam<std::vector<VariableName>>("variable",
                                             "Variable(s) to apply periodic boundary conditions "
                                             "to; if unset, apply to all field variables.");

  params.addClassDescription("Action that adds periodic boundary conditions");

  return params;
}

AddPeriodicBCAction::AddPeriodicBCAction(const InputParameters & params)
  : Action(params), Moose::PeriodicBCHelper(static_cast<const Action &>(*this))
{
  checkPeriodicParams();
}

void
AddPeriodicBCAction::act()
{
  if (_current_task == "add_geometric_rm")
    // Tell the mesh to hold off on deleting remote elements because we need to wait for our
    // periodic boundaries to be added
    Action::_mesh->allowRemoteElementRemoval(false);

  if (_current_task == "add_algebraic_rm" && getPeriodicBoundaries().size())
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

    // Set _vars so that variables can be used in onSetupPeriodicBoundary
    _vars = getVariables();
    mooseAssert(_vars.size(), "Shouldn't run without variables");

    // Query the helper to determine the periodic boundaries, which will
    // call onSetupPeriodicBoundary() for every periodic boundary
    setupPeriodicBoundaries(*_problem);

    if (!_mesh->isRegularOrthogonal())
    {
      std::ostringstream out;
      out << "Periodicity information for the variables\n";
      for (const auto & var_ptr : _vars)
        out << "  - " << var_ptr->name() << "\n";
      out << "will only be stored in the system's DoF map, not on the MooseMesh";
      mooseInfoRepeated(out.str());
    }
  }
}

void
AddPeriodicBCAction::onSetupPeriodicBoundary(libMesh::PeriodicBoundaryBase & p)
{
  const auto is_regular_orthogonal = _mesh->isRegularOrthogonal();
  for (const auto & var_ptr : _vars)
  {
    const auto sys_num = var_ptr->sys().number();
    for (const auto component : make_range(var_ptr->count()))
    {
      const auto var_num = var_ptr->number() + component;

      // Set variable number in PeriodicBoundaryBase object
      p.set_variable(var_num);

      // Add to MooseMesh to querying variable periodicity
      if (is_regular_orthogonal)
        _mesh->addPeriodicVariable(sys_num, var_num, p.myboundary, p.pairedboundary);

      // Add to dof maps for algebraic ghosting
      const auto add_to_dof_map = [&p, &sys_num](auto & problem)
      { problem.es().get_system(sys_num).get_dof_map().add_periodic_boundary(p); };
      add_to_dof_map(*_problem);
      if (auto displaced_problem = _problem->getDisplacedProblem())
        add_to_dof_map(*displaced_problem);
    }
  }
}

std::vector<const MooseVariableFieldBase *>
AddPeriodicBCAction::getVariables() const
{
  std::vector<VariableName> var_names;
  // Variable is set, use it
  if (isParamValid("variable"))
  {
    var_names = getParam<std::vector<VariableName>>("variable");
    if (var_names.empty())
      paramError("variable", "No variables are set to apply periodic boundaries to");
    for (const auto & var_name : var_names)
    {
      if (_problem->hasScalarVariable(var_name))
        paramError("variable",
                   "Variable '" + var_name +
                       "' is a scalar variable and does not support a periodic boundary condition");
      if (!_problem->hasVariable(var_name))
        paramError("variable", "Nonlinear variable '" + var_name + "' not found");
    }
  }
  // Variable is not set, use all the variables
  else
  {
    // We can't currently distinguish PeriodicBoundaries objects across
    // multiple systems so we can't use vars across all systems
    if (_problem->numSolverSystems() > 1)
      mooseError("Parameter 'variable' must be specified when multiple solver systems exist");
    // Use all field variables from system 0, excluding scalar varaibles
    const auto & nl = _problem->getNonlinearSystemBase(0);
    var_names = nl.getVariableNames();
    var_names.erase(std::remove_if(var_names.begin(),
                                   var_names.end(),
                                   [&nl](const auto & var_name)
                                   { return nl.hasScalarVariable(var_name); }),
                    var_names.end());
    if (var_names.empty())
      mooseError("There are no variables to apply periodic boundaries to");
  }

  // Verify and collect variables
  std::vector<const MooseVariableFieldBase *> vars;
  vars.reserve(var_names.size());
  std::optional<unsigned int> used_sys_num;
  for (const auto & var_name : var_names)
  {
    const auto & var = _problem->getVariable(0, var_name);
    const auto sys_num = var.sys().number();

    // Until we have a way to have separate PeriodicBoundaries objects for each systems,
    // we can't do these in the same block
    if (used_sys_num && *used_sys_num != sys_num)
      paramError("variable",
                 "Variables were specified across multiple systems; this is not supported. Use a "
                 "separate [Periodic/BCs] block for each system.");
    used_sys_num = sys_num;

    vars.push_back(&var);
  }

  return vars;
}
