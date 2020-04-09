//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctionInterface.h"

#include "MemoizedFunctionInterface.h"
#include "MutableCoefficientsInterface.h"

/**
 * Interface for a type of functions using coefficients that may be changed before or after a solve
 */
class MutableCoefficientsFunctionInterface : public MemoizedFunctionInterface,
                                             protected FunctionInterface,
                                             public MutableCoefficientsInterface
{
public:
  static InputParameters validParams();

  MutableCoefficientsFunctionInterface(const MooseObject * moose_object,
                                       const InputParameters & parameters);

protected:
  // Override from MemoizedFunctionInterface
  virtual void coefficientsChanged() override;
};
