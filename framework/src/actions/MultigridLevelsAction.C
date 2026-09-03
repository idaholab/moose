//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultigridLevelsAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "SetupPreconditionerAction.h"
#include "AddVariableAction.h"
#include "AddKernelAction.h"
#include "AddBCAction.h"
#include "CreateProblemAction.h"

#include "libmesh/string_to_enum.h"

registerMooseAction("MooseApp", MultigridLevelsAction, "meta_action");
registerMooseAction("MooseApp", MultigridLevelsAction, "add_variable");
registerMooseAction("MooseApp", MultigridLevelsAction, "add_kernel");
registerMooseAction("MooseApp", MultigridLevelsAction, "add_bc");

InputParameters
MultigridLevelsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Creates the coarse-level nonlinear systems a Multigrid ('MG') preconditioner needs, by "
      "cloning the fine system's variable/kernel/BC action blocks at lower polynomial orders. "
      "A no-op on any Preconditioning/* block whose object is not a Multigrid preconditioner.");
  return params;
}

MultigridLevelsAction::MultigridLevelsAction(const InputParameters & params) : Action(params) {}

SetupPreconditionerAction *
MultigridLevelsAction::preconditionerAction() const
{
  for (auto it = _awh.actionBlocksWithActionBegin("add_preconditioning");
       it != _awh.actionBlocksWithActionEnd("add_preconditioning");
       ++it)
  {
    auto * candidate = dynamic_cast<SetupPreconditionerAction *>(*it);
    if (candidate && candidate->name() == name())
      return candidate;
  }

  return nullptr;
}

std::vector<unsigned int>
MultigridLevelsAction::orders() const
{
  auto * p = preconditionerAction();
  if (!p || !p->getObjectParams().isParamValid("orders"))
    return {};
  return p->getObjectParams().get<std::vector<unsigned int>>("orders");
}

NonlinearSystemName
MultigridLevelsAction::fineSysName() const
{
  auto * p = preconditionerAction();
  if (p && p->getObjectParams().isParamValid("nl_sys"))
    return p->getObjectParams().get<NonlinearSystemName>("nl_sys");
  return NonlinearSystemName("nl0");
}

template <typename T>
T *
MultigridLevelsAction::findFineVariableAction(const std::string & task,
                                              const std::string & solver_sys_param) const
{
  const auto fine_sys = fineSysName();

  T * found = nullptr;
  for (auto it = _awh.actionBlocksWithActionBegin(task); it != _awh.actionBlocksWithActionEnd(task);
       ++it)
  {
    auto * candidate = dynamic_cast<T *>(*it);
    if (!candidate)
      continue;

    const auto & obj_params = candidate->getObjectParams();
    if (obj_params.isParamValid(solver_sys_param) &&
        obj_params.template get<SolverSystemName>(solver_sys_param) != fine_sys)
      continue;

    if (found)
      mooseError("Multigrid preconditioner: found more than one variable on nonlinear system '",
                 fine_sys,
                 "'. Multigrid requires the fine system to hold exactly one variable.");
    found = candidate;
  }

  return found;
}

template <typename T, typename VarNameT>
std::vector<T *>
MultigridLevelsAction::findFineVariableActions(const std::string & task,
                                               const std::string & variable_param) const
{
  std::vector<T *> found;
  for (auto it = _awh.actionBlocksWithActionBegin(task); it != _awh.actionBlocksWithActionEnd(task);
       ++it)
  {
    auto * candidate = dynamic_cast<T *>(*it);
    if (!candidate)
      continue;

    const auto & obj_params = candidate->getObjectParams();
    if (obj_params.isParamValid(variable_param) &&
        obj_params.template get<VarNameT>(variable_param) == _fine_var_name)
      found.push_back(candidate);
  }

  return found;
}

void
MultigridLevelsAction::act()
{
  const auto lvl_orders = orders();
  if (lvl_orders.empty())
    // Not a Multigrid ("MG") preconditioner block; nothing for this action to do
    return;

  if (lvl_orders.size() < 2)
    mooseError("Multigrid preconditioner: 'orders' must list at least two levels.");
  if (lvl_orders.front() != 1)
    mooseError("Multigrid preconditioner: 'orders' must start at 1 (LAGRANGE p=1).");
  for (const auto i : make_range(std::size_t(1), lvl_orders.size()))
    if (lvl_orders[i] <= lvl_orders[i - 1])
      mooseError("Multigrid preconditioner: 'orders' must be strictly ascending.");

  if (_current_task == "meta_action")
    addLevelSystemNames();
  else if (_current_task == "add_variable")
    addLevelVariables();
  else if (_current_task == "add_kernel")
    addLevelKernels();
  else if (_current_task == "add_bc")
    addLevelBCs();
}

