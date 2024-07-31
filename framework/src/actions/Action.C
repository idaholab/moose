//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Action.h"
#include "ActionWarehouse.h"
#include "MooseApp.h"
#include "MooseTypes.h"
#include "MooseUtils.h" // remove when getBaseName is removed
#include "Builder.h"
#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "RelationshipManager.h"
#include "InputParameterWarehouse.h"
#include "ActionFactory.h"

InputParameters
Action::validParams()
{
  InputParameters params = Moose::Builder::validParams();

  params.addPrivateParam<std::string>("_moose_docs_type",
                                      "action"); // the type of syntax for documentation system
  params.addPrivateParam<std::string>("_action_name"); // the name passed to ActionFactory::create
  params.addPrivateParam<std::string>("task");
  params.addPrivateParam<std::string>("registered_identifier");
  params.addPrivateParam<std::string>("action_type");
  params.addPrivateParam<ActionWarehouse *>("awh", nullptr);

  params.addParam<std::vector<std::string>>(
      "control_tags",
      "Adds user-defined labels for accessing object parameters via control logic.");
  params.addParamNamesToGroup("control_tags", "Advanced");
  params.registerBase("Action");
  return params;
}

Action::Action(const InputParameters & parameters)
  : ParallelParamObject(
        parameters.get<std::string>("action_type"),
        parameters.get<std::string>("_action_name"),
        *parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor"),
        parameters),
    MeshMetaDataInterface(
        *parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")),
    PerfGraphInterface(
        parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")
            ->perfGraph(),
        "Action" +
            (parameters.get<std::string>("action_type") != ""
                 ? std::string("::") + parameters.get<std::string>("action_type")
                 : "") +
            (parameters.get<std::string>("_action_name") != ""
                 ? std::string("::") + parameters.get<std::string>("_action_name")
                 : "") +
            (parameters.isParamValid("task") && parameters.get<std::string>("task") != ""
                 ? std::string("::") + parameters.get<std::string>("task")
                 : "")),
    _registered_identifier(isParamValid("registered_identifier")
                               ? getParam<std::string>("registered_identifier")
                               : ""),
    _specific_task_name(_pars.isParamValid("task") ? getParam<std::string>("task") : ""),
    _awh(*getCheckedPointerParam<ActionWarehouse *>("awh")),
    _current_task(_awh.getCurrentTaskName()),
    _mesh(_awh.mesh()),
    _displaced_mesh(_awh.displacedMesh()),
    _problem(_awh.problemBase()),
    _act_timer(registerTimedSection("act", 4))
{
  if (_app.getActionFactory().currentlyConstructing() != &parameters)
    mooseError("This object was not constructed using the ActionFactory, which is not supported.");
}

void
Action::timedAct()
{
  TIME_SECTION(_act_timer);
  act();
}

bool
Action::addRelationshipManager(
    Moose::RelationshipManagerType /*input_rm_type*/,
    const InputParameters & moose_object_pars,
    std::string rm_name,
    Moose::RelationshipManagerType rm_type,
    Moose::RelationshipManagerInputParameterCallback rm_input_parameter_func,
    Moose::RMSystemType)
{
  // These need unique names
  static unsigned int unique_object_id = 0;

  auto new_name = moose_object_pars.get<std::string>("_moose_base") + '_' + name() + '_' + rm_name +
                  "_" + Moose::stringify(rm_type) + " " + std::to_string(unique_object_id);

  auto rm_params = _factory.getValidParams(rm_name);
  rm_params.set<Moose::RelationshipManagerType>("rm_type") = rm_type;

  rm_params.set<std::string>("for_whom") = name();

  // If there is a callback for setting the RM parameters let's use it
  if (rm_input_parameter_func)
    rm_input_parameter_func(moose_object_pars, rm_params);

  rm_params.set<MooseMesh *>("mesh") = _mesh.get();

  if (!rm_params.areAllRequiredParamsValid())
    mooseError("Missing required parameters for RelationshipManager " + rm_name + " for object " +
               name());

  auto rm_obj = _factory.create<RelationshipManager>(rm_name, new_name, rm_params);

  const bool added = _app.addRelationshipManager(rm_obj);

  // Delete the resources created on behalf of the RM if it ends up not being added to the App.
  if (!added)
    _factory.releaseSharedObjects(*rm_obj);
  else // we added it
    unique_object_id++;

  return added;
}

void
Action::addRelationshipManagers(Moose::RelationshipManagerType)
{
}

bool
Action::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type,
                                const InputParameters & moose_object_pars)
{
  const auto & buildable_types = moose_object_pars.getBuildableRelationshipManagerTypes();

  bool added = false;

  for (const auto & buildable_type : buildable_types)
  {
    auto & rm_name = std::get<0>(buildable_type);
    auto & rm_type = std::get<1>(buildable_type);
    auto rm_input_parameter_func = std::get<2>(buildable_type);

    added = addRelationshipManager(
                input_rm_type, moose_object_pars, rm_name, rm_type, rm_input_parameter_func) ||
            added;
  }

  return added;
}

void
Action::associateWithParameter(const std::string & param_name, InputParameters & params) const
{
  associateWithParameter(parameters(), param_name, params);
}

void
Action::associateWithParameter(const InputParameters & from_params,
                               const std::string & param_name,
                               InputParameters & params) const
{
  const auto to_hit_node = params.getHitNode();
  if (!to_hit_node || to_hit_node->isRoot())
  {
    if (const auto hit_node = from_params.getHitNode(param_name))
      params.setHitNode(*hit_node, {});
    else if (const auto hit_node = from_params.getHitNode())
      params.setHitNode(*hit_node, {});
  }
}
