//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

#include <optional>

/**
 * Base class for an essential boundary condition whose prescribed values come from libMesh's
 * Dirichlet constraint machinery.
 *
 * NodalBC-derived DirichletBCBase assigns the prescribed value to one degree of freedom per node,
 * as though that degree of freedom were a point value of the solution. On a modal basis such as
 * HIERARCHIC above first order that is wrong twice over: a degree of freedom attached to a node is
 * a bubble coefficient rather than a point value, and a node may carry several of them while
 * MooseVariableData::nodalDofIndex() names only the first.
 *
 * This family instead sources a prescribed value for every degree of freedom on the boundary from
 * NonlinearSystemBase::libmeshDirichletValues(), which libMesh derives by a local per-entity
 * mass-matrix projection of value(). A projected value is a coefficient in whatever basis is
 * current, so this family is correct on a modal and an interpolatory basis alike, and it constrains
 * every degree of freedom the boundary touches rather than the first one per node.
 *
 * Enforcement stays with MOOSE, the way it does for DirichletBCBase: this object writes its own
 * residual and Jacobian rows. libMesh's own constraint enforcement cannot be used for it, because
 * the nonlinear solver enforces constraints in their homogeneous form (see
 * DofMap::enforce_constraints_on_residual, whose \p homogeneous argument defaults to true), which
 * carries a hanging-node or periodic constraint but drops a prescribed value.
 */
class LibmeshDirichletBCBase : public NodalBC
{
public:
  static InputParameters validParams();

  LibmeshDirichletBCBase(const InputParameters & parameters);

  /**
   * Evaluate the prescribed value at a point, at a given time. libMesh's constraint machinery calls
   * this to build the boundary data it projects, so it has to be evaluable anywhere on the boundary
   * rather than only at a node.
   * @param p The point to evaluate the value at
   * @param time The time to evaluate the value at
   * @returns The prescribed value
   */
  virtual Real value(const libMesh::Point & p, Real time) const = 0;

  /// Write the prescribed values of the current node into the given solution vector
  virtual void computeValue(NumericVector<Number> & current_solution) override;

  virtual void computeResidual() override;
  virtual void computeJacobian() override;

protected:
  /**
   * Get the projected value of a degree of freedom this boundary condition prescribes
   * @param dof The global degree of freedom index
   * @returns The projected prescribed value, or nothing if libMesh's own constraint machinery
   * determines this degree of freedom and this boundary condition should leave it alone
   */
  std::optional<Real> prescribedValue(dof_id_type dof) const;

  /// Never called; this family sources its values by projection rather than pointwise
  virtual Real computeQpResidual() override;
};
