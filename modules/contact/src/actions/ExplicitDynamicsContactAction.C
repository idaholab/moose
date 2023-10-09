//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitDynamicsContactAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AddVariableAction.h"
#include "NonlinearSystemBase.h"
#include "Parser.h"

#include "NanoflannMeshAdaptor.h"
#include "PointListAdaptor.h"

#include <set>
#include <algorithm>
#include <unordered_map>
#include <limits>

#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/string_to_enum.h"

// Counter for naming auxiliary kernels
static unsigned int ed_contact_auxkernel_counter = 0;

// Counter for naming nodal area user objects
static unsigned int ed_contact_userobject_counter = 0;

// Counter for distinct contact action objects
static unsigned int ed_contact_action_counter = 0;

registerMooseAction("ContactApp", ExplicitDynamicsContactAction, "add_aux_variable");
registerMooseAction("ContactApp", ExplicitDynamicsContactAction, "add_contact_aux_variable");
registerMooseAction("ContactApp", ExplicitDynamicsContactAction, "add_aux_kernel");

registerMooseAction("ContactApp", ExplicitDynamicsContactAction, "add_constraint");
registerMooseAction("ContactApp", ExplicitDynamicsContactAction, "add_user_object");

InputParameters
ExplicitDynamicsContactAction::validParams()
{
  InputParameters params = Action::validParams();
  params += ExplicitDynamicsContactAction::commonParameters();

  params.addParam<std::vector<BoundaryName>>(
      "primary", "The list of boundary IDs referring to primary sidesets");
  params.addParam<std::vector<BoundaryName>>(
      "secondary", "The list of boundary IDs referring to secondary sidesets");
  params.addParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<MooseEnum>(
      "model", ExplicitDynamicsContactAction::getModelEnum(), "The contact model to use");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addClassDescription("Sets up all objects needed for mechanical contact enforcement");
  params.addParam<std::vector<TagName>>(
      "extra_vector_tags",
      "The tag names for extra vectors that residual data should be saved into");
  params.addParam<std::vector<TagName>>(
      "absolute_value_vector_tags",
      "The tags for the vectors this residual object should fill with the "
      "absolute value of the residual contribution");

  params.addParam<VariableName>("secondary_gap_offset",
                                "Offset to gap distance from secondary side");
  params.addParam<VariableName>("mapped_primary_gap_offset",
                                "Offset to gap distance mapped from primary side");

  return params;
}

ExplicitDynamicsContactAction::ExplicitDynamicsContactAction(const InputParameters & params)
  : Action(params),
    _boundary_pairs(getParam<BoundaryName, BoundaryName>("primary", "secondary")),
    _model(getParam<MooseEnum>("model").getEnum<ExplicitDynamicsContactModel>())
{
}

void
ExplicitDynamicsContactAction::act()
{
  // proform problem checks/corrections once during the first feasible task
  if (_current_task == "add_contact_aux_variable")
  {
    if (!_problem->getDisplacedProblem())
      mooseError(
          "Contact requires updated coordinates.  Use the 'displacements = ...' parameter in the "
          "Mesh block.");

    // It is risky to apply this optimization to contact problems
    // since the problem configuration may be changed during Jacobian
    // evaluation. We therefore turn it off for all contact problems so that
    // PETSc-3.8.4 or higher will have the same behavior as PETSc-3.8.3.
    if (!_problem->isSNESMFReuseBaseSetbyUser())
      _problem->setSNESMFReuseBase(false, false);
  }

  addNodeFaceContact();

  if (_current_task == "add_aux_kernel")
  { // Add ContactPenetrationAuxAction.
    if (!_problem->getDisplacedProblem())
      mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
                 "Mesh block.");
    // Create auxiliary kernels for each contact pairs
    for (const auto & contact_pair : _boundary_pairs)
    {
      {
        InputParameters params = _factory.getValidParams("PenetrationAux");
        params.applyParameters(parameters(),
                               {"secondary_gap_offset", "mapped_primary_gap_offset", "order"});

        std::vector<VariableName> displacements =
            getParam<std::vector<VariableName>>("displacements");
        const auto order = _problem->systemBaseNonlinear()
                               .system()
                               .variable_type(displacements[0])
                               .order.get_order();

        params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};
        params.set<std::vector<BoundaryName>>("boundary") = {contact_pair.second};
        params.set<BoundaryName>("paired_boundary") = contact_pair.first;
        params.set<AuxVariableName>("variable") = "penetration";
        if (isParamValid("secondary_gap_offset"))
          params.set<std::vector<VariableName>>("secondary_gap_offset") = {
              getParam<VariableName>("secondary_gap_offset")};
        if (isParamValid("mapped_primary_gap_offset"))
          params.set<std::vector<VariableName>>("mapped_primary_gap_offset") = {
              getParam<VariableName>("mapped_primary_gap_offset")};
        params.set<bool>("use_displaced_mesh") = true;
        std::string name = _name + "_contact_" + Moose::stringify(ed_contact_auxkernel_counter++);

        _problem->addAuxKernel("PenetrationAux", name, params);
      }
    }

    addContactPressureAuxKernel();
  }

  if (_current_task == "add_contact_aux_variable")
  {
    std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
    const auto order =
        _problem->systemBaseNonlinear().system().variable_type(displacements[0]).order.get_order();
    // Add ContactPenetrationVarAction
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "penetration", var_params);
    }
    // Add ContactPressureVarAction
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "contact_pressure", var_params);
    }
    // Add nodal area contact variable
    {
      auto var_params = _factory.getValidParams("MooseVariable");
      var_params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
      var_params.set<MooseEnum>("family") = "LAGRANGE";

      _problem->addAuxVariable("MooseVariable", "nodal_area", var_params);
    }
  }

  if (_current_task == "add_user_object")
  {
    auto var_params = _factory.getValidParams("NodalArea");

    // Get secondary_boundary_vector from possibly updated set from the
    // ContactAction constructor cleanup
    const auto actions = _awh.getActions<ExplicitDynamicsContactAction>();

    std::vector<BoundaryName> secondary_boundary_vector;
    for (const auto * const action : actions)
      for (const auto j : index_range(action->_boundary_pairs))
        secondary_boundary_vector.push_back(action->_boundary_pairs[j].second);

    var_params.set<std::vector<BoundaryName>>("boundary") = secondary_boundary_vector;
    var_params.set<std::vector<VariableName>>("variable") = {"nodal_area"};

    mooseAssert(_problem, "Problem pointer is NULL");
    var_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
    var_params.set<bool>("use_displaced_mesh") = true;

    _problem->addUserObject("NodalArea",
                            "nodal_area_object_" +
                                Moose::stringify(ed_contact_userobject_counter++),
                            var_params);
  }
}

