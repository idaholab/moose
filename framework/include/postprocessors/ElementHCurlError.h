//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementHCurlSemiError.h"

/**
 * This postprocessor will print out the H(curl)-norm of the difference
 * between the computed solution and the passed function by summing the
 * squares of the L2-norm and the H(curl)-seminorm of the same difference.
 */
class ElementHCurlError : public ElementHCurlSemiError
{
public:
  static InputParameters validParams();

  ElementHCurlError(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};
