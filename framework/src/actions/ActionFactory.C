//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ActionFactory.h"
#include "MooseApp.h"
#include "InputParameterWarehouse.h"
#include "MooseObjectAction.h"

ActionFactory::ActionFactory(MooseApp & app) : _app(app) {}

ActionFactory::~ActionFactory() {}

void
ActionFactory::reg(std::shared_ptr<RegistryEntryBase> obj)
{
  const std::string & name = obj->_classname;
  const std::string & task = obj->_name;

  auto key = std::make_pair(name, task);
  if (_current_objs.count(key) > 0)
    return;
  _current_objs.insert(key);

  BuildInfo build_info{obj, task};
  _name_to_build_info.insert(std::make_pair(name, build_info));
  _task_to_action_map.insert(std::make_pair(task, name));
  _tasks.insert(task);
  _name_to_line.addInfo(name, task, obj->_file, obj->_line);
}

std::shared_ptr<Action>
ActionFactory::create(const std::string & action,
                      const std::string & full_action_name,
                      InputParameters & incoming_parser_params)
{
  std::string action_name = MooseUtils::shortName(full_action_name);
  incoming_parser_params.addPrivateParam("_moose_app", &_app);
  incoming_parser_params.addPrivateParam("action_type", action);
  std::pair<ActionFactory::iterator, ActionFactory::iterator> iters;

  std::string unique_action_name =
      action + incoming_parser_params.get<std::string>("task") + full_action_name;
  // Create the actual parameters object that the object will reference
  InputParameters & action_params = _app.getInputParameterWarehouse().addInputParameters(
      unique_action_name, incoming_parser_params, 0, {});

  if (!action_params.getHitNode())
  {
    // If we currently are in an action, that means that we're creating an
    // action from within an action. Associate the action creating this one
    // with the new action's parameters so that errors can be associated with it
    if (const auto hit_node = _app.getCurrentActionHitNode())
      action_params.setHitNode(*hit_node, {});
    // Don't have one, so just use the root
    else
      action_params.setHitNode(*_app.parser().root(), {});
  }

  // Check and finalize the parameters
  action_params.finalize(action_name);

  iters = _name_to_build_info.equal_range(action);
  BuildInfo * build_info = &(iters.first->second);
  if (!build_info)
    mooseError(
        std::string("Unable to find buildable Action from supplied InputParameters Object for ") +
        action_name);

  // Add the name to the parameters
  action_params.set<std::string>("_action_name") = action_name;
  action_params.set<std::string>("_unique_action_name") = unique_action_name;

  // Create the object
  _currently_constructing.push_back(&action_params);
  std::shared_ptr<Action> action_obj = build_info->_obj_pointer->buildAction(action_params);
  _currently_constructing.pop_back();

  if (action_params.get<std::string>("task") == "")
    action_obj->appendTask(build_info->_task);

  return action_obj;
}

InputParameters
ActionFactory::getValidParams(const std::string & name)
{
  /**
   * If an Action is registered more than once, it'll appear in the _name_to_build_info data
   * structure multiple times.  The actual parameters function remains the same however
   * so we can safely use the first instance
   */
  ActionFactory::iterator iter = _name_to_build_info.find(name);

  if (iter == _name_to_build_info.end())
    mooseError(std::string("A '") + name + "' is not a registered Action\n\n");

  InputParameters params = iter->second._obj_pointer->buildParameters();
  params.addPrivateParam("_moose_app", &_app);
  params.addPrivateParam<ActionWarehouse *>("awh", &_app.actionWarehouse());

  return params;
}

std::string
ActionFactory::getTaskName(const std::string & action)
{
  // We are returning only the first found instance here
  std::multimap<std::string, BuildInfo>::iterator iter = _name_to_build_info.find(action);

  if (iter != _name_to_build_info.end())
    return iter->second._task;
  else
    return "";
}

ActionFactory::iterator
ActionFactory::begin()
{
  return _name_to_build_info.begin();
}

ActionFactory::const_iterator
ActionFactory::begin() const
{
  return _name_to_build_info.begin();
}

ActionFactory::iterator
ActionFactory::end()
{
  return _name_to_build_info.end();
}

ActionFactory::const_iterator
ActionFactory::end() const
{
  return _name_to_build_info.end();
}

std::pair<std::multimap<std::string, std::string>::const_iterator,
          std::multimap<std::string, std::string>::const_iterator>
ActionFactory::getActionsByTask(const std::string & task) const
{
  return _task_to_action_map.equal_range(task);
}

std::set<std::string>
ActionFactory::getTasksByAction(const std::string & action) const
{
  std::set<std::string> tasks;

  std::pair<std::multimap<std::string, ActionFactory::BuildInfo>::const_iterator,
            std::multimap<std::string, ActionFactory::BuildInfo>::const_iterator>
      iters = _name_to_build_info.equal_range(action);
  for (std::multimap<std::string, ActionFactory::BuildInfo>::const_iterator it = iters.first;
       it != iters.second;
       ++it)
    tasks.insert(it->second._task);

  return tasks;
}

const InputParameters *
ActionFactory::currentlyConstructing() const
{
  return _currently_constructing.size() ? _currently_constructing.back() : nullptr;
}

FileLineInfo
ActionFactory::getLineInfo(const std::string & name, const std::string & task) const
{
  return _name_to_line.getInfo(name, task);
}
