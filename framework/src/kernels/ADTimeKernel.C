//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTimeKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/quadrature.h"

defineADValidParams(ADTimeKernel, ADKernelValue, params.set<MultiMooseEnum>("vector_tags") = "time";
                    params.set<MultiMooseEnum>("matrix_tags") = "system time";);
defineADValidParams(ADVectorTimeKernel,
                    ADVectorKernelValue,
                    params.set<MultiMooseEnum>("vector_tags") = "time";
                    params.set<MultiMooseEnum>("matrix_tags") = "system time";);

template <typename T, ComputeStage compute_stage>
ADTimeKernelTempl<T, compute_stage>::ADTimeKernelTempl(const InputParameters & parameters)
  : ADKernelValueTempl<T, compute_stage>(parameters), _u_dot(_var.template adUDot<compute_stage>())
{
}

template class ADTimeKernelTempl<Real, RESIDUAL>;
template class ADTimeKernelTempl<Real, JACOBIAN>;
template class ADTimeKernelTempl<RealVectorValue, RESIDUAL>;
template class ADTimeKernelTempl<RealVectorValue, JACOBIAN>;
