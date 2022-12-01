//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InternalSideIntegralVariablePostprocessor.h"

/**
 * This postprocessor computes the L2 norm of a variable over element
 * sides
 */
class ElementSidesL2Norm : public InternalSideIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  ElementSidesL2Norm(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;
};
