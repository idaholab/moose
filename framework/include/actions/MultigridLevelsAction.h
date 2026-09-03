//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

class SetupPreconditionerAction;

/**
 * Creates the coarse-level nonlinear systems (variable, kernels, boundary conditions) that a
 * Multigrid ("MG") preconditioner block needs, by cloning the fine system's variable/kernel/BC
 * action blocks at successively lower polynomial orders. Runs alongside SetupPreconditionerAction
 * on every Preconditioning block, reading that co-located action's object params, but only acts
 * when they declare an "orders" parameter (i.e. the block is a Multigrid preconditioner) --
 * every other preconditioner type is a no-op for this action.
 *
 * This is a plain Action, not a MooseObjectAction, specifically so that it carries no "type" of
 * its own: were it a MooseObjectAction sharing SetupPreconditionerAction's "type" (e.g. "MG"),
 * the framework's registerObjectTask cross-check would reject registering that object type to the
 * add_variable/add_kernel/add_bc tasks this action also runs on.
 *
 * The level systems must exist before FEProblemBase is constructed (the solver system count is
 * fixed in its constructor), so the coarse system names are registered at the "meta_action" task,
 * the earliest task in the dependency chain, by appending to the [Problem] block's "nl_sys_names"
 * parameter.
 */
class MultigridLevelsAction : public Action
{
public:
  static InputParameters validParams();

  MultigridLevelsAction(const InputParameters & params);

  virtual void act() override;

  /// The coarse-level nonlinear system name for level l (l = 0 .. orders.size() - 2), given the
  /// fine variable's name. Shared with the Multigrid preconditioner so both agree on the name
  /// without one having to search the other's action blocks for it.
  static std::string levelSystemName(const std::string & fine_var_name, unsigned int level)
  {
    return fine_var_name + "_mg_sys_" + std::to_string(level);
  }
  /// The coarse-level variable name for level l, given the fine variable's name. See
  /// levelSystemName().
  static std::string levelVariableName(const std::string & fine_var_name, unsigned int level)
  {
    return fine_var_name + "_mg_level_" + std::to_string(level);
  }

protected:
  /// Registers the coarse level system names onto the [Problem] block, synthesizing a
  /// CreateProblemAction if the input has no [Problem] block of its own
  void addLevelSystemNames();
  /// Clones the fine variable's AddVariableAction onto each coarse level, at that level's order
  void addLevelVariables();
  /// Clones every AddKernelAction targeting the fine variable onto each coarse level
  void addLevelKernels();
  /// Clones every AddBCAction targeting the fine variable onto each coarse level
  void addLevelBCs();

  /// Finds the single existing action (of type T, registered to the given task) whose object
  /// params place it on solver system fineSysName(); nullptr if none is registered
  template <typename T>
  T * findFineVariableAction(const std::string & task, const std::string & solver_sys_param) const;

  /// Finds every existing action (of type T, registered to the given task) whose object params
  /// target the fine variable through the given variable parameter name, stored as VarNameT
  /// (e.g. NonlinearVariableName for kernels, VariableName for boundary conditions -- distinct
  /// derived-string-class types, not interchangeable through InputParameters::get())
  template <typename T, typename VarNameT>
  std::vector<T *> findFineVariableActions(const std::string & task,
                                           const std::string & variable_param) const;

  /// The co-located SetupPreconditionerAction for this same Preconditioning/* block (same name),
  /// whose object params carry "orders"/"nl_sys"/etc.; nullptr should never happen, since
  /// SetupPreconditionerAction is registered to the same syntax unconditionally
  SetupPreconditionerAction * preconditionerAction() const;

  /// The vector<unsigned int> "orders" parameter of the Multigrid object this action shadows;
  /// empty if this Preconditioning/* block is not a Multigrid preconditioner
  std::vector<unsigned int> orders() const;

  /// The name of the existing nonlinear system holding the fine (highest-order) variable
  NonlinearSystemName fineSysName() const;

  /// The fine variable's name and the coarse level system/variable names; populated once, at the
  /// "meta_action" task, and reused by the later tasks
  ///@{
  std::string _fine_var_name;
  std::vector<std::string> _level_sys_names;
  std::vector<std::string> _level_var_names;
  ///@}
};
