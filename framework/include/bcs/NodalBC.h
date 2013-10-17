/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef NODALBC_H
#define NODALBC_H

#include "BoundaryCondition.h"
#include "RandomInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"

// libMesh
#include "libmesh/numeric_vector.h"
#include "libmesh/sparse_matrix.h"

class NodalBC;

template<>
InputParameters validParams<NodalBC>();

/**
 * Base class for deriving any boundary condition that works at nodes
 *
 */
class NodalBC :
  public BoundaryCondition,
  public RandomInterface,
  public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  NodalBC(const std::string & name, InputParameters parameters);

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

protected:
  /// current node being processed
  const Node * & _current_node;

  /// Quadrature point index
  unsigned int _qp;
  /// Value of the unknown variable this BC is action on
  VariableValue & _u;

  virtual Real computeQpResidual() = 0;
};

#endif /* NODALBC_H */
