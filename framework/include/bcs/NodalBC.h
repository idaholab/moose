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
#include "Coupleable.h"

// libMesh
#include "numeric_vector.h"
#include "sparse_matrix.h"

class NodalBC;

template<>
InputParameters validParams<NodalBC>();

class NodalBC :
  public BoundaryCondition,
  public Coupleable
{
public:
  NodalBC(const std::string & name, InputParameters parameters);

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

protected:
  const Node * & _current_node;

  unsigned int _qp;
  VariableValue & _u;

  virtual Real computeQpResidual() = 0;
};

#endif /* NODALBC_H */
