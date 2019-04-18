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

defineADValidParams(ADTimeKernelGrad,
                    ADKernelGrad,
                    params.set<MultiMooseEnum>("vector_tags") = "time";
                    params.set<MultiMooseEnum>("matrix_tags") = "system time";);
defineADValidParams(ADVectorTimeKernelGrad,
                    ADVectorKernelGrad,
                    params.set<MultiMooseEnum>("vector_tags") = "time";
                    params.set<MultiMooseEnum>("matrix_tags") = "system time";);

template <typename T, ComputeStage compute_stage>
ADTimeKernelGradTempl<T, compute_stage>::ADTimeKernelGradTempl(const InputParameters & parameters)
  : ADKernelGradTempl<T, compute_stage>(parameters), _u_dot(_var.template adUDot<compute_stage>())
{
}

template class ADTimeKernelGradTempl<Real, RESIDUAL>;
template class ADTimeKernelGradTempl<Real, JACOBIAN>;
template class ADTimeKernelGradTempl<RealVectorValue, RESIDUAL>;
template class ADTimeKernelGradTempl<RealVectorValue, JACOBIAN>;
