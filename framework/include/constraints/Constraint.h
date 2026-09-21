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
   * @return Whether this constraint is enforced with exact constraints instead of a residual
   */
  virtual bool usesExactConstraints() const { return false; }

  /**
   * Add the exact constraints of this constraint. Every rank must call this, and each adds the
   * constraints of the dependent nodes it has.
   * @param dof_map The DofMap that receives the exact constraints
   */
  virtual void addExactConstraints(libMesh::DofMap & /*dof_map*/) const {}

  /**
   * @return Whether the exact constraints changed since they were last added. Not const: an
   * implementation may record the coefficients it reports on.
   */
  virtual bool exactConstraintsChanged() { return false; }

  /**
   * @return Whether to add the coupling entries of this constraint to the Jacobian sparsity. Exact
   * constraints need none: libMesh adds them.
   */
  virtual bool addCouplingEntriesToJacobian() { return !usesExactConstraints(); }

  virtual void subdomainSetup() override final
  {
    mooseError("subdomain setup for constraints is not implemented");
  }

  virtual void residualEnd() {}

protected:
  /**
   * Collects the first error a rank finds in a collective method and raises it on every rank, so
   * that one rank erroring alone cannot leave the others waiting.
   */
  class CollectiveError
  {
  public:
    CollectiveError(const Constraint & constraint) : _constraint(constraint) {}

    /**
     * Record a problem found on this rank, unless one is already recorded
     * @param message The error message
     */
    void record(const std::string & message)
    {
      if (_message.empty())
        _message = message;
    }

    /**
     * Raise the first message any rank recorded on every rank. Every rank must call this.
     */
    void raise() const;

  private:
    /// The constraint that raises the error
    const Constraint & _constraint;

    /// The first problem this rank found
    std::string _message;
  };

  /**
   * @param var_name The name of the variable
   * @return The undisplaced nonlinear system that holds the variable
   */
  NonlinearSystemBase & referenceSystem(const std::string & var_name) const;

  /**
   * Exact constraints skip the nodes this returns, so the boundary condition wins, as it does in
   * the residual formulations.
   * @param var_name The name of the variable
   * @return The IDs of the nodes where an enabled nodal boundary condition pins the variable
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
