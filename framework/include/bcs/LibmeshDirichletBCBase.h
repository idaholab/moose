//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "BoundaryRestrictableRequired.h"
#include "FunctionInterface.h"
#include "MooseVariableInterface.h"

/**
 * Base class for an essential boundary condition imposed as a libMesh DirichletBoundary.
 *
 * libMesh derives the prescribed value of every degree of freedom the boundary reaches by a local
 * per-entity mass-matrix projection of prescribedValue() onto the variable's boundary trace space,
 * so the result is a coefficient in whatever basis the variable uses. That is what a nodal
 * Dirichlet boundary condition misses on a modal basis such as HIERARCHIC above first order, where
 * a degree of freedom attached to a node is the coefficient of an edge mode rather than a point
 * value, and a node may carry several of them.
 *
 * libMesh also enforces the constraint, in the DofMap: Assembly condenses it out of the element
 * residuals and Jacobians, and the nonlinear solver writes the prescribed value into each iterate
 * before a residual evaluation and writes the constrained residual and Jacobian rows. This object
 * therefore computes no residual. It is a description of a constraint, registered with the DofMap
 * once, in the same way a periodic boundary is, and like one it cannot be enabled or disabled
 * during a run.
 */
class LibmeshDirichletBCBase : public MooseObject,
                               public BoundaryRestrictableRequired,
                               public FunctionInterface,
                               public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  LibmeshDirichletBCBase(const InputParameters & parameters);

  /// The private parameter that marks the parameters of one of these objects
  static const std::string marker_param;

  /**
   * Evaluate the prescribed value at a point, at a given time. libMesh's constraint machinery calls
   * this to build the boundary data it projects, so it has to be evaluable anywhere on the
   * boundary.
   * @param p The point to evaluate the value at
   * @param time The time to evaluate the value at
   * @returns The prescribed value
   */
  virtual Real prescribedValue(const libMesh::Point & p, Real time) const = 0;

  /**
   * Whether prescribedValue() depends on time, so that libMesh has to reproject it when time
   * changes
   */
  virtual bool isTimeDependent() const = 0;

  /// The variable this boundary condition constrains
  const MooseVariableFE<Real> & variable() const { return _var; }

protected:
  /// The variable this boundary condition constrains
  const MooseVariableFE<Real> & _var;
};
