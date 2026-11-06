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
   * @return Whether this constraint adds exact constraints instead of a residual
   */
  virtual bool usesExactConstraints() const { return false; }

  /**
   * Add the exact constraints of this object. Every rank must call this. Each rank adds the rows
   * of the dependent nodes that it owns, and libMesh sends each row to the other ranks that need
   * it.
   * @param dof_map The DofMap that receives the exact constraints
   */
  virtual void addExactConstraints(libMesh::DofMap & /*dof_map*/) const {}

  /**
   * @return Whether the coefficients of the exact constraints changed since the last call. This
   * method is not const, because an implementation can record the coefficients that it compares.
   */
  virtual bool exactConstraintsChanged() { return false; }

  /**
   * @return Whether to add the coupling entries of this constraint to the Jacobian sparsity. Exact
   * constraints return false, because libMesh adds their entries.
   */
  virtual bool addCouplingEntriesToJacobian() { return !usesExactConstraints(); }

  virtual void subdomainSetup() override final
  {
    mooseError("subdomain setup for constraints is not implemented");
  }

  virtual void residualEnd() {}

protected:
  /**
   * Records the first error that a rank finds in a collective method, and raises it on every rank.
   * Use it for an error that only some ranks find, because an error on only some ranks leaves the
   * other ranks waiting.
   */
  class CollectiveError
  {
  public:
    CollectiveError(const Constraint & constraint) : _constraint(constraint) {}

    /**
     * Record an error message from this rank, unless this rank already recorded one
     * @param message The error message
     * @param param_name The parameter to attribute the error to, or empty for none
     */
    void record(const std::string & message, const std::string & param_name = "");

    /**
     * Raise the first recorded message of any rank on every rank. Every rank must call this.
     */
    void raise() const;

  private:
    /// The constraint that raises the error
    const Constraint & _constraint;

    /// The first error message that this rank recorded
    std::string _message;

    /// The parameter that the first error names, or empty for none
    std::string _param_name;
  };

  /**
   * @param var_name The name of the variable
   * @return The undisplaced nonlinear system that holds the variable
   */
  NonlinearSystemBase & referenceSystem(const std::string & var_name) const;

  /**
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
