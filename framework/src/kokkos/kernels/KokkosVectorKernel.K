//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosVectorKernel.h"

namespace Moose::Kokkos
{

InputParameters
VectorKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("VectorKernel");
  return params;
}

VectorKernel::VectorKernel(const InputParameters & parameters)
  : KernelBase(parameters, Moose::VarFieldType::VAR_FIELD_VECTOR),
    _test(),
    _grad_test(),
    _phi(),
    _grad_phi(),
    _u(_var),
    _grad_u(_var),
    _curl_test(),
    _curl_phi(),
    _curl_u(_var)
{
  addMooseVariableDependency(&_var);
}

void
VectorKernel::computeResidual()
{
  computePrecomputedResidual(numKokkosBlockElements(), kokkosAssembly().getMaxQpsPerElem());
}

void
VectorKernel::computeJacobian()
{
  computePrecomputedJacobian(
      numKokkosBlockElements(), kokkosAssembly().getMaxQpsPerElem(), supports_scalar_jacobian);
}

} // namespace Moose::Kokkos
