//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDNEUMANNBC_H
#define COUPLEDNEUMANNBC_H

#include "IntegratedBC.h"

// Forward Declarations
class CoupledNeumannBC;

template <>
InputParameters validParams<CoupledNeumannBC>();

/**
 * Implements a simple constant Neumann BC where grad(u)=alpha * v on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class CoupledNeumannBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledNeumannBC(const InputParameters & parameters);

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

#endif // COUPLEDNEUMANNBC_H
