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

#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "Parser.h"
#include "MooseObjectAction.h"
#include "InputFileFormatter.h"
#include "InputParameters.h"
#include "MooseMesh.h"
#include "AddVariableAction.h"
#include "AddAuxVariableAction.h"
#include "XTermConstants.h"
#include "InfixIterator.h"

ActionWarehouse::ActionWarehouse(MooseApp & app, Syntax & syntax, ActionFactory & factory) :
    _app(app),
    _syntax(syntax),
    _action_factory(factory),
    _generator_valid(false),
    _show_actions(false),
    _mesh(NULL),
    _displaced_mesh(NULL),
    _problem(NULL),
    _executioner(NULL)
{
}

ActionWarehouse::~ActionWarehouse()
{
}

void
ActionWarehouse::build()
{
  _ordered_names = _syntax.getSortedTask();
  for (std::vector<std::string>::iterator it = _ordered_names.begin(); it != _ordered_names.end(); ++it)
    buildBuildableActions(*it);
}

void
ActionWarehouse::clear()
{
  std::set<Action *> unique_action_ptrs;
  for (std::map<std::string, std::vector<Action *> >::iterator it = _action_blocks.begin(); it != _action_blocks.end(); ++it)
    for (std::vector<Action *>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); ++jt)
      unique_action_ptrs.insert(*jt);

  for (std::set<Action *>::iterator it = unique_action_ptrs.begin(); it != unique_action_ptrs.end(); ++it)
    delete *it;

  _action_blocks.clear();
  _generator_valid = false;
}

void
ActionWarehouse::addActionBlock(Action * action)
{
  /**
   * Note: This routine uses the XTerm colors directly which is not advised for general purpose output coloring.
   * Most users should prefer using Problem::colorText() which respects the "color_output" option for terminals
   * that do not support coloring.  Since this routine is intended for debugging only and runs before several
   * objects exist in the system, we are just using the constants directly.
   */

  std::string registered_identifier = action->getParams().get<std::string>("registered_identifier");
  std::set<std::string> tasks;

# if DEBUG_PARSER
  Moose::err << COLOR_DEFAULT << "Parsing Syntax:        " << GREEN   << action->name() << '\n'
             << COLOR_DEFAULT << "Building Action:       " << DEFAULT << action->type() << '\n'
             << COLOR_DEFAULT << "Registered Identifier: " << GREEN   << registered_identifier << '\n'
             << COLOR_DEFAULT << "Specific Task:         " << CYAN    << action->specificTaskName() << '\n';
# endif

  /**
   * We need to see if the current Action satisfies multiple tasks. There are a few cases to consider:
   *
   * 1. The current Action is registered with multiple syntax blocks. In this case we can only use the
   *    current instance to satisfy the specific task listed for this syntax block.  We can detect this
   *    case by inspecting whether it has a "specific task name" set in the Action instance.
   *
   * 2. This action does not have a valid "registered identifier" set in the Action instance. This means
   *    that this Action was not built by the Parser.  It was most likely created through a Meta-Action.
   *    In this case, the ActionFactory itself would have already set the task it found from the build
   *    info used to construct the Action.
   *
   * 3. The current Action is registered with only a single syntax block. In this case we can simply
   *    re-use the current instance to act and satisfy _all_ registered tasks. This is the normal
   *    case where we have a Parser-built Action that does not have a specific task name to satisfy.
   *    We will use the Action "type" to retrieve the list of tasks that this Action may satisfy.
   */
  if (action->specificTaskName() != "")           // Case 1
    tasks.insert(action->specificTaskName());
  else if (registered_identifier == "")           // Case 2
  {
    std::set<std::string> local_tasks = action->getAllTasks();
    mooseAssert(local_tasks.size() == 1, "More than one task inside of the " << action->name());
    tasks.insert(*local_tasks.begin());
  }
  else                                            // Case 3
    tasks = _action_factory.getTasksByAction(action->type());


  //TODO: Now we need to weed out the double registrations!
  for (std::set<std::string>::iterator it = tasks.begin(); it != tasks.end(); ++it)
  {
    const std::string & task = *it;

    // Some error checking
    if (!_syntax.hasTask(task))
      mooseError("A(n) " << task << " is not a registered task");

    // Make sure that the ObjectAction task and Action task are consistent
    // otherwise that means that is action was built by the wrong type
    MooseObjectAction * moa = dynamic_cast<MooseObjectAction *>(action);
    if (moa)
    {
      InputParameters mparams = moa->getObjectParams();

      if (mparams.have_parameter<std::string>("_moose_base"))
      {
        const std::string & base = mparams.get<std::string>("_moose_base");

        if (!_syntax.verifyMooseObjectTask(base, *it))
          mooseError("Task " << *it << " is not registered to build " << base << " derived objects");
      }
      else
        mooseError("Unable to locate registered base parameter for " << moa->getMooseObjectType());
    }

    // Add the current task to current action
    action->appendTask(*it);

#   if DEBUG_PARSER
    Moose::err << YELLOW << "Adding Action:         " << COLOR_DEFAULT << action->type() << " (" << YELLOW << *it << COLOR_DEFAULT << ")\n";
#   endif

    // Add it to the warehouse
    _action_blocks[*it].push_back(action);
  }
# if DEBUG_PARSER
  Moose::err << std::endl;
# endif

}

