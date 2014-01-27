/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Syntax.h"
#include "MooseUtils.h"


Syntax::Syntax()
{
}

void
Syntax::registerTaskName(std::string task, bool is_required)
{
  if (_registered_tasks.find(task) != _registered_tasks.end())
    mooseError("A " << task << " is already registered.  Do you need to use appendTaskName instead?");

  _tasks.addItem(task);
  _registered_tasks[task] = is_required;
}

void
Syntax::registerTaskName(std::string task, std::string moose_object_type, bool is_required)
{
  if (_registered_tasks.find(task) != _registered_tasks.end())
    mooseError("A " << task << " is already registered.  Do you need to use appendTaskName instead?");

  _tasks.addItem(task);
  _registered_tasks[task] = is_required;
  _moose_systems_to_tasks.insert(std::make_pair(moose_object_type, task));
}

void
Syntax::appendTaskName(std::string task, std::string moose_object_type)
{
  if (_registered_tasks.find(task) == _registered_tasks.end())
    mooseError("A " << task << " is not a registered task name.");

  _moose_systems_to_tasks.insert(std::make_pair(moose_object_type, task));
}

void
Syntax::addDependency(std::string task, std::string pre_req)
{
  if (_registered_tasks.find(task) == _registered_tasks.end())
    mooseError("A " << task << " is not a registered task name.");

  _tasks.insertDependency(task, pre_req);
}

void
Syntax::addDependencySets(const std::string & action_sets)
{
  std::vector<std::string> sets, prev_names, tasks;
  MooseUtils::tokenize(action_sets, sets, 1, "()");

  for (unsigned int i=0; i<sets.size(); ++i)
  {
    tasks.clear();
    MooseUtils::tokenize(sets[i], tasks, 0, ", ");
    for (unsigned int j=0; j<tasks.size(); ++j)
    {
      // Each line should depend on each item in the previous line
      for (unsigned int k=0; k<prev_names.size(); ++k)
        addDependency(tasks[j], prev_names[k]);
    }
    // Copy the the current items to the previous items for the next iteration
    std::swap(tasks, prev_names);
  }
}

const std::vector<std::string> &
Syntax::getSortedTask()
{
  return _tasks.getSortedValues();
}

const std::vector<std::set<std::string> > &
Syntax::getSortedTaskSet()
{
  return _tasks.getSortedValuesSets();
}

bool
Syntax::hasTask(const std::string & task)
{
  return (_registered_tasks.find(task) != _registered_tasks.end());
}

bool
Syntax::isActionRequired(const std::string & task)
{
  return _registered_tasks[task];
}

void
Syntax::registerActionSyntax(const std::string & action, const std::string & syntax, const std::string & task)
{
  ActionInfo action_info;
  action_info._action = action;
  action_info._task = task;

  _associated_actions.insert(std::make_pair(syntax, action_info));
}

void
Syntax::replaceActionSyntax(const std::string & action, const std::string & syntax, const std::string & task)
{
  _associated_actions.erase(syntax);
  registerActionSyntax(action, syntax, task);
}

std::string
Syntax::getSyntaxByAction(const std::string & action, const std::string & task)
{
  std::string syntax;
  /**
   * For now we don't have a data structure that maps Actions to Syntax but this routine
   * is only used by the build full tree routine so it doesn't need to be fast.  We
   * will do a linear search for each call to this routine
   */
  for (std::multimap<std::string, ActionInfo>::const_iterator iter = _associated_actions.begin();
       iter != _associated_actions.end(); ++iter)
  {
    if (iter->second._action == action &&
        (iter->second._task == task || iter->second._task == ""))
      syntax = iter->first;
  }

  return syntax;
}

std::string
Syntax::isAssociated(const std::string & real_id, bool * is_parent)
{
  /**
   * This implementation assumes that wildcards can occur in the place of an entire token but not as part
   * of a token (i.e.  'Variables/ * /InitialConditions' is valid but not 'Variables/Partial* /InitialConditions'.
   * Since maps are ordered, a reverse traversal through the registered list will always select a more
   * specific match before a wildcard match ('*' == char(42))
   */
  bool local_is_parent;
  if (is_parent == NULL)
   is_parent = &local_is_parent;  // Just so we don't have to keep checking below when we want to set the value
  std::multimap<std::string, ActionInfo>::reverse_iterator it;
  std::vector<std::string> real_elements, reg_elements;
  std::string return_value;

  MooseUtils::tokenize(real_id, real_elements);

  *is_parent = false;
  for (it=_associated_actions.rbegin(); it != _associated_actions.rend(); ++it)
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
      for (unsigned int j=0; keep_going && j<real_elements.size(); ++j)
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
    return std::string("");
}

std::pair<std::multimap<std::string, Syntax::ActionInfo>::iterator, std::multimap<std::string, Syntax::ActionInfo>::iterator>
Syntax::getActions(const std::string & name)
{
  return _associated_actions.equal_range(name);
}

bool
Syntax::verifyMooseObjectTask(const std::string & base, const std::string & task) const
{
  std::pair<std::multimap<std::string, std::string>::const_iterator, std::multimap<std::string, std::string>::const_iterator> iters =
    _moose_systems_to_tasks.equal_range(base);

  for (std::multimap<std::string, std::string>::const_iterator it = iters.first; it != iters.second; ++it)
    if (task == it->second)
      return true;

  return false;
}
