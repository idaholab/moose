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
class InterfaceIntegralVariableValuePostprocessor;

template <>
InputParameters validParams<InterfaceIntegralVariableValuePostprocessor>();

/**
 * This postprocessor computes a weighted (by area) integral of the specified variable.
 * The type of integral is determined by the _interface_value_type input parameter
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class InterfaceIntegralVariableValuePostprocessor : public InterfaceIntegralPostprocessor,
                                                    public MooseVariableInterface<Real>
{
public:
  InterfaceIntegralVariableValuePostprocessor(const InputParameters & parameters);
  // virtual void initialize() override;
  // virtual void execute() override;
  // virtual Real getValue() override;
  // virtual void threadJoin(const UserObject & y) override;

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

  /// the type of interface value we want to compute
  const MooseEnum _interface_value_type;
  /// the contribution of a qp to the integral
  virtual Real computeQpIntegral() override;
};

#endif