ActionIterator
ActionWarehouse::actionBlocksWithActionBegin(const std::string & task)
{
  return _action_blocks[task].begin();
}

ActionIterator
ActionWarehouse::actionBlocksWithActionEnd(const std::string & task)
{
  return _action_blocks[task].end();
}

const std::vector<Action *> &
ActionWarehouse::getActionsByName(const std::string & task) const
{
  return _action_blocks.at(task);
}

void
ActionWarehouse::buildBuildableActions(const std::string &task)
{
  if (_syntax.isActionRequired(task) && _action_blocks[task].empty())
  {
    bool ret_value = false;
    std::pair<std::multimap<std::string, std::string>::const_iterator,
              std::multimap<std::string, std::string>::const_iterator> range = _action_factory.getActionsByTask(task);
    for (std::multimap<std::string, std::string>::const_iterator it = range.first; it != range.second; ++it)
    {
      InputParameters params = _action_factory.getValidParams(it->second);
      params.set<ActionWarehouse *>("awh") = this;

      if (params.areAllRequiredParamsValid())
      {
        params.set<std::string>("registered_identifier") = "(AutoBuilt)";
        params.set<std::string>("task") = task;
        addActionBlock(_action_factory.create(it->second, "", params));
        ret_value = true;
      }
    }

    if (!ret_value)
      _unsatisfied_dependencies.insert(task);
  }
}

void
ActionWarehouse::checkUnsatisfiedActions() const
{
  std::stringstream oss;
  bool empty = true;

  for (std::set<std::string>::const_iterator i = _unsatisfied_dependencies.begin(); i != _unsatisfied_dependencies.end(); ++i)
  {
    if (_action_blocks.find(*i) == _action_blocks.end())
    {
      if (empty)
        empty = false;
      else
        oss << " ";
      oss << *i;
    }
  }

  if (!empty)
    mooseError(std::string("The following unsatisfied actions where found while setting up the MOOSE problem:\n")
               + oss.str() + "\n");
}

