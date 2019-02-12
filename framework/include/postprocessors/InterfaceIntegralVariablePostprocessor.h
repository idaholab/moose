//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEINTEGRALVARIABLEPOSTPROCESSOR_H
#define INTERFACEINTEGRALVARIABLEPOSTPROCESSOR_H

#include "InterfaceIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class InterfaceIntegralVariablePostprocessor;

template <>
InputParameters validParams<InterfaceIntegralVariablePostprocessor>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class InterfaceIntegralVariablePostprocessor : public InterfaceIntegralPostprocessor,
                                               public MooseVariableInterface<Real>
{
public:
  InterfaceIntegralVariablePostprocessor(const InputParameters & parameters);

protected:
  /// Holds the solution at current quadrature points
  const VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;
  /// Holds the solution at current quadrature points on the neighbor side
  const VariableValue & _u_neighbor;
  /// Holds the solution gradient at the current quadrature points on the
  /// neighbor side
  const VariableGradient & _grad_u_neighbor;
  // virtual Real computeQpIntegral() override = 0;
};

#endif
