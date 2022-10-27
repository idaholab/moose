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

/**
 * This postprocessor computes an element integral of the specified functor
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
template <bool is_ad>
class ElementIntegralFunctorPostprocessorTempl : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ElementIntegralFunctorPostprocessorTempl(const InputParameters & parameters);

protected:
  Real computeQpIntegral() override;

  /// Functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /// Factor multiplying the functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _prefactor;
};

typedef ElementIntegralFunctorPostprocessorTempl<false> ElementIntegralFunctorPostprocessor;
typedef ElementIntegralFunctorPostprocessorTempl<true> ADElementIntegralFunctorPostprocessor;
