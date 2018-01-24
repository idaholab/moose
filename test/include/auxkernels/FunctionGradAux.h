//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONGRADAUX_H
#define FUNCTIONGRADAUX_H

#include "AuxKernel.h"

// Forward Declarations
class FunctionGradAux;
class Function;

template <>
InputParameters validParams<FunctionGradAux>();

/**
 * AuxKernel for computing the gradient of a function and selecting one component
 * to be saved in the AuxVariable
 */
class FunctionGradAux : public AuxKernel
{
public:
  FunctionGradAux(const InputParameters & parameters);

  virtual ~FunctionGradAux();

protected:
  virtual Real computeValue();

  /// Function object from which gradient is retrieved
  Function & _func;

  /// The dimension index: 0|1|2 for x|y|z
  unsigned int _dim_index;
};

#endif // FunctionGradAux_H
