//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KernelValue.h"
#include "ADKernelValue.h"

// switch parent class depending on is_ad value
template <bool is_ad>
using KernelValueParent = typename std::conditional<is_ad, ADKernelValue, KernelValue>::type;

/**
 * Define the Kernel that computes the residual to match the value of a functor
 */
template <bool is_ad>
class FunctorValueTempl : public KernelValueParent<is_ad>
{
public:
  static InputParameters validParams();

  FunctorValueTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> precomputeQpResidual();
  virtual Real precomputeQpJacobian();

  const Real _kernel_sign;
  const Moose::Functor<GenericReal<is_ad>> & _functor;

  using KernelValueParent<is_ad>::_qp;
  using KernelValueParent<is_ad>::_u;
  using KernelValueParent<is_ad>::_phi;
  using KernelValueParent<is_ad>::_j;
};

using FunctorValue = FunctorValueTempl<false>;
using ADFunctorValue = FunctorValueTempl<true>;
