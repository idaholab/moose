//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Time derivative of a functor, which can be a variable, function, functor material property
 */
template <bool is_ad>
class TimeDerivativeAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  TimeDerivativeAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// Functor to take the time derivative of
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  /// Factor to multiply the time derivative with
  const Moose::Functor<GenericReal<is_ad>> & _factor;

  /// Whether to use a quadrature-based functor argument, appropriate for finite element
  /// evaluations. If false, use a cell-center functor argument appropriate for finite volume
  /// calculations
  const bool _use_qp_arg;
};

typedef TimeDerivativeAuxTempl<false> TimeDerivativeAux;
typedef TimeDerivativeAuxTempl<true> ADTimeDerivativeAux;
