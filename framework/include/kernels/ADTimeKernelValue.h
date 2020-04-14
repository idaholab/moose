//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * AD time kernels should inherit from this class when the time portion of the weak residual is
 * multiplied by the test function
 */
template <typename T>
class ADTimeKernelValueTempl : public ADKernelValueTempl<T>
{
public:
  static InputParameters validParams();

  ADTimeKernelValueTempl(const InputParameters & parameters);

protected:
  /// Holds the time derivatives at the quadrature points
  const ADTemplateVariableValue<T> & _u_dot;

  using ADKernelTempl<T>::_var;
};

using ADTimeKernelValue = ADTimeKernelValueTempl<Real>;
using ADVectorTimeKernelValue = ADTimeKernelValueTempl<RealVectorValue>;
