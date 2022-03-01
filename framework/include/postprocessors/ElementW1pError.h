//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

class Function;

/**
 * This postprocessor computes the Sobolev norm W^{1,p} of the
 * difference between the computed solution and the passed in function.
 * If p==2, this is equivalent to the H1-norm, but p can be any real
 * number >= 1.  There are two possible definitions of this norm:
 *
 * 1.) ||u-f||_{W^{1,p}} \equiv (\int |u-f|^p dx + sum_{i=1}^3 \int |du/dx_i - df/dx_i|^p dx)^{1/p}
 * 2.) ||u-f||_{W^{1,p}} \equiv (\int |u-f|^p dx)^{1/p} + sum_{i=1}^3 (\int |du/dx_i - df/dx_i|^p
 * dx)^{1/p}
 *
 * which are equivalent in the "equivalence of norms" sense.  (The
 * difference is that the "1/p" exponent is on the outside of the sum
 * in case 1, while it is on every term in case 2.  We use definition
 * 1 here for consistency with the original ElementH1Error class.
 */
class ElementW1pError : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  ElementW1pError(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  // The exponent used in the norm
  Real _p;
  const Function & _func;
};
