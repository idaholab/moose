//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIDEINTEGRALVARIABLEPOSTPROCESSOR_H
#define SIDEINTEGRALVARIABLEPOSTPROCESSOR_H

#include "SideIntegralPostprocessor.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class SideIntegralVariablePostprocessor;

template <>
InputParameters validParams<SideIntegralVariablePostprocessor>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class SideIntegralVariablePostprocessor : public SideIntegralPostprocessor,
                                          public MooseVariableInterface
{
public:
  SideIntegralVariablePostprocessor(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;
};

#endif
