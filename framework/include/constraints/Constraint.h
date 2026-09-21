//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "NeighborResidualObject.h"
#include "GeometricSearchInterface.h"

#include <set>
#include <string>

class NonlinearSystemBase;

namespace libMesh
{
class DofMap;
}

/**
 * Base class for all Constraint types
 */
class Constraint : public NeighborResidualObject, protected GeometricSearchInterface
{
public:
  static InputParameters validParams();

  Constraint(const InputParameters & parameters);

  /**
   * @return Whether this constraint is enforced with degree-of-freedom constraint rows added to
   * the DofMap instead of with a residual and a Jacobian
   */
  virtual bool usesConstraintRows() const { return false; }

  /**
   * Add the rows of this constraint to \p dof_map with
   * libMesh::DofMap::add_constraint_row(). This method is collective: it is called on every rank
   * each time libMesh rebuilds the constraints of the system, and every rank must add the rows of
   * the dependent nodes it has. It is only called on a constraint whose usesConstraintRows()
   * returns true.
   */
  virtual void addConstraintRows(libMesh::DofMap & /*dof_map*/) const {}

  /**
   * @return Whether the rows of this constraint have changed since they were last added, so that
   * libMesh must rebuild the constraints of the system
   *
   * This is the only one of these four methods that is not const, because an implementation MAY
   * RECORD the coefficients it reports on: LinearNodalConstraint answers by comparing its
   * controllable 'weights' parameter with the copy it last emitted rows from, and refreshes that
   * copy here. NonlinearSystemBase::preSolve() therefore asks every row provider once per solve
   * instead of stopping at the first one that answers true, so that no provider is left holding a
   * stale copy
   */
  virtual bool constraintRowsChanged() { return false; }

  /**
   * @return Whether the implicit geometric coupling entries of this constraint must be added to
   * the Jacobian sparsity pattern. A constraint enforced with rows needs none, because libMesh
   * already expands the sparsity pattern of a constrained degree of freedom to the degrees of
   * freedom constraining it.
   */
  virtual bool addCouplingEntriesToJacobian() { return !usesConstraintRows(); }

  virtual void subdomainSetup() override final
  {
    mooseError("subdomain setup for constraints is not implemented");
  }

  virtual void residualEnd() {}

protected:
  /**
   * Collects the first problem a rank finds inside a collective method and raises it on every
   * rank. A rank that errored on its own would leave the other ranks waiting in the next
   * collective call instead of stopping the run.
   */
  class CollectiveError
  {
  public:
    CollectiveError(const Constraint & constraint) : _constraint(constraint) {}

    /// Record \p message, unless this rank has already recorded a problem
    void record(const std::string & message)
    {
      if (_message.empty())
        _message = message;
    }

    /**
     * Raise, on every rank, the first message any rank recorded, and do nothing when no rank
     * recorded one. This is collective: every rank must reach it
     */
    void raise() const;

  private:
    /// The constraint whose communicator gathers the messages and which raises the error
    const Constraint & _constraint;

    /// The first problem this rank found, empty while it has found none
    std::string _message;
  };

  /**
   * @return The reference (undisplaced) nonlinear system the variable \p var_name lives in. It is
   * the system _sys already is, unless 'use_displaced_mesh' is true, in which case _sys is the
   * displaced copy of it and only the reference system carries the degree of freedom numbering
   * that the rows of a constraint are written in
   */
  NonlinearSystemBase & referenceSystem(const std::string & var_name) const;

  /**
   * @return The IDs of the nodes of the reference mesh at which an enabled nodal boundary
   * condition pins a degree of freedom of the variable \p var_name
   *
   * libMesh applies a degree of freedom constraint row at the linear algebra level, after the
   * residual form nodal boundary conditions, so a row on one of these nodes would override the
   * boundary condition. The residual formulations let the boundary condition win, so a constraint
   * enforced with rows leaves these nodes unconstrained instead of disagreeing with them
   */
  std::set<dof_id_type> nodesPinnedByNodalBCs(const std::string & var_name) const;

  unsigned int _i, _j;
  unsigned int _qp;
};

#define usingConstraintMembers                                                                     \
  usingMooseObjectMembers;                                                                         \
  usingUserObjectInterfaceMembers;                                                                 \
  usingTaggingInterfaceMembers;                                                                    \
  using Constraint::_i;                                                                            \
  using Constraint::_qp;                                                                           \
  using Constraint::_tid
