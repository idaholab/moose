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
 * Computes the divergence of a vector of variables
 */
template <bool is_ad>
class DivergenceAuxTempl : public AuxKernel
{
public:
  static InputParameters validParams();

  DivergenceAuxTempl(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  // Component variable gradients
  const Moose::Functor<GenericReal<is_ad>> & _u;
  const Moose::Functor<GenericReal<is_ad>> * _v;
  const Moose::Functor<GenericReal<is_ad>> * _w;

  /// Whether to use a quadrature-based functor argument, appropriate for finite element
  /// evaluations. If false, use a cell-center functor argument appropriate for finite volume
  /// calculations
  const bool _use_qp_arg;
};

typedef DivergenceAuxTempl<false> DivergenceAux;
typedef DivergenceAuxTempl<true> ADDivergenceAux;
