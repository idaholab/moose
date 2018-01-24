//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTINTEGRALPOSTPROCESSOR_H
#define ELEMENTINTEGRALPOSTPROCESSOR_H

#include "ElementPostprocessor.h"

// Forward Declarations
class ElementIntegralPostprocessor;

template <>
InputParameters validParams<ElementIntegralPostprocessor>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementIntegralPostprocessor : public ElementPostprocessor
{
public:
  ElementIntegralPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() = 0;
  virtual Real computeIntegral();

  unsigned int _qp;

  Real _integral_value;
};

#endif
