//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INTERFACEINTEGRALVARIABLEAVERAGEPOSTPROCESSOR_H
#define INTERFACEINTEGRALVARIABLEAVERAGEPOSTPROCESSOR_H

#include "InterfaceIntegralVariablePostprocessor.h"

// Forward Declarations
class InterfaceIntegralVariableAveragePostprocessor;

template <>
InputParameters validParams<InterfaceIntegralVariableAveragePostprocessor>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class InterfaceIntegralVariableAveragePostprocessor : public InterfaceIntegralVariablePostprocessor
{
public:
  InterfaceIntegralVariableAveragePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  virtual Real volume();
  const MooseEnum _average_type;
  Real _volume;
  virtual Real computeQpIntegral() override;
};

#endif
