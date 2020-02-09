//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelGrad.h"

/**
 * AD time kernels should inherit from this class when the time portion of the weak residual is
 * multiplied by the gradient of the test function
 */
template <typename T>
class ADTimeKernelGradTempl : public ADKernelGradTempl<T>
{
public:
  static InputParameters validParams();

  ADTimeKernelGradTempl(const InputParameters & parameters);

protected:
  /// Holds the time derivatives at the quadrature points
  const ADTemplateVariableValue<T> & _u_dot;

  using ADKernelTempl<T>::_var;
};

using ADTimeKernelGrad = ADTimeKernelGradTempl<Real>;
using ADVectorTimeKernelGrad = ADTimeKernelGradTempl<RealVectorValue>;
