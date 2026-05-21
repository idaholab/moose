//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosADKernel.h"

namespace Moose::Kokkos
{

InputParameters
ADKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  return params;
}

ADKernel::ADKernel(const InputParameters & parameters)
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
ADKernel::computeResidual()
{
  _computing_residual = true;
  _computing_jacobian = false;

  dispatch();
}

void
ADKernel::computeJacobian()
{
  _computing_residual = false;
  _computing_jacobian = true;

  dispatch();
}

void
ADKernel::computeResidualAndJacobian()
{
  _computing_residual = true;
  _computing_jacobian = true;

  dispatch();
}

void
ADKernel::dispatch()
{
  _thread.resize(_num_local_threads, numKokkosBlockElements());

  Policy policy(0, _thread.size());

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  _residual_dispatcher->parallelFor(policy);
}

} // namespace Moose::Kokkos
