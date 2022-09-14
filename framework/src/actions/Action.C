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
#include "MooseMesh.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "RelationshipManager.h"
#include "InputParameterWarehouse.h"

InputParameters
Action::validParams()
{
  InputParameters params = emptyInputParameters();

  /**
   * Add the "active" and "inactive" parameters so that all blocks in the input file can selectively
   * create white or black lists of active/inactive sub-blocks.
   */
  params.addParam<std::vector<std::string>>(
      "active",
      std::vector<std::string>({"__all__"}),
      "If specified only the blocks named will be visited and made active");
  params.addParam<std::vector<std::string>>(
      "inactive",
      std::vector<std::string>(),
      "If specified blocks matching these identifiers will be skipped.");

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
  params.registerBase("Action");
  return params;
}

Action::Action(const InputParameters & parameters)
  : ConsoleStreamInterface(
        *parameters.getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")),
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
    ParallelObject(*parameters.getCheckedPointerParam<MooseApp *>("_moose_app")),
    DataFileInterface<Action>(*this),
    _pars(parameters),
    _registered_identifier(isParamValid("registered_identifier")
                               ? getParam<std::string>("registered_identifier")
                               : ""),
    _name(getParam<std::string>("_action_name")),
    _action_type(getParam<std::string>("action_type")),
    _app(*getCheckedPointerParam<MooseApp *>("_moose_app", "In Action constructor")),
    _factory(_app.getFactory()),
    _action_factory(_app.getActionFactory()),
    _specific_task_name(_pars.isParamValid("task") ? getParam<std::string>("task") : ""),
    _awh(*getCheckedPointerParam<ActionWarehouse *>("awh")),
    _current_task(_awh.getCurrentTaskName()),
    _mesh(_awh.mesh()),
    _displaced_mesh(_awh.displacedMesh()),
    _problem(_awh.problemBase()),
    _act_timer(registerTimedSection("act", 4))
{
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

void Action::addRelationshipManagers(Moose::RelationshipManagerType) {}

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

/// DEPRECATED METHODS
std::string
Action::getShortName() const
{
  mooseDeprecated("getShortName() is deprecated.");
  return MooseUtils::shortName(_name);
}

std::string
Action::getBaseName() const
{
  mooseDeprecated("getBaseName() is deprecated.");
  return MooseUtils::baseName(_name);
}

void
Action::connectControllableParams(const std::string & parameter,
                                  const std::string & object_type,
                                  const std::string & object_name,
                                  const std::string & object_parameter) const
{
  MooseObjectParameterName primary_name(uniqueActionName(), parameter);
  auto base_type = _factory.getValidParams(object_type).get<std::string>("_moose_base");
  MooseObjectParameterName secondary_name(base_type, object_name, object_parameter);
  _app.getInputParameterWarehouse().addControllableParameterConnection(primary_name,
                                                                       secondary_name);

  const std::vector<std::string> & tags = _pars.get<std::vector<std::string>>("control_tags");
  for (const auto & tag : tags)
  {
    if (!tag.empty())
    {
      MooseObjectParameterName tagged_name(tag, _name, parameter);
      _app.getInputParameterWarehouse().addControllableParameterConnection(tagged_name,
                                                                           secondary_name);
    }
  }
}
