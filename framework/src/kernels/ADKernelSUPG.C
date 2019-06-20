//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADKernelSUPG.h"
#include "MathUtils.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/threads.h"

#define PGParams                                                                                   \
  params.addParam<MaterialPropertyName>(                                                           \
      "tau_name", "tau", "The name of the stabilization parameter tau.");                          \
  params.addRequiredCoupledVar("velocity", "The velocity variable.")

defineADValidParams(ADKernelSUPG, ADKernel, PGParams;);
defineADValidParams(ADVectorKernelSUPG, ADVectorKernel, PGParams;);

template <typename T, ComputeStage compute_stage>
ADKernelSUPGTempl<T, compute_stage>::ADKernelSUPGTempl(const InputParameters & parameters)
  : ADKernelStabilizedTempl<T, compute_stage>(parameters),
    _tau(getADMaterialProperty<Real>("tau_name")),
    _velocity(adCoupledVectorValue("velocity"))
{
}

template <typename T, ComputeStage compute_stage>
ADRealVectorValue
ADKernelSUPGTempl<T, compute_stage>::computeQpStabilization()
{
  return _velocity[_qp] * _tau[_qp];
}

template class ADKernelSUPGTempl<Real, RESIDUAL>;
template class ADKernelSUPGTempl<Real, JACOBIAN>;
template class ADKernelSUPGTempl<RealVectorValue, RESIDUAL>;
template class ADKernelSUPGTempl<RealVectorValue, JACOBIAN>;
