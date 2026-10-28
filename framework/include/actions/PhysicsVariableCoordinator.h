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
 * A finite volume variable may not be shared across two adjacent subdomains unless one of the
 * requesting Physics is defined on both: otherwise each Physics' flux kernels would treat the faces
 * between the subdomains as boundaries of their own blocks, so no flux would cross them, even
 * though the variable is defined on both sides.
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

  /**
   * Requests that a shared variable be created on \p blocks, in addition to any blocks already
   * requested for this name by another Physics
   *
   * @param nl whether the variable is a solver variable, as opposed to an auxiliary one
   * @param var_type type of the variable, e.g. 'MooseVariable' or 'MooseVariableFVReal'
   * @param var_name name of the variable
   * @param params parameters of the variable. The 'block' parameter is ignored and set to the
   * union of the requested blocks. Every other parameter must be identical to the parameters of
   * any previous request for the same name.
   * @param blocks blocks to define the variable on
   * @param requester name of the Physics making the request, for error messages
   */
  void requestVariable(bool nl,
                       const std::string & var_type,
                       const VariableName & var_name,
                       const InputParameters & params,
                       const std::vector<SubdomainName> & blocks,
                       const std::string & requester);

  /// Finds this simulation's coordinator, creating it and adding it to \p awh (using
  /// \p action_factory) if it does not exist yet
  static PhysicsVariableCoordinator & findOrCreate(ActionWarehouse & awh,
                                                   ActionFactory & action_factory);

private:
  /// The merged requests for one shared variable
  struct MergedRequest
  {
    /// Whether the variable is a solver variable, as opposed to an auxiliary one
    bool _nl;
    /// Type of the variable
    std::string _type;
    /// Parameters of the variable, from the first request
    InputParameters _params;
    /// Name of each requesting Physics, in request order, with the blocks it requested
    std::vector<std::pair<std::string, std::vector<SubdomainName>>> _requester_blocks;
  };

  /// Errors if \p req is a finite volume variable on two adjacent subdomains that no single
  /// requester is defined on
  void checkFVAdjacency(const VariableName & var_name, const MergedRequest & req) const;

  /// Pending variable requests, keyed by variable name
  std::map<VariableName, MergedRequest> _requests;
};