void
ExplicitDynamicsContactAction::addContactPressureAuxKernel()
{
  // Add ContactPressureAux: Only one object for all contact pairs
  // if (_formulation != ContactFormulation::MORTAR)
  const auto actions = _awh.getActions<ExplicitDynamicsContactAction>();

  // Increment counter for contact action objects
  ed_contact_action_counter++;
  // Add auxiliary kernel if we are the last contact action object.
  if (ed_contact_action_counter == actions.size())
  {
    std::vector<BoundaryName> boundary_vector;
    std::vector<BoundaryName> pair_boundary_vector;

    for (const auto * const action : actions)
      for (const auto j : index_range(action->_boundary_pairs))
      {
        boundary_vector.push_back(action->_boundary_pairs[j].second);
        pair_boundary_vector.push_back(action->_boundary_pairs[j].first);
      }

    InputParameters params = _factory.getValidParams("ContactPressureAux");
    params.applyParameters(parameters(), {"order"});

    std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
    const auto order =
        _problem->systemBaseNonlinear().system().variable_type(displacements[0]).order.get_order();

    params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});
    params.set<std::vector<BoundaryName>>("boundary") = boundary_vector;
    params.set<std::vector<BoundaryName>>("paired_boundary") = pair_boundary_vector;
    params.set<AuxVariableName>("variable") = "contact_pressure";
    params.addRequiredCoupledVar("nodal_area", "The nodal area");
    params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area"};
    params.set<bool>("use_displaced_mesh") = true;

    std::string name = _name + "_contact_pressure";
    params.set<ExecFlagEnum>("execute_on",
                             true) = {EXEC_NONLINEAR, EXEC_TIMESTEP_END, EXEC_TIMESTEP_BEGIN};
    _problem->addAuxKernel("ContactPressureAux", name, params);
  }
}

void
ExplicitDynamicsContactAction::addNodeFaceContact()
{
  if (_current_task != "add_constraint")
    return;

  std::string action_name = MooseUtils::shortName(name());
  std::vector<VariableName> displacements = getParam<std::vector<VariableName>>("displacements");
  const unsigned int ndisp = displacements.size();

  std::string constraint_type;

  constraint_type = "ExplicitDynamicsContactConstraint";

  InputParameters params = _factory.getValidParams(constraint_type);

  params.applyParameters(parameters(),
                         {"displacements",
                          "secondary_gap_offset",
                          "mapped_primary_gap_offset",
                          "primary",
                          "secondary"});

  const auto order =
      _problem->systemBaseNonlinear().system().variable_type(displacements[0]).order.get_order();

  params.set<std::vector<VariableName>>("displacements") = displacements;
  params.set<bool>("use_displaced_mesh") = true;
  params.set<MooseEnum>("order") = Utility::enum_to_string<Order>(OrderWrapper{order});

  for (const auto & contact_pair : _boundary_pairs)
  {
    params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area"};
    params.set<BoundaryName>("boundary") = contact_pair.first;
    if (isParamValid("secondary_gap_offset"))
      params.set<std::vector<VariableName>>("secondary_gap_offset") = {
          getParam<VariableName>("secondary_gap_offset")};
    if (isParamValid("mapped_primary_gap_offset"))
      params.set<std::vector<VariableName>>("mapped_primary_gap_offset") = {
          getParam<VariableName>("mapped_primary_gap_offset")};

    for (unsigned int i = 0; i < ndisp; ++i)
    {
      std::string name = action_name + "_constraint_" + Moose::stringify(contact_pair, "_") + "_" +
                         Moose::stringify(i);

      params.set<unsigned int>("component") = i;

      params.set<BoundaryName>("primary") = contact_pair.first;
      params.set<BoundaryName>("secondary") = contact_pair.second;
      params.set<NonlinearVariableName>("variable") = displacements[i];
      params.set<std::vector<VariableName>>("primary_variable") = {displacements[i]};
      params.applySpecificParameters(parameters(),
                                     {"extra_vector_tags", "absolute_value_vector_tags"});
      _problem->addConstraint(constraint_type, name, params);
    }
  }
}

MooseEnum
ExplicitDynamicsContactAction::getModelEnum()
{
  return MooseEnum("frictionless", "frictionless");
}

InputParameters
ExplicitDynamicsContactAction::commonParameters()
{
  InputParameters params = emptyInputParameters();

  params.addParam<MooseEnum>(
      "model", ExplicitDynamicsContactAction::getModelEnum(), "The contact model to use");

  return params;
}
