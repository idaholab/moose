//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementLoopUserObject.h"

/**
 * Computes the integral of a function using an element loop.
 */
class FunctionElementLoopIntegralUserObject : public ElementLoopUserObject
{
public:
  FunctionElementLoopIntegralUserObject(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void computeElement() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

  /**
   * Returns the integral value
   */
  virtual Real getValue() const;

protected:
  /**
   * Computes integral on an element
   */
  virtual Real computeIntegral();
  /**
   * Computes value of integrand at quadrature point
   */
  virtual Real computeQpIntegral();

  /// Function to integrate
  const Function & _function;

  /// Quadrature point index
  unsigned int _qp;

  /// Integral
  Real _integral_value;

public:
  static InputParameters validParams();
};
