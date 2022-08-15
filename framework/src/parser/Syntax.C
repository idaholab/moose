//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Syntax.h"
#include "MooseUtils.h"

#include "libmesh/simple_range.h"

#include <algorithm>

Syntax::Syntax() : _actions_to_syntax_valid(false) {}

void
Syntax::registerTaskName(const std::string & task, bool should_auto_build)
{
  if (_registered_tasks.count(task) > 0)
    return;
  _tasks.addItem(task);
  _registered_tasks[task] = should_auto_build;
}

void
Syntax::registerTaskName(const std::string & task,
                         const std::string & moose_object_type,
                         bool should_auto_build)
{
  auto range = _moose_systems_to_tasks.equal_range(moose_object_type);
  for (auto it = range.first; it != range.second; ++it)
    if (it->second == task)
      return;

  if (_registered_tasks.find(task) != _registered_tasks.end())
    mooseError("A ", task, " is already registered.  Do you need to use appendTaskName instead?");

  registerTaskName(task, should_auto_build);
  _moose_systems_to_tasks.insert(std::make_pair(moose_object_type, task));
}

void
Syntax::appendTaskName(const std::string & task, const std::string & moose_object_type)
{
  if (_registered_tasks.find(task) == _registered_tasks.end())
    mooseError("A ", task, " is not a registered task name.");

  _moose_systems_to_tasks.insert(std::make_pair(moose_object_type, task));
}

void
Syntax::addDependency(const std::string & task, const std::string & pre_req)
{
  if (_registered_tasks.find(task) == _registered_tasks.end())
    mooseError("A ", task, " is not a registered task name.");

  _tasks.insertDependency(task, pre_req);
}

void
Syntax::addDependencySets(const std::string & action_sets)
{
  std::vector<std::string> sets, prev_names, tasks;
  MooseUtils::tokenize(action_sets, sets, 1, "()");

  for (unsigned int i = 0; i < sets.size(); ++i)
  {
    tasks.clear();
    MooseUtils::tokenize(sets[i], tasks, 0, ", ");
    for (unsigned int j = 0; j < tasks.size(); ++j)
    {
      // Each line should depend on each item in the previous line
      for (unsigned int k = 0; k < prev_names.size(); ++k)
        addDependency(tasks[j], prev_names[k]);
    }
    // Copy the the current items to the previous items for the next iteration
    std::swap(tasks, prev_names);
  }
}

void
Syntax::deleteTaskDependencies(const std::string & task)
{
  if (_registered_tasks.find(task) == _registered_tasks.end())
    mooseError("A ", task, " is not a registered task name.");

  _tasks.deleteDependenciesOfKey(task);
}

void
Syntax::clearTaskDependencies()
{
  _tasks.clear();
}

const std::vector<std::string> &
Syntax::getSortedTask()
{
  try
  {
    return _tasks.getSortedValues();
  }
  catch (CyclicDependencyException<std::string> & e)
  {
    const auto & cycle = e.getCyclicDependencies();
    mooseError("Cyclic dependencies detected: ", MooseUtils::join(cycle, " <- "));
  }
}

const std::vector<std::vector<std::string>> &
Syntax::getSortedTaskSet()
{
  return _tasks.getSortedValuesSets();
}

bool
Syntax::hasTask(const std::string & task) const
{
  return (_registered_tasks.find(task) != _registered_tasks.end());
}

bool
Syntax::isActionRequired(const std::string & task) const
{
  mooseDeprecated("Syntax::isActionRequired is deprecated, use shouldAutoBuild() instead");
  return shouldAutoBuild(task);
}

bool
Syntax::shouldAutoBuild(const std::string & task) const
{
  auto map_pair = _registered_tasks.find(task);
  mooseAssert(map_pair != _registered_tasks.end(), std::string("Unregistered task: ") + task);

  return map_pair->second;
}

void
Syntax::registerActionSyntax(const std::string & action,
                             const std::string & syntax,
                             const std::string & task,
                             const std::string & file,
                             int line)
{
  auto range = _syntax_to_actions.equal_range(syntax);
  for (auto it = range.first; it != range.second; ++it)
    if (it->second._action == action && it->second._task == task)
      return;

  _syntax_to_actions.insert(std::make_pair(syntax, ActionInfo{action, task}));
  _syntax_to_line.addInfo(syntax, action, task, file, line);
  _actions_to_syntax_valid = false;
}

void
Syntax::replaceActionSyntax(const std::string & action,
                            const std::string & syntax,
                            const std::string & task,
                            const std::string & file,
                            int line)
{
  _syntax_to_actions.erase(syntax);
  registerActionSyntax(action, syntax, task, file, line);
}

void
Syntax::deprecateActionSyntax(const std::string & syntax)
{
  const std::string message = "\"[" + syntax + "]\" is deprecated.";
  deprecateActionSyntax(syntax, message);
}

