//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  addMooseVariableDependency(variables());
}

void
IntegratedBC::computeResidual()
{
  _thread.resize({variables().size(), numKokkosBoundarySides()});

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
    _thread.resize({variables().size(), numKokkosBoundarySides()});

    Policy policy(0, _thread.size());

    if (!_jacobian_dispatcher)
      _jacobian_dispatcher = DispatcherRegistry::build<JacobianLoop>(this, type());

    _jacobian_dispatcher->parallelFor(policy);
  }

  if (DispatcherRegistry::hasUserMethod<OffDiagJacobianLoop>(type()))
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    if (variables().size() > 1)
      for (unsigned int i = 1; i < variables().size(); ++i)
        if (sys.getCoupling(_kokkos_var.var(0)).size() !=
            sys.getCoupling(_kokkos_var.var(i)).size())
          mooseError("For multi-variable kernels/BCs, all variables should have the same number of "
                     "off-diagonal coupling variables.");

    _thread.resize(
        {variables().size(), sys.getCoupling(_kokkos_var.var()).size(), numKokkosBoundarySides()});

    Policy policy(0, _thread.size());

    if (!_offdiag_jacobian_dispatcher)
      _offdiag_jacobian_dispatcher = DispatcherRegistry::build<OffDiagJacobianLoop>(this, type());

    _offdiag_jacobian_dispatcher->parallelFor(policy);
  }
}

} // namespace Moose::Kokkos
