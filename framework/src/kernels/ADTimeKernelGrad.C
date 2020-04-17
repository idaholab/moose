//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTimeKernelGrad.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

template <typename T>
InputParameters
ADTimeKernelGradTempl<T>::validParams()
{
  InputParameters params = ADKernelGradTempl<T>::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  return params;
}

template <typename T>
ADTimeKernelGradTempl<T>::ADTimeKernelGradTempl(const InputParameters & parameters)
  : ADKernelGradTempl<T>(parameters), _u_dot(_var.adUDot())
{
}

template class ADTimeKernelGradTempl<Real>;
template class ADTimeKernelGradTempl<RealVectorValue>;
