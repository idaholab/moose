//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

class Function;

/**
 * Gravity computes the body force (force/volume) given the acceleration of gravity (value) and the
 * density
 */
template <bool is_ad>
class GravityTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  GravityTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  const GenericMaterialProperty<Real, is_ad> & _density;
  const Real _value;
  const Function & _function;

  // _alpha parameter for HHT time integration scheme
  const Real _alpha;

  using GenericKernel<is_ad>::_dt;
  using GenericKernel<is_ad>::_i;
  using GenericKernel<is_ad>::_q_point;
  using GenericKernel<is_ad>::_qp;
  using GenericKernel<is_ad>::_t;
  using GenericKernel<is_ad>::_test;
};

using Gravity = GravityTempl<false>;
using ADGravity = GravityTempl<true>;
