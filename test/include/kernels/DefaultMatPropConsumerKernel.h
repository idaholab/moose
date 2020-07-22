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
#include "DerivativeMaterialInterface.h"

template <bool is_ad>
class DefaultMatPropConsumerKernelTempl : public DerivativeMaterialInterface<GenericKernel<is_ad>>
{
public:
  static InputParameters validParams();

  DefaultMatPropConsumerKernelTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() { return 0.0; };

  const GenericMaterialProperty<Real, is_ad> & _prop;
};

typedef DefaultMatPropConsumerKernelTempl<false> DefaultMatPropConsumerKernel;
typedef DefaultMatPropConsumerKernelTempl<true> ADDefaultMatPropConsumerKernel;
