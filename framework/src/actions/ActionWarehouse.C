//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
#include "FEProblem.h"
#include "MemoryUtils.h"
#include "InputParameterWarehouse.h"

#include "libmesh/simple_range.h"

ActionWarehouse::ActionWarehouse(MooseApp & app, Syntax & syntax, ActionFactory & factory)
  : ConsoleStreamInterface(app),
    _app(app),
    _syntax(syntax),
    _action_factory(factory),
    _generator_valid(false),
    _show_action_dependencies(false),
    _show_actions(false),
    _show_parser(false),
    _current_action(nullptr),
    _mesh(nullptr),
    _displaced_mesh(nullptr)
{
}

ActionWarehouse::~ActionWarehouse() {}

void
ActionWarehouse::setFinalTask(const std::string & task)
{
  if (!_syntax.hasTask(task))
    mooseError("cannot use unregistered task '", task, "' as final task");
  _final_task = task;
}

void
ActionWarehouse::build()
{
  _ordered_names = _syntax.getSortedTask();
  for (const auto & name : _ordered_names)
    buildBuildableActions(name);
}

void
ActionWarehouse::clear()
{
  for (auto & ptr : _all_ptrs)
    ptr.reset();

  _action_blocks.clear();
  _generator_valid = false;

  // Due to the way ActionWarehouse is cleaned up (see MooseApp's
  // destructor) we must guarantee that ActionWarehouse::clear()
  // releases all the resources which have to be released _before_ the
  // _comm object owned by the MooseApp is destroyed.
  _problem.reset();
  _displaced_mesh.reset();
  _mesh.reset();
}

