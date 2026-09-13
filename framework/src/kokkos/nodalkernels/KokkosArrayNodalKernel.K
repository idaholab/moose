//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "KokkosArrayNodalKernel.h"

namespace Moose::Kokkos
{

InputParameters
ArrayNodalKernel::validParams()
{
  InputParameters params = NodalKernelBase::validParams();
  return params;
}

ArrayNodalKernel::ArrayNodalKernel(const InputParameters & parameters)
  : NodalKernelBase(parameters, Moose::VarFieldType::VAR_FIELD_ARRAY),
    _u(_var, Moose::SOLUTION_TAG, true),
    _count(_var.count()),
    _boundary_restricted(boundaryRestricted())
{
  addMooseVariableDependency(&_var);
}

void
ArrayNodalKernel::computeResidual()
{
  _thread.resize(_count, _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes());

  Policy policy(0, _thread.size());

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  _residual_dispatcher->parallelFor(policy);
}

void
ArrayNodalKernel::computeJacobian()
{
  const auto num_nodes = _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes();

  if (DispatcherRegistry::hasUserMethod<JacobianLoop>(type()))
  {
    _thread.resize(_count, num_nodes);

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

    _thread.resize(_count, max_couplings, num_nodes);

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_dispatcher)
      _offdiag_jacobian_dispatcher = DispatcherRegistry::build<OffDiagJacobianLoop>(this, type());

    _offdiag_jacobian_dispatcher->parallelFor(policy);
  }
}

} // namespace Moose::Kokkos
