//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JunctionOneToOne1PhaseAC.h"
#include "FlowChannel1PhaseAC.h"
#include "THMNames.h"
#include "THMUtils.h"
#include "AugmentSparsityBetweenElements.h"
#include "MooseUtils.h"
#include "MooseApp.h"

// Registered under the classic Component's name ('JunctionOneToOne1Phase') rather than this
// class's own name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           JunctionOneToOne1PhaseAC,
                           "JunctionOneToOne1Phase",
                           "add_user_object");
registerMooseActionAliased("ThermalHydraulicsApp",
                           JunctionOneToOne1PhaseAC,
                           "JunctionOneToOne1Phase",
                           "add_bc");
registerActionComponentAliased("ThermalHydraulicsApp",
                               JunctionOneToOne1PhaseAC,
                               "JunctionOneToOne1Phase");

InputParameters
JunctionOneToOne1PhaseAC::validParams()
{
  InputParameters params = ActionComponent::validParams();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "connections",
      "The two flow channel boundaries to connect, each in the form 'component_name:in' or "
      "'component_name:out'");
  params.addClassDescription("Junction connecting one flow channel to one other flow channel for "
                             "1-phase flow.");
  return params;
}

JunctionOneToOne1PhaseAC::JunctionOneToOne1PhaseAC(const InputParameters & params)
  : ActionComponent(params),
    _connections(getParam<std::vector<BoundaryName>>("connections")),
    _junction_uo_name(genName(name(), "junction_uo")),
    _rm_added(false)
{
  if (_connections.size() != 2)
    paramError("connections", "There must be exactly 2 connections for a one-to-one junction.");

  for (const auto & connection : _connections)
  {
    _connected_component_names.push_back(THM::parseConnectedComponentName(connection));
    _normals.push_back(THM::parseConnectionNormal(connection));
  }

  addRequiredTask("add_user_object");
  addRequiredTask("add_bc");
}

const FlowChannel1PhasePhysics &
JunctionOneToOne1PhaseAC::connectedPhysics(unsigned int i) const
{
  return _awh.getAction<FlowChannel1PhaseAC>(_connected_component_names[i]).getPhysics();
}

void
JunctionOneToOne1PhaseAC::addUserObjects()
{
  // Build the element-adjacency map for AugmentSparsityBetweenElements now that the mesh exists -
  // the two connected pipes' end elements are not mesh-adjacent, since each pipe's mesh was built
  // and combined independently.
  {
    auto & mesh = getProblem().mesh();
    const auto boundary_ids = mesh.getBoundaryIDs(_connections, true);
    std::vector<dof_id_type> connected_elem_ids;
    for (const auto & bid : boundary_ids)
    {
      dof_id_type found_elem_id = libMesh::DofObject::invalid_id;
      for (const auto & side_tuple : mesh.buildSideList())
        if (std::get<2>(side_tuple) == bid)
        {
          found_elem_id = std::get<0>(side_tuple);
          break;
        }
      if (found_elem_id == libMesh::DofObject::invalid_id)
        mooseError(
            "Junction '", name(), "': could not find an element on one of the 'connections'.");
      connected_elem_ids.push_back(found_elem_id);
    }
    _elem_map[connected_elem_ids[0]].push_back(connected_elem_ids[1]);
    _elem_map[connected_elem_ids[1]].push_back(connected_elem_ids[0]);
  }

  ExecFlagEnum execute_on(MooseUtils::getDefaultExecFlagEnum());
  execute_on = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};

  // It is assumed both connected flow channels use the same numerical flux; use the first one's.
  const std::string class_name = "ADJunctionOneToOne1PhaseUserObject";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<BoundaryName>>("boundary") = _connections;
  params.set<std::vector<Real>>("normals") = _normals;
  params.set<UserObjectName>("fluid_properties") = connectedPhysics(0).fluidPropertiesName();
  params.set<UserObjectName>("numerical_flux") = connectedPhysics(0).numericalFluxUserObjectName();
  params.set<std::vector<VariableName>>("A_elem") = {THM::AREA};
  params.set<std::vector<VariableName>>("A_linear") = {THM::AREA_LINEAR};
  params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
  params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
  params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
  params.set<std::string>("junction_name") = name();
  // No slope reconstruction (piecewise-constant / first-order) for now, matching
  // FlowChannel1PhasePhysics
  params.set<MooseEnum>("scheme") = "None";
  params.set<ExecFlagEnum>("execute_on") = execute_on;
  getProblem().addUserObject(class_name, _junction_uo_name, params);
}

void
JunctionOneToOne1PhaseAC::addWeakBCs()
{
  for (std::size_t i = 0; i < _connections.size(); i++)
    for (const auto & var : {THM::RHOA, THM::RHOUA, THM::RHOEA})
    {
      const std::string class_name = "ADJunctionOneToOne1PhaseBC";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<BoundaryName>>("boundary") = {_connections[i]};
      params.set<Real>("normal") = _normals[i];
      params.set<NonlinearVariableName>("variable") = var;
      params.set<UserObjectName>("junction_uo") = _junction_uo_name;
      params.set<unsigned int>("connection_index") = i;
      params.set<std::vector<VariableName>>("rhoA") = {THM::RHOA};
      params.set<std::vector<VariableName>>("rhouA") = {THM::RHOUA};
      params.set<std::vector<VariableName>>("rhoEA") = {THM::RHOEA};
      params.set<bool>("implicit") = THM::implicitTimeIntegrationFlag(_app);
      getProblem().addBoundaryCondition(
          class_name, genName(name(), i, var + ":junction_bc"), params);
    }
}

void
JunctionOneToOne1PhaseAC::actOnAdditionalTasks()
{
  if (_current_task == "add_bc")
    addWeakBCs();
}

void
JunctionOneToOne1PhaseAC::addRelationshipManagers(Moose::RelationshipManagerType when_type)
{
  // This virtual is called once per RelationshipManagerType category (geometric, algebraic,
  // coupling); add the (single) RM only once, on the first call. _elem_map is populated later, in
  // addUserObjects() - the RM holds a reference to it, consulted only once mesh distribution
  // actually needs it, well after the mesh (and thus _elem_map) exist.
  if (_rm_added || when_type != Moose::RelationshipManagerType::GEOMETRIC)
    return;
  _rm_added = true;

  const std::string class_name = "AugmentSparsityBetweenElements";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Moose::RelationshipManagerType>("rm_type") =
      Moose::RelationshipManagerType::COUPLING | Moose::RelationshipManagerType::ALGEBRAIC |
      Moose::RelationshipManagerType::GEOMETRIC;
  params.set<std::string>("for_whom") = name();
  params.set<MooseMesh *>("mesh") = _awh.getMesh().get();
  params.set<std::map<dof_id_type, std::vector<dof_id_type>> *>("_elem_map") = &_elem_map;
  auto rm =
      _factory.create<RelationshipManager>(class_name, genName(name(), "sparsity_rm"), params);
  if (!_app.addRelationshipManager(rm))
    _factory.releaseSharedObjects(*rm);
}
