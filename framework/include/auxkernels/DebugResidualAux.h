//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel for debugging convergence.
 */
class DebugResidualAux : public AuxKernel
{
public:
  static InputParameters validParams();

  DebugResidualAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  MooseVariableFEBase & _debug_var;
  NumericVector<Number> & _residual_copy;
};
