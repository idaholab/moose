//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
template <bool is_ad>
class ElementL2FunctorErrorTempl : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ElementL2FunctorErrorTempl(const InputParameters & parameters);

  virtual Real getValue() const override;

protected:
  virtual Real computeQpIntegral() override;

  /// The approximate functor
  const Moose::Functor<ADReal> & _approx;

  /// The analytical functor
  const Moose::Functor<GenericReal<is_ad>> & _exact;
};

typedef ElementL2FunctorErrorTempl<false> ElementL2FunctorError;
typedef ElementL2FunctorErrorTempl<true> ADElementL2FunctorError;
