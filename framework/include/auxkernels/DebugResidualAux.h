//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DEBUGRESIDUALAUX_H
#define DEBUGRESIDUALAUX_H

#include "AuxKernel.h"

class DebugResidualAux;

template <>
InputParameters validParams<DebugResidualAux>();

/**
 * Auxiliary kernel for debugging convergence.
 */
class DebugResidualAux : public AuxKernel
{
public:
  DebugResidualAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  MooseVariable & _debug_var;
  NumericVector<Number> & _residual_copy;
};

#endif /* DEBUGRESIDUALAUX_H */
