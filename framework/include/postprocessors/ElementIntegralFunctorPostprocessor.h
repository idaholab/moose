//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"
#include "MooseEnum.h"

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
  CreateMooseEnumClass(FunctorEvaluationType, CELL_AVERAGE, QUADRATURE_POINT);

  virtual Real computeIntegral() override;
  virtual Real computeQpIntegral() override;
  virtual Real cellAverage();

  /// Functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /// Factor multiplying the functor being integrated
  const Moose::Functor<GenericReal<is_ad>> & _prefactor;

  /// How to evaluate the functor
  const FunctorEvaluationType _evaluation_type;
};

typedef ElementIntegralFunctorPostprocessorTempl<false> ElementIntegralFunctorPostprocessor;
typedef ElementIntegralFunctorPostprocessorTempl<true> ADElementIntegralFunctorPostprocessor;
