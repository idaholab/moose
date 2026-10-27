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
 * Lets multiple Physics instances (for example one per component) share a single variable across
 * all of their blocks.
 *
 * A Physics calls PhysicsBase::addSharedSolverVariable() or PhysicsBase::addSharedAuxVariable()
 * instead of adding the variable directly. Requests for the same variable name are merged, with
 * their blocks unioned, rather than creating separate and conflicting variables with the same
 * name. The merged variables are only added to the problem on the 'flush_shared_physics_variables'
 * task, which runs after every Physics has made its requests and before anything that couples to
 * variables by name (initial conditions, materials, kernels, user objects, boundary conditions,
 * auxiliary kernels).
 *
 * There is at most one coordinator per simulation. It is created on demand by
 * PhysicsBase (see findOrCreate()) and is not meant to be declared in an input file.
 */
class PhysicsVariableCoordinator : public Action
{
public:
  static InputParameters validParams();
  PhysicsVariableCoordinator(const InputParameters & params);

  virtual void act() override;

  /// The definition of a shared variable that a Physics asks for
  struct VariableRequest
  {
    /// Whether the variable is a solver variable, as opposed to an auxiliary one
    bool _nl;
    /// Finite element family
    std::string _family;
    /// Finite element order
    std::string _order;
    /// Scaling factor (solver variables only)
    Real _scaling;
    /// Solver system the variable belongs to (solver variables only)
    SolverSystemName _solver_sys;
    /// Blocks the variable is defined on
    std::vector<SubdomainName> _blocks;
  };

  /**
   * Requests that a shared variable be created on the blocks of \p request, in addition to any
   * blocks already requested for this name by another Physics
   * @param var_name name of the variable
   * @param request definition of the variable. Everything but the blocks must be identical to the
   * definition in any previous request for the same name
   * @param requester name of the Physics making the request, for error messages
   */
  void requestVariable(const VariableName & var_name,
                       const VariableRequest & request,
                       const std::string & requester);

  /// Finds this simulation's coordinator, creating it and adding it to \p awh (using
  /// \p action_factory) if it does not exist yet
  static PhysicsVariableCoordinator & findOrCreate(ActionWarehouse & awh,
                                                   ActionFactory & action_factory);

private:
  /// A request along with the first Physics that made it
  struct MergedRequest
  {
    VariableRequest _request;
    std::string _requester;
  };

  /// Pending variable requests, keyed by variable name
  std::map<VariableName, MergedRequest> _requests;
};
