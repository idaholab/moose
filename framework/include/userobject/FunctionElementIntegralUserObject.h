//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralUserObject.h"

/**
 * This user object computes a volume integral of a specified function.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class FunctionElementIntegralUserObject : public ElementIntegralUserObject
{
public:
  static InputParameters validParams();

  FunctionElementIntegralUserObject(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Function to integrate
  const Function & _function;
};
