//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDDIRICHLETBC_H
#define COUPLEDDIRICHLETBC_H

#include "NodalBC.h"

// Forward Declarations
class CoupledDirichletBC;

template <>
InputParameters validParams<CoupledDirichletBC>();

/**
 * Implements a coupled Dirichlet BC where u = alpha * some_var on the boundary.
 */
class CoupledDirichletBC : public NodalBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  /**
   * Multiplier on the boundary.
   */
  Real _alpha;

  /**
   * Holds the values at the quadrature points
   * of a coupled variable.
   */
  const VariableValue & _some_var_val;
};

#endif // COUPLEDDIRICHLETBC_H
