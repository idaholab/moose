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

/**
 * Lets multiple Physics instances of the same kind (one per component - flow channels, heat
 * structures, ...) share a single, bare-named MOOSE variable (e.g. 'rhoA', 'A', 'p', 'T_solid')
 * across all of their blocks, the same way classic THM's Simulation::addSimVariable() lets
 * multiple Components share one variable.
 *
 * Each such Physics calls requestVariable() (during its own addSolverVariables()/
 * addAuxiliaryVariables()) instead of adding the variable directly; requests for the same name are
 * merged (their blocks are unioned) rather than creating separate, conflicting MOOSE variables of
 * the same name. The merged set of variables is only actually added to the problem later, once
 * every component's requests have been collected, on the 'THM:flush_shared_variables' task - a
 * new task this class' registration splices into the task graph strictly after
 * 'add_variables_physics'/'add_variable'/'add_aux_variable' and strictly before anything that
 * couples to these variables by name (materials, kernels, user objects, BCs, ICs, aux kernels).
 *
 * There is exactly one coordinator per simulation, auto-created (see findOrCreate()) by whichever
 * Physics happens to be constructed first - it is not meant to be declared by the user in an input
 * file.
 */
class THMVariableCoordinator : public Action
{
public:
  static InputParameters validParams();
  THMVariableCoordinator(const InputParameters & params);

  virtual void act() override;

  /**
   * Requests that a shared variable named \p var_name be created on \p blocks (in addition to
   * any blocks already requested for this name by another component).
   * @param nl Whether this is a nonlinear (solver) variable, as opposed to an auxiliary one
   * @param scaling Scaling factor (only meaningful, and required to be consistent, for nl = true)
   */
  void requestVariable(bool nl,
                       const VariableName & var_name,
                       const std::string & family,
                       const std::string & order,
                       Real scaling,
                       const std::vector<SubdomainName> & blocks);

  /// Finds this simulation's coordinator, creating and registering it (via \p action_factory) with
  /// \p awh if it does not already exist
  static THMVariableCoordinator & findOrCreate(ActionWarehouse & awh,
                                               ActionFactory & action_factory);

private:
  struct VariableRequest
  {
    bool _nl;
    std::string _family;
    std::string _order;
    Real _scaling;
    std::vector<SubdomainName> _blocks;
  };

  /// Pending variable requests, keyed by variable name
  std::map<VariableName, VariableRequest> _requests;
};
