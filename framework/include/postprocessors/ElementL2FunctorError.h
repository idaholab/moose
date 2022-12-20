//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

/**
 * Computes L2 error between an 'approximate' functor and an 'exact' functor
 */
class ElementL2FunctorError : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ElementL2FunctorError(const InputParameters & parameters);

  Real getValue() override;

protected:
  Real computeQpIntegral() override;

  /// The approximate functor
  const Moose::Functor<ADReal> & _approx;

  /// The analytical functor
  const Moose::Functor<ADReal> & _exact;
};
