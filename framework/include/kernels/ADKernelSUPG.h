//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelStabilized.h"

template <typename>
class ADKernelSUPGTempl;

using ADKernelSUPG = ADKernelSUPGTempl<Real>;
using ADVectorKernelSUPG = ADKernelSUPGTempl<RealVectorValue>;

template <typename T>
class ADKernelSUPGTempl : public ADKernelStabilizedTempl<T>
{
public:
  static InputParameters validParams();

  ADKernelSUPGTempl(const InputParameters & parameters);

protected:
  ADRealVectorValue computeQpStabilization() override;

  const ADMaterialProperty<Real> & _tau;
  const ADVectorVariableValue & _velocity;

  using ADKernelStabilizedTempl<T>::_qp;
};
