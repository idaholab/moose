//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementAverageValue.h"

/**
 * This postprocessor computes a volume integral of the time derivative of a given variable.
 */
class ElementAverageTimeDerivative : public ElementAverageValue
{
public:
  static InputParameters validParams();

  ElementAverageTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the solution derivative at the current quadrature points
  const VariableValue & _u_dot;
};
