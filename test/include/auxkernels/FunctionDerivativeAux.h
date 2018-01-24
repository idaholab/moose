//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONDERIVATIVEAUX_H
#define FUNCTIONDERIVATIVEAUX_H

#include "AuxKernel.h"

// Forward Declarations
class FunctionDerivativeAux;
class Function;

template <>
InputParameters validParams<FunctionDerivativeAux>();

/**
 * Function auxiliary value
 */
class FunctionDerivativeAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  FunctionDerivativeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Function being used to compute the value of this kernel
  Function & _func;
  unsigned int _component;
};

#endif // FUNCTIONDERIVATIVEAUX_H