void
MultigridLevelsAction::addLevelSystemNames()
{
  const auto lvl_orders = orders();

  auto * fine_var_action = findFineVariableAction<AddVariableAction>("add_variable", "solver_sys");
  if (!fine_var_action)
    mooseError("Multigrid preconditioner: could not find the fine variable on nonlinear system '",
               fineSysName(),
               "'. The fine system's variable must be declared in the [Variables] block.");

  _fine_var_name = fine_var_action->name();

  const auto & fine_pars = fine_var_action->parameters();
  const auto fine_order =
      libMesh::Utility::string_to_enum<Order>(std::string(fine_pars.get<MooseEnum>("order")));
  if (static_cast<unsigned int>(fine_order) != lvl_orders.back())
    mooseError("Multigrid preconditioner: the last entry of 'orders' (",
               lvl_orders.back(),
               ") must equal the fine variable's order (",
               static_cast<unsigned int>(fine_order),
               ").");

  // One new coarse-level system per entry of 'orders' except the last, which is the existing
  // fine system
  _level_sys_names.clear();
  _level_var_names.clear();
  for (const auto l : make_range(lvl_orders.size() - 1))
  {
    _level_sys_names.push_back(levelSystemName(_fine_var_name, l));
    _level_var_names.push_back(levelVariableName(_fine_var_name, l));
  }

  // Reach the [Problem] block's params exactly as CreateProblemDefaultAction::act() does, so our
  // appended names are in place before FEProblemBase is constructed. Synthesize the action if the
  // input has no [Problem] block of its own.
  auto * problem_action = const_cast<CreateProblemAction *>(
      _awh.getActionByTask<CreateProblemAction>("create_problem"));
  if (!problem_action)
  {
    InputParameters params = _action_factory.getValidParams("CreateProblemAction");
    auto action = _action_factory.create("CreateProblemAction", "MultigridLevelsProblem", params);
    _awh.addActionBlock(action);
    problem_action = static_cast<CreateProblemAction *>(action.get());
  }

  auto & problem_params = problem_action->getObjectParams();
  auto nl_sys_names = problem_params.get<std::vector<NonlinearSystemName>>("nl_sys_names");
  for (const auto & sys_name : _level_sys_names)
    nl_sys_names.push_back(sys_name);
  problem_params.set<std::vector<NonlinearSystemName>>("nl_sys_names") = nl_sys_names;
}

void
MultigridLevelsAction::addLevelVariables()
{
  auto * fine_var_action = findFineVariableAction<AddVariableAction>("add_variable", "solver_sys");
  if (!fine_var_action)
    return;

  const auto & fine_pars = fine_var_action->parameters();
  const auto lvl_orders = orders();

  for (const auto l : index_range(_level_sys_names))
  {
    InputParameters params = _action_factory.getValidParams("AddVariableAction");
    params.set<std::string>("type") = "MooseVariableBase";
    params.set<MooseEnum>("family") = fine_pars.get<MooseEnum>("family");
    params.set<MooseEnum>("order") =
        libMesh::Utility::enum_to_string<Order>(static_cast<Order>(lvl_orders[l]));

    auto action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create("AddVariableAction", _level_var_names[l], params));
    action->getObjectParams().set<SolverSystemName>("solver_sys") = _level_sys_names[l];

    _awh.addActionBlock(action);
  }
}

void
MultigridLevelsAction::addLevelKernels()
{
  if (_level_sys_names.empty())
    return;

  auto fine_kernel_actions =
      findFineVariableActions<AddKernelAction, NonlinearVariableName>("add_kernel", "variable");

  for (auto * fine_kernel : fine_kernel_actions)
    for (const auto l : index_range(_level_sys_names))
    {
      InputParameters params = _action_factory.getValidParams("AddKernelAction");
      params.set<std::string>("type") = fine_kernel->getMooseObjectType();

      const auto clone_name = fine_kernel->name() + "_mg_level_" + std::to_string(l);
      auto action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create("AddKernelAction", clone_name, params));

      action->getObjectParams() = fine_kernel->getObjectParams();
      action->getObjectParams().set<NonlinearVariableName>("variable") = _level_var_names[l];

      _awh.addActionBlock(action);
    }
}

void
MultigridLevelsAction::addLevelBCs()
{
  if (_level_sys_names.empty())
    return;

  auto fine_bc_actions =
      findFineVariableActions<AddBCAction, NonlinearVariableName>("add_bc", "variable");

  for (auto * fine_bc : fine_bc_actions)
    for (const auto l : index_range(_level_sys_names))
    {
      InputParameters params = _action_factory.getValidParams("AddBCAction");
      params.set<std::string>("type") = fine_bc->getMooseObjectType();

      const auto clone_name = fine_bc->name() + "_mg_level_" + std::to_string(l);
      auto action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create("AddBCAction", clone_name, params));

      action->getObjectParams() = fine_bc->getObjectParams();
      action->getObjectParams().set<NonlinearVariableName>("variable") = _level_var_names[l];

      _awh.addActionBlock(action);
    }
}
