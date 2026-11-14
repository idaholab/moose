//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosKernel.h"

namespace Moose::Kokkos
{

InputParameters
Kernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  return params;
}

Kernel::Kernel(const InputParameters & parameters)
  : KernelBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _test(),
    _grad_test(),
    _phi(),
    _grad_phi(),
    _u(_var),
    _grad_u(_var)
{
  addMooseVariableDependency(&_var);
}

void
Kernel::computeResidual()
{
  computePrecomputedResidual(numKokkosBlockElements(), kokkosAssembly().getMaxQpsPerElem());
}

void
Kernel::computeJacobian()
{
  computePrecomputedJacobian(
      numKokkosBlockElements(), kokkosAssembly().getMaxQpsPerElem(), supports_scalar_jacobian);
}

} // namespace Moose::Kokkos
