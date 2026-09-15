//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosArrayKernel.h"

namespace Moose::Kokkos
{

InputParameters
ArrayKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("ArrayKernel");
  return params;
}

ArrayKernel::ArrayKernel(const InputParameters & parameters)
  : KernelBase(parameters, Moose::VarFieldType::VAR_FIELD_ARRAY),
    _test(),
    _grad_test(),
    _phi(),
    _grad_phi(),
    _u(_var),
    _grad_u(_var),
    _count(_var.count())
{
  addMooseVariableDependency(&_var);
}

void
ArrayKernel::computeResidual()
{
  _thread.resize(_count, _num_local_residual_threads, numKokkosBlockElements());

  Policy policy(0, _thread.size());

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  _residual_dispatcher->parallelFor(policy);
}

void
ArrayKernel::computeJacobian()
{
  if (DispatcherRegistry::hasUserMethod<JacobianLoop>(type()))
  {
    _thread.resize(_count, _num_local_jacobian_threads, numKokkosBlockElements());

    Policy policy(0, _thread.size());

    if (!_jacobian_dispatcher)
      _jacobian_dispatcher = DispatcherRegistry::build<JacobianLoop>(this, type());

    _jacobian_dispatcher->parallelFor(policy);
  }

  if (DispatcherRegistry::hasUserMethod<OffDiagJacobianLoop>(type()))
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());
    unsigned int max_couplings = 0;

    for (const auto comp : make_range(_count))
      if (sys.getCoupling(_kokkos_var.var(comp)).size() > max_couplings)
        max_couplings = sys.getCoupling(_kokkos_var.var(comp)).size();

    _thread.resize(_count, _num_local_jacobian_threads, max_couplings, numKokkosBlockElements());

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_dispatcher)
      _offdiag_jacobian_dispatcher = DispatcherRegistry::build<OffDiagJacobianLoop>(this, type());

    _offdiag_jacobian_dispatcher->parallelFor(policy);
  }
}

} // namespace Moose::Kokkos
