//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosIntegratedBC.h"

namespace Moose::Kokkos
{

InputParameters
IntegratedBC::validParams()
{
  InputParameters params = IntegratedBCBase::validParams();
  return params;
}

IntegratedBC::IntegratedBC(const InputParameters & parameters)
  : IntegratedBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _test(),
    _grad_test(),
    _phi(),
    _grad_phi(),
    _u(_kokkos_var),
    _grad_u(_kokkos_var)
{
  addMooseVariableDependency(_vars);
}

void
IntegratedBC::computeResidual()
{
  _thread.resize(_num_local_residual_threads, _vars.size(), numKokkosBoundarySides());

  Policy policy(0, _thread.size());

  if (!_residual_dispatcher)
    _residual_dispatcher = DispatcherRegistry::build<ResidualLoop>(this, type());

  _residual_dispatcher->parallelFor(policy);
}

void
IntegratedBC::computeJacobian()
{
  if (DispatcherRegistry::hasUserMethod<JacobianLoop>(type()))
  {
    _thread.resize(_num_local_jacobian_threads, _vars.size(), numKokkosBoundarySides());

    Policy policy(0, _thread.size());

    if (!_jacobian_dispatcher)
      _jacobian_dispatcher = DispatcherRegistry::build<JacobianLoop>(this, type());

    _jacobian_dispatcher->parallelFor(policy);
  }

  if (DispatcherRegistry::hasUserMethod<OffDiagJacobianLoop>(type()))
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    for (unsigned int i = 1; i < _vars.size(); ++i)
      if (sys.getFieldCoupling(_kokkos_var.var(0)).size() !=
          sys.getFieldCoupling(_kokkos_var.var(i)).size())
        mooseError("For multi-variable kernels/BCs, all variables should have the same number of "
                   "off-diagonal coupling variables.");

    _thread.resize(_num_local_jacobian_threads,
                   _vars.size(),
                   sys.getFieldCoupling(_kokkos_var.var()).size(),
                   numKokkosBoundarySides());

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_dispatcher)
      _offdiag_jacobian_dispatcher = DispatcherRegistry::build<OffDiagJacobianLoop>(this, type());

    _offdiag_jacobian_dispatcher->parallelFor(policy);
  }

  if (DispatcherRegistry::hasUserMethod<OffDiagJacobianScalarLoop>(type()))
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    for (unsigned int i = 1; i < _vars.size(); ++i)
      if (sys.getScalarCoupling(_kokkos_var.var(0)).size() !=
          sys.getScalarCoupling(_kokkos_var.var(i)).size())
        mooseError("For multi-variable kernels/BCs, all variables should have the same number of "
                   "off-diagonal scalar coupling variables.");

    _thread.resize(
        _vars.size(), sys.getScalarCoupling(_kokkos_var.var()).size(), numKokkosBoundarySides());

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_scalar_dispatcher)
      _offdiag_jacobian_scalar_dispatcher =
          DispatcherRegistry::build<OffDiagJacobianScalarLoop>(this, type());

    _offdiag_jacobian_scalar_dispatcher->parallelFor(policy);
  }
}

} // namespace Moose::Kokkos
