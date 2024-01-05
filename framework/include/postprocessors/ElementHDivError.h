//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementHDivSemiError.h"

/**
 * This postprocessor will print out the H(div)-norm of the difference
 * between the computed solution and the passed function by summing the
 * squares of the L2-norm and the H(div)-seminorm of the same difference.
 */
class ElementHDivError : public ElementHDivSemiError
{
public:
  static InputParameters validParams();

  ElementHDivError(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
