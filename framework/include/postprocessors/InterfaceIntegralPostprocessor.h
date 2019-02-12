//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEINTEGRALPOSTPROCESSOR_H
#define INTERFACEINTEGRALPOSTPROCESSOR_H

#include "InterfacePostprocessor.h"

// Forward Declarations
class InterfaceIntegralPostprocessor;

template <>
InputParameters validParams<InterfaceIntegralPostprocessor>();

/**
 * This postprocessor computes a side integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class InterfaceIntegralPostprocessor : public InterfacePostprocessor
{
public:
  InterfaceIntegralPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

  static MooseEnum integralTypeOptions();

protected:
  virtual Real computeQpIntegral() = 0; // MUST BE OVERRIDDEN
  virtual Real computeIntegral();

  unsigned int _qp;

  Real _integral_value;
};

#endif