void
ActionWarehouse::printActionDependencySets() const
{
  /**
   * Note: This routine uses the XTerm colors directly which is not advised for general purpose output coloring.
   * Most users should prefer using Problem::colorText() which respects the "color_output" option for terminals
   * that do not support coloring.  Since this routine is intended for debugging only and runs before several
   * objects exist in the system, we are just using the constants directly.
   */
  std::ostringstream oss;

  const std::vector<std::set<std::string> > & ordered_names = _syntax.getSortedTaskSet();
  for (std::vector<std::set<std::string> >::const_iterator i = ordered_names.begin(); i != ordered_names.end(); ++i)
  {
    oss << "[DBG][ACT] (" << YELLOW;
    std::copy(i->begin(), i->end(), infix_ostream_iterator<std::string>(oss, ", "));
    oss << COLOR_DEFAULT << ")\n";

    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      if (_action_blocks.find(*j) == _action_blocks.end()) continue;
      for (std::vector<Action *>::const_iterator k = _action_blocks.at(*j).begin(); k != _action_blocks.at(*j).end(); ++k)
      {
        Action * act = *k;

        // The Syntax of the Action if it exists
        if ((*k)->name() != "")
          oss << "[DBG][ACT]\t" << GREEN << (*k)->name() << COLOR_DEFAULT << '\n';

        // The task sets
        oss << "[DBG][ACT]\t" << act->type();
        const std::set<std::string> tasks = act->getAllTasks();
        if (tasks.size() > 1)
        {
          oss << " (";
          // Break the current Action's tasks into 2 sets, those intersecting with current set and then the difference.
          std::set<std::string> intersection, difference;
          std::set_intersection(tasks.begin(), tasks.end(), i->begin(), i->end(),
                                std::inserter(intersection, intersection.end()));
          std::set_difference(tasks.begin(), tasks.end(), intersection.begin(), intersection.end(),
                              std::inserter(difference, difference.end()));

          oss << CYAN;
          std::copy(intersection.begin(), intersection.end(), infix_ostream_iterator<std::string>(oss, ", "));
          oss << MAGENTA << (difference.empty() ? "" : ", ");
          std::copy(difference.begin(), difference.end(), infix_ostream_iterator<std::string>(oss, ", "));
          oss << COLOR_DEFAULT << ")";
        }
        oss << '\n';
      }
    }
  }

  if (_show_actions)
    Moose::out << oss.str() << std::endl;
}

void
ActionWarehouse::executeAllActions()
{
  if (_show_actions)
  {
    Moose::out << "[DBG][ACT] Action Dependency Sets:\n";
    printActionDependencySets();

    Moose::out << "\n[DBG][ACT] Executing actions:" << std::endl;
  }


  for (std::vector<std::string>::iterator it = _ordered_names.begin(); it != _ordered_names.end(); ++it)
  {
    std::string task = *it;

    // Set the current task name
    _current_task = task;
    executeActionsWithAction(task);
  }
}

void
ActionWarehouse::executeActionsWithAction(const std::string & task)
{
  for (ActionIterator act_iter = actionBlocksWithActionBegin(task);
       act_iter != actionBlocksWithActionEnd(task);
       ++act_iter)
  {
    if (_show_actions)
      Moose::out << "[DBG][ACT] " << (*act_iter)->type() << " (" << YELLOW << task << COLOR_DEFAULT << ")"  << std::endl;
    (*act_iter)->act();
  }
}

void
ActionWarehouse::printInputFile(std::ostream & out)
{
  InputFileFormatter tree(false);

  std::map<std::string, std::vector<Action *> >::iterator iter;

  std::vector<Action *> ordered_actions;
  ordered_actions.clear();
  for (iter = _action_blocks.begin(); iter != _action_blocks.end(); ++iter)
    for (std::vector<Action *>::iterator j = iter->second.begin(); j != iter->second.end(); ++j)
      ordered_actions.push_back(*j);

  for (std::vector<Action* >::iterator i = ordered_actions.begin();
       i != ordered_actions.end();
       ++i)
   {
    std::string name ((*i)->name());
    const std::set<std::string> & tasks = ((*i)->getAllTasks());
    mooseAssert(!tasks.empty(), "Task list is empty");

    bool is_parent;
    if (_syntax.isAssociated(name, &is_parent) != "")
     {
      InputParameters params = (*i)->getParams();

      // TODO: Do we need to insert more nodes for each task?
      tree.insertNode(name, *tasks.begin(), true, &params);

      MooseObjectAction *moose_object_action = dynamic_cast<MooseObjectAction *>(*i);
      if (moose_object_action)
       {
        InputParameters obj_params = moose_object_action->getObjectParams();
        tree.insertNode(name, *tasks.begin(), false, &obj_params);
       }
     }
  }

  out << tree.print("");
}
