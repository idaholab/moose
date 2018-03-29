//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORNODALBC_H
#define VECTORNODALBC_H

#include "BoundaryCondition.h"
#include "RandomInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseVariableInterface.h"

// Forward declarations
class VectorNodalBC;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<VectorNodalBC>();

/**
 * Base class for deriving any boundary condition that works at nodes
 */
class VectorNodalBC : public BoundaryCondition,
                      public RandomInterface,
                      public CoupleableMooseVariableDependencyIntermediateInterface,
                      public MooseVariableInterface<RealVectorValue>
{
public:
  VectorNodalBC(const InputParameters & parameters);

  /**
   * Gets the variable this BC is active on
   * @return the variable
   */
  virtual VectorMooseVariable & variable() override { return _var; }

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

  void setBCOnEigen(bool iseigen) { _is_eigen = iseigen; }

protected:
  VectorMooseVariable & _var;

  /// current node being processed
  const Node *& _current_node;

  /// Quadrature point index
  unsigned int _qp;
  /// Value of the unknown variable this BC is acting on
  const MooseArray<Number> & _u;

  /// Indicate whether or not the boundary condition is applied to the right
  /// hand side of eigenvalue problems
  bool _is_eigen;

  virtual Real computeQpResidual(size_t comp) = 0;

  /**
   * The user can override this function to compute the "on-diagonal"
   * Jacobian contribution for this VectorNodalBC.  If not overriden,
   * returns (1, 1, 1).
   */
  virtual RealVectorValue computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for
   * computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};

#endif /* VECTORNODALBC_H */