void
Syntax::deprecateActionSyntax(const std::string & syntax, const std::string & message)
{
  _deprecated_syntax.insert(std::make_pair(syntax, message));
}

std::string
Syntax::deprecatedActionSyntaxMessage(const std::string syntax)
{
  auto it = _deprecated_syntax.find(syntax);

  if (it != _deprecated_syntax.end())
    return it->second;
  else
    mooseError("The action syntax ", syntax, " is not deprecated");
}

bool
Syntax::isDeprecatedSyntax(const std::string & syntax) const
{
  return _deprecated_syntax.find(syntax) != _deprecated_syntax.end();
}

std::vector<std::string>
Syntax::getSyntaxByAction(const std::string & action, const std::string & task)
{
  // See if the reverse multimap has been built yet, if not build it now
  if (!_actions_to_syntax_valid)
  {
    std::transform(_syntax_to_actions.begin(),
                   _syntax_to_actions.end(),
                   std::inserter(_actions_to_syntax, _actions_to_syntax.begin()),
                   [](const std::pair<std::string, ActionInfo> pair) {
                     return std::make_pair(pair.second._action,
                                           std::make_pair(pair.first, pair.second._task));
                   });
    _actions_to_syntax_valid = true;
  }

  std::vector<std::string> syntax;
  auto it_pair = _actions_to_syntax.equal_range(action);
  for (const auto & syntax_pair : as_range(it_pair))
    // If task is blank, return all syntax, otherwise filter by task
    if (task == "" || syntax_pair.second.second == task)
      syntax.emplace_back(syntax_pair.second.first);

  return syntax;
}

std::string
Syntax::isAssociated(const std::string & real_id, bool * is_parent) const
{
  /**
   * This implementation assumes that wildcards can occur in the place of an entire token but not as
   * part of a token (i.e.  'Variables/ * /InitialConditions' is valid but not 'Variables/Partial*
   * /InitialConditions'. Since maps are ordered, a reverse traversal through the registered list
   * will always select a more specific match before a wildcard match ('*' == char(42)).
   */
  bool local_is_parent;
  if (is_parent == nullptr)
    is_parent = &local_is_parent; // Just so we don't have to keep checking below when we want to
                                  // set the value
  std::vector<std::string> real_elements, reg_elements;
  std::string return_value;

  MooseUtils::tokenize(real_id, real_elements);

  *is_parent = false;
  for (auto it = _syntax_to_actions.rbegin(); it != _syntax_to_actions.rend(); ++it)
  {
    std::string reg_id = it->first;
    if (reg_id == real_id)
    {
      *is_parent = false;
      return reg_id;
    }
    reg_elements.clear();
    MooseUtils::tokenize(reg_id, reg_elements);
    if (real_elements.size() <= reg_elements.size())
    {
      bool keep_going = true;
      for (unsigned int j = 0; keep_going && j < real_elements.size(); ++j)
      {
        if (real_elements[j] != reg_elements[j] && reg_elements[j] != std::string("*"))
          keep_going = false;
      }
      if (keep_going)
      {
        if (real_elements.size() < reg_elements.size() && !*is_parent)
        {
          // We found a parent, the longest parent in fact but we need to keep
          // looking to make sure that the real thing isn't registered
          *is_parent = true;
          return_value = reg_id;
        }
        else if (real_elements.size() == reg_elements.size())
        {
          *is_parent = false;
          return reg_id;
        }
      }
    }
  }

  if (*is_parent)
    return return_value;
  else
    return "";
}

std::pair<std::multimap<std::string, Syntax::ActionInfo>::const_iterator,
          std::multimap<std::string, Syntax::ActionInfo>::const_iterator>
Syntax::getActions(const std::string & syntax) const
{
  return _syntax_to_actions.equal_range(syntax);
}

bool
Syntax::verifyMooseObjectTask(const std::string & base, const std::string & task) const
{
  auto iters = _moose_systems_to_tasks.equal_range(base);

  for (const auto & task_it : as_range(iters))
    if (task == task_it.second)
      return true;

  return false;
}

void
Syntax::registerSyntaxType(const std::string & syntax, const std::string & type)
{
  _associated_types.insert(std::make_pair(syntax, type));
}

const std::multimap<std::string, std::string> &
Syntax::getAssociatedTypes() const
{
  return _associated_types;
}

const std::multimap<std::string, Syntax::ActionInfo> &
Syntax::getAssociatedActions() const
{
  return _syntax_to_actions;
}

FileLineInfo
Syntax::getLineInfo(const std::string & syntax,
                    const std::string & action,
                    const std::string & task) const
{
  return _syntax_to_line.getInfo(syntax, action, task);
}
