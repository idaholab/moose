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

defineADValidParams(ADTimeKernelValue,
                    ADKernelValue,
                    params.set<MultiMooseEnum>("vector_tags") = "time";
                    params.set<MultiMooseEnum>("matrix_tags") = "system time";);
defineADValidParams(ADVectorTimeKernelValue,
                    ADVectorKernelValue,
                    params.set<MultiMooseEnum>("vector_tags") = "time";
                    params.set<MultiMooseEnum>("matrix_tags") = "system time";);

template <typename T, ComputeStage compute_stage>
ADTimeKernelValueTempl<T, compute_stage>::ADTimeKernelValueTempl(const InputParameters & parameters)
  : ADKernelValueTempl<T, compute_stage>(parameters), _u_dot(_var.template adUDot<compute_stage>())
{
}

template class ADTimeKernelValueTempl<Real, RESIDUAL>;
template class ADTimeKernelValueTempl<Real, JACOBIAN>;
template class ADTimeKernelValueTempl<RealVectorValue, RESIDUAL>;
template class ADTimeKernelValueTempl<RealVectorValue, JACOBIAN>;
