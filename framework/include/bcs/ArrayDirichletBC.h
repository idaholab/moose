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

#ifndef ARRAYDIRICHLETBC_H
#define ARRAYDIRICHLETBC_H

#include "ArrayNodalBC.h"

class ArrayDirichletBC;

template<>
InputParameters validParams<ArrayDirichletBC>();

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value in the node
 */
class ArrayDirichletBC : public ArrayNodalBC
{
public:
  ArrayDirichletBC(const InputParameters & parameters);

protected:
  virtual void computeQpResidual() override;

  /// The value for this BC
  Eigen::VectorXd _value;
};

#endif /* ARRAYDIRICHLETBC_H */
