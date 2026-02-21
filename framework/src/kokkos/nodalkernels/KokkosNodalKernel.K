//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNodalKernel.h"

namespace Moose::Kokkos
{

InputParameters
NodalKernel::validParams()
{
  InputParameters params = NodalKernelBase::validParams();
  return params;
}

NodalKernel::NodalKernel(const InputParameters & parameters)
  : NodalKernelBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(_var, Moose::SOLUTION_TAG, true),
    _boundary_restricted(boundaryRestricted())
{
  addMooseVariableDependency(&_var);
}

void
NodalKernel::computeResidual()
{
  Policy policy(0, _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes());

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  _residual_dispatcher->parallelFor(policy);
}

void
NodalKernel::computeJacobian()
{
  if (DispatcherRegistry::hasUserMethod<JacobianLoop>(type()))
  {
    Policy policy(0, _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes());

    if (!_jacobian_dispatcher)
      _jacobian_dispatcher = DispatcherRegistry::build<JacobianLoop>(this, type());

    _jacobian_dispatcher->parallelFor(policy);
  }

  if (DispatcherRegistry::hasUserMethod<OffDiagJacobianLoop>(type()))
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    _thread.resize({sys.getCoupling(_kokkos_var.var()).size(),
                    _boundary_restricted ? numKokkosBoundaryNodes() : numKokkosBlockNodes()});

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_dispatcher)
      _offdiag_jacobian_dispatcher = DispatcherRegistry::build<OffDiagJacobianLoop>(this, type());

    _offdiag_jacobian_dispatcher->parallelFor(policy);
  }
}

} // namespace Moose::Kokkos
