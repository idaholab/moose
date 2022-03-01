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

class Function;

/**
 * Function auxiliary value
 */
class VectorFunctionAux : public VectorAuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  VectorFunctionAux(const InputParameters & parameters);

protected:
  virtual RealVectorValue computeValue() override;

  /// Function being used to compute the value of this kernel
  const Function & _function;
};
