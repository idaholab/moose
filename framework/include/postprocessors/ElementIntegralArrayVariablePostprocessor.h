//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementIntegralArrayVariablePostprocessor : public ElementIntegralPostprocessor,
                                                  public MooseVariableInterface<RealEigenVector>
{
public:
  static InputParameters validParams();

  ElementIntegralArrayVariablePostprocessor(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the solution at current quadrature points
  const ArrayVariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const ArrayVariableGradient & _grad_u;
  /// The component
  const unsigned int _component;
};
