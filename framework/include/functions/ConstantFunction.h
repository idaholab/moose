//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"

/**
 * Class that represents constant function
 */
class ConstantFunction : public Function
{
public:
  static InputParameters validParams();

  ConstantFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual ADReal value(const ADReal & t, const ADPoint & p) const override;

  virtual Real timeDerivative(Real t, const Point & p) const override;
  virtual RealVectorValue gradient(Real t, const Point & p) const override;

  virtual Real timeIntegral(Real t1, Real t2, const Point & p) const override;

protected:
  const Real & _value;
};
