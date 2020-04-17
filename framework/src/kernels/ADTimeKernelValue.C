//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTimeKernelValue.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <typename T>
InputParameters
ADTimeKernelValueTempl<T>::validParams()
{
  InputParameters params = ADKernelValueTempl<T>::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  return params;
}

template <typename T>
ADTimeKernelValueTempl<T>::ADTimeKernelValueTempl(const InputParameters & parameters)
  : ADKernelValueTempl<T>(parameters), _u_dot(_var.adUDot())
{
}

template class ADTimeKernelValueTempl<Real>;
template class ADTimeKernelValueTempl<RealVectorValue>;
