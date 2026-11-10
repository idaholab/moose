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
    _u(_var),
    _grad_u(_var)
{
  addMooseVariableDependency(&_var);
}

void
IntegratedBC::computeResidual()
{
  _thread.resize(_num_local_residual_threads, numKokkosBoundarySides());

  Policy policy(0, _thread.size());

  if (!_residual_dispatcher)
    _residual_dispatcher = buildConfigurationDispatcher<ResidualLoop>(this);

  _residual_dispatcher->parallelFor(policy);
}

void
IntegratedBC::computeJacobian()
{
  if (hasConfigurationUserMethod<JacobianLoop>())
  {
    _thread.resize(_num_local_jacobian_threads, numKokkosBoundarySides());

    Policy policy(0, _thread.size());

    if (!_jacobian_dispatcher)
      _jacobian_dispatcher = buildConfigurationDispatcher<JacobianLoop>(this);

    _jacobian_dispatcher->parallelFor(policy);
  }

  if (hasConfigurationUserMethod<OffDiagJacobianLoop>())
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    _thread.resize(_num_local_jacobian_threads,
                   sys.getFieldCoupling(_kokkos_var.var()).size(),
                   numKokkosBoundarySides());

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_dispatcher)
      _offdiag_jacobian_dispatcher = buildConfigurationDispatcher<OffDiagJacobianLoop>(this);

    _offdiag_jacobian_dispatcher->parallelFor(policy);
  }

  if (hasConfigurationUserMethod<OffDiagJacobianScalarLoop>())
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    _thread.resize(sys.getScalarCoupling(_kokkos_var.var()).size(), numKokkosBoundarySides());

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_scalar_dispatcher)
      _offdiag_jacobian_scalar_dispatcher =
          buildConfigurationDispatcher<OffDiagJacobianScalarLoop>(this);

    _offdiag_jacobian_scalar_dispatcher->parallelFor(policy);
  }

  // An object posed on the current configuration assembles its Jacobian with respect to the
  // displacements from its own hooks
  if (_configuration.displaced())
    dispatchDisplacementJacobian(numKokkosBoundarySides(), false);
}

} // namespace Moose::Kokkos