void
ActionWarehouse::addActionBlock(std::shared_ptr<Action> action)
{
  /**
   * Note: This routine uses the XTerm colors directly which is not advised for general purpose
   * output coloring. Most users should prefer using Problem::colorText() which respects the
   * "color_output" option for terminals that do not support coloring.  Since this routine is
   * intended for debugging only and runs before several objects exist in the system, we are just
   * using the constants directly.
   */
  std::string registered_identifier =
      action->parameters().get<std::string>("registered_identifier");
  std::set<std::string> tasks;

  if (_show_parser)
    Moose::err << COLOR_DEFAULT << "Parsing Syntax:        " << COLOR_GREEN << action->name()
               << '\n'
               << COLOR_DEFAULT << "Building Action:       " << COLOR_DEFAULT << action->type()
               << '\n'
               << COLOR_DEFAULT << "Registered Identifier: " << COLOR_GREEN << registered_identifier
               << '\n'
               << COLOR_DEFAULT << "Specific Task:         " << COLOR_CYAN
               << action->specificTaskName() << std::endl;

  /**
   * We need to see if the current Action satisfies multiple tasks. There are a few cases to
   * consider:
   *
   * 1. The current Action is registered with multiple syntax blocks. In this case we can only use
   *    the current instance to satisfy the specific task listed for this syntax block.  We can
   *    detect this case by inspecting whether it has a "specific task name" set in the Action
   *    instance.
   *
   * 2. This action does not have a valid "registered identifier" set in the Action instance. This
   *    means that this Action was not built by the Parser.  It was most likely created through a
   *    Meta-Action (See Case 3 for exception). In this case, the ActionFactory itself would have
   *    already set the task it found from the build info used to construct the Action so we'd
   *    arbitrarily satisfy a single task in this case.
   *
   * 3. The current Action is registered with only a single syntax block. In this case we can simply
   *    re-use the current instance to act and satisfy _all_ registered tasks. This is the normal
   *    case where we have a Parser-built Action that does not have a specific task name to satisfy.
   *    We will use the Action "type" to retrieve the list of tasks that this Action may satisfy.
   */
  if (action->specificTaskName() != "") // Case 1
    tasks.insert(action->specificTaskName());
  else if (registered_identifier == "" &&
           _syntax.getNonDeprecatedSyntaxByAction(action->type()).size() > 1) // Case 2
  {
    std::set<std::string> local_tasks = action->getAllTasks();
    mooseAssert(local_tasks.size() == 1, "More than one task inside of the " << action->name());
    tasks.insert(*local_tasks.begin());
  }
  else // Case 3
    tasks = _action_factory.getTasksByAction(action->type());

  // TODO: Now we need to weed out the double registrations!
  for (const auto & task : tasks)
  {
    // Some error checking
    if (!_syntax.hasTask(task))
      mooseError("A(n) ", task, " is not a registered task");

    // Make sure that the ObjectAction task and Action task are consistent
    // otherwise that means that is action was built by the wrong type
    std::shared_ptr<MooseObjectAction> moa = std::dynamic_pointer_cast<MooseObjectAction>(action);
    if (moa.get())
    {
      const InputParameters & mparams = moa->getObjectParams();

      if (mparams.have_parameter<std::string>("_moose_base"))
      {
        const std::string & base = mparams.get<std::string>("_moose_base");
        if (!_syntax.verifyMooseObjectTask(base, task))
          mooseError("Task ", task, " is not registered to build ", base, " derived objects");
      }
      else
        mooseError("Unable to locate registered base parameter for ", moa->getMooseObjectType());
    }

    // Add the current task to current action
    action->appendTask(task);

    if (_show_parser)
      Moose::err << COLOR_YELLOW << "Adding Action:         " << COLOR_DEFAULT << action->type()
                 << " (" << COLOR_YELLOW << task << COLOR_DEFAULT << ")" << std::endl;

    // Add it to the warehouse
    _action_blocks[task].push_back(action.get());
  }
  _all_ptrs.push_back(action);

  if (_show_parser)
    Moose::err << std::endl;
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

const std::vector<std::shared_ptr<Action>> &
ActionWarehouse::allActionBlocks() const
{
  return _all_ptrs;
}

const std::list<Action *> &
ActionWarehouse::getActionListByName(const std::string & task) const
{
  const auto it = _action_blocks.find(task);
  if (it == _action_blocks.end())
    return _empty_action_list;
  else
    return it->second;
}

bool
ActionWarehouse::hasActions(const std::string & task) const
{
  auto it = _action_blocks.find(task);
  return it != _action_blocks.end() && !it->second.empty();
}

void
ActionWarehouse::buildBuildableActions(const std::string & task)
{
  if (_syntax.shouldAutoBuild(task) && _action_blocks[task].empty())
  {
    bool ret_value = false;
    auto it_pair = _action_factory.getActionsByTask(task);
    for (const auto & action_pair : as_range(it_pair))
    {
      InputParameters params = _action_factory.getValidParams(action_pair.second);
      params.set<ActionWarehouse *>("awh") = this;

      if (params.areAllRequiredParamsValid())
      {
        params.set<std::string>("registered_identifier") = "(AutoBuilt)";
        addActionBlock(_action_factory.create(action_pair.second, "", params));
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

  for (const auto & udep : _unsatisfied_dependencies)
  {
    if (_action_blocks.find(udep) == _action_blocks.end())
    {
      if (empty)
        empty = false;
      else
        oss << " ";
      oss << udep;
    }
  }

  if (!empty)
    mooseError(
        std::string(
            "The following unsatisfied actions where found while setting up the MOOSE problem:\n") +
        oss.str() + "\n");
}

void
ActionWarehouse::printActionDependencySets() const
{
  /**
   * Note: This routine uses the XTerm colors directly which is not advised for general purpose
   * output coloring.
   * Most users should prefer using Problem::colorText() which respects the "color_output" option
   * for terminals
   * that do not support coloring.  Since this routine is intended for debugging only and runs
   * before several
   * objects exist in the system, we are just using the constants directly.
   */
  std::ostringstream oss;

  const auto & ordered_names = _syntax.getSortedTaskSet();
  for (const auto & task_vector : ordered_names)
  {
    oss << "[DBG][ACT] (" << COLOR_YELLOW;
    std::copy(
        task_vector.begin(), task_vector.end(), infix_ostream_iterator<std::string>(oss, ", "));
    oss << COLOR_DEFAULT << ")\n";

    std::set<std::string> task_set(task_vector.begin(), task_vector.end());
    for (const auto & task : task_set)
    {
      if (_action_blocks.find(task) == _action_blocks.end())
        continue;

      for (const auto & act : _action_blocks.at(task))
      {
        // The Syntax of the Action if it exists
        if (act->name() != "")
          oss << "[DBG][ACT]\t" << COLOR_GREEN << act->name() << COLOR_DEFAULT << '\n';

        // The task sets
        oss << "[DBG][ACT]\t" << act->type();
        const std::set<std::string> tasks = act->getAllTasks();
        if (tasks.size() > 1)
        {
          oss << " (";
          // Break the current Action's tasks into 2 sets, those intersecting with current set and
          // then the difference.
          std::set<std::string> intersection, difference;
          std::set_intersection(tasks.begin(),
                                tasks.end(),
                                task_set.begin(),
                                task_set.end(),
                                std::inserter(intersection, intersection.end()));
          std::set_difference(tasks.begin(),
                              tasks.end(),
                              intersection.begin(),
                              intersection.end(),
                              std::inserter(difference, difference.end()));

          oss << COLOR_CYAN;
          std::copy(intersection.begin(),
                    intersection.end(),
                    infix_ostream_iterator<std::string>(oss, ", "));
          oss << COLOR_MAGENTA << (difference.empty() ? "" : ", ");
          std::copy(
              difference.begin(), difference.end(), infix_ostream_iterator<std::string>(oss, ", "));
          oss << COLOR_DEFAULT << ")";
        }
        oss << '\n';
      }
    }
  }

  if (_show_action_dependencies)
    _console << oss.str() << std::endl;
}

void
ActionWarehouse::executeAllActions()
{
  _completed_tasks.clear();

  if (_show_action_dependencies)
  {
    _console << "[DBG][ACT] Action Dependency Sets:\n";
    printActionDependencySets();

    _console << "\n[DBG][ACT] Executing actions:" << std::endl;
  }

  for (const auto & task : _ordered_names)
  {
    executeActionsWithAction(task);
    std::scoped_lock lock(_completed_tasks_mutex);
    _completed_tasks.insert(task);
    if (_final_task != "" && task == _final_task)
      break;
  }

  if (_show_actions)
  {
    MemoryUtils::Stats stats;
    MemoryUtils::getMemoryStats(stats);
    auto usage =
        MemoryUtils::convertBytes(stats._physical_memory, MemoryUtils::MemUnits::Megabytes);
    _console << "[DBG][ACT] Finished executing all actions with memory usage " << usage << "MB\n"
             << std::endl;
  }
}

void
ActionWarehouse::executeActionsWithAction(const std::string & task)
{
  // Set the current task name
  _current_task = task;

  for (auto it = actionBlocksWithActionBegin(task); it != actionBlocksWithActionEnd(task); ++it)
  {
    _current_action = *it;

    if (_show_actions)
    {
      MemoryUtils::Stats stats;
      MemoryUtils::getMemoryStats(stats);
      auto usage =
          MemoryUtils::convertBytes(stats._physical_memory, MemoryUtils::MemUnits::Megabytes);
      _console << "[DBG][ACT] "
               << "TASK (" << COLOR_YELLOW << std::setw(24) << task << COLOR_DEFAULT << ") "
               << "TYPE (" << COLOR_YELLOW << std::setw(32) << _current_action->type()
               << COLOR_DEFAULT << ") "
               << "NAME (" << COLOR_YELLOW << std::setw(16) << _current_action->name()
               << COLOR_DEFAULT << ") Memory usage " << usage << "MB" << std::endl;
    }

    _current_action->timedAct();
  }

  _current_action = nullptr;
}

void
ActionWarehouse::printInputFile(std::ostream & out)
{
  InputFileFormatter tree(false);

  std::map<std::string, std::vector<Action *>>::iterator iter;

  std::vector<Action *> ordered_actions;
  for (const auto & block : _action_blocks)
    for (const auto & act : block.second)
      ordered_actions.push_back(act);

  for (const auto & act : ordered_actions)
  {
    std::string name;
    if (act->parameters().blockFullpath() != "")
      name = act->parameters().blockFullpath();
    else
      name = act->name();
    const std::set<std::string> & tasks = act->getAllTasks();
    mooseAssert(!tasks.empty(), "Task list is empty");

    bool is_parent;
    if (_syntax.isAssociated(name, &is_parent) != "")
    {
      const auto & all_params = _app.getInputParameterWarehouse().getInputParameters();
      InputParameters & params = *(all_params.find(act->uniqueActionName())->second.get());

      // temporarily allow input parameter copies required by the input file formatter
      params.allowCopy(true);

      // TODO: Do we need to insert more nodes for each task?
      tree.insertNode(name, *tasks.begin(), true, &params);
      params.allowCopy(false);

      MooseObjectAction * moose_object_action = dynamic_cast<MooseObjectAction *>(act);
      if (moose_object_action)
      {
        InputParameters obj_params = moose_object_action->getObjectParams();
        tree.insertNode(name, *tasks.begin(), false, &obj_params);
      }
    }
  }

  out << tree.print("");
}

std::shared_ptr<FEProblem>
ActionWarehouse::problem()
{
  mooseDeprecated(
      "ActionWarehouse::problem() is deprecated, please use ActionWarehouse::problemBase() \n");
  return std::dynamic_pointer_cast<FEProblem>(_problem);
}

std::string
ActionWarehouse::getCurrentActionName() const
{
  return getCurrentAction()->parameters().getHitNode()->fullpath();
}

const std::string &
ActionWarehouse::getMooseAppName()
{
  return _app.name();
}

bool
ActionWarehouse::hasTask(const std::string & task) const
{
  return _action_factory.isRegisteredTask(task);
}

bool
ActionWarehouse::isTaskComplete(const std::string & task) const
{
  if (!hasTask(task))
    mooseError("\"", task, "\" is not a registered task.");
  std::scoped_lock lock(_completed_tasks_mutex);
  return _completed_tasks.count(task);
}
