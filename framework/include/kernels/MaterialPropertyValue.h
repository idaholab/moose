//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

template <bool is_ad>
class MaterialPropertyValueTempl : public KernelValueParent<is_ad>
{
public:
  static InputParameters validParams();

  MaterialPropertyValueTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> precomputeQpResidual();
  virtual Real precomputeQpJacobian();

  const Real _kernel_sign;
  const GenericMaterialProperty<Real, is_ad> & _prop;

  using KernelValueParent<is_ad>::_qp;
  using KernelValueParent<is_ad>::_u;
  using KernelValueParent<is_ad>::_phi;
  using KernelValueParent<is_ad>::_j;
};

using MaterialPropertyValue = MaterialPropertyValueTempl<false>;
using ADMaterialPropertyValue = MaterialPropertyValueTempl<true>;
