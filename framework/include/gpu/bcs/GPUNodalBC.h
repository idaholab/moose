//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUNodalBCBase.h"

namespace Moose
{
namespace Kokkos
{

template <typename Derived>
class NodalBC : public NodalBCBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = NodalBCBase::validParams();

    return params;
  }

  NodalBC(const InputParameters & parameters)
    : NodalBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
      _u(kokkosSystems(), _var),
      _default_offdiag(&Derived::computeQpOffDiagJacobian == &NodalBC::computeQpOffDiagJacobian)
  {
    addMooseVariableDependency(&_var);
  }

  NodalBC(const NodalBC<Derived> & object)
    : NodalBCBase(object), _u(object._u), _default_offdiag(object._default_offdiag)
  {
  }

  // Dispatch residual calculation to GPU
  virtual void computeResidual() override
  {
    ::Kokkos::RangePolicy<ResidualLoop, ::Kokkos::IndexType<size_t>> policy(0, numBoundaryNodes());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();
  }
  // Dispatch diagonal Jacobian calculation to GPU
  virtual void computeJacobian() override
  {
    ::Kokkos::RangePolicy<JacobianLoop, ::Kokkos::IndexType<size_t>> policy(0, numBoundaryNodes());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();

    if (!_default_offdiag)
    {
      auto & sys = kokkosSystem(_kokkos_var.sys());

      _thread.resize({sys.getCoupling(_kokkos_var.var()).size(), numBoundaryNodes()});

      ::Kokkos::RangePolicy<OffDiagJacobianLoop, ::Kokkos::IndexType<size_t>> policy(
          0, _thread.size());
      ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
      ::Kokkos::fence();
    }
  }

  // Empty methods to prevent compile errors even when these methods were not hidden by the derived
  // class
  KOKKOS_FUNCTION Real computeQpJacobian(const dof_id_type /* node */) const { return 1; }
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* jvar */,
                                                const dof_id_type /* node */) const
  {
    return 0;
  }

  // Overloaded operators called by Kokkos::parallel_for
  KOKKOS_FUNCTION void operator()(ResidualLoop, const size_t tid) const
  {
    auto bc = static_cast<const Derived *>(this);
    auto node = boundaryNodeID(tid);

    Real local_re = bc->computeQpResidual(node);

    accumulateTaggedNodalResidual(false, local_re, node);
  }
  KOKKOS_FUNCTION void operator()(JacobianLoop, const size_t tid) const
  {
    auto bc = static_cast<const Derived *>(this);
    auto node = boundaryNodeID(tid);

    Real local_ke = bc->computeQpJacobian(node);

    // This initializes the row to zero except the diagonal
    accumulateTaggedNodalMatrix(false, local_ke, node, _kokkos_var.var());
  }
  KOKKOS_FUNCTION void operator()(OffDiagJacobianLoop, const size_t tid) const
  {
    auto bc = static_cast<const Derived *>(this);
    auto node = boundaryNodeID(_thread(tid, 1));

    auto & sys = kokkosSystem(_kokkos_var.sys());
    auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 0)];

    Real local_ke = bc->computeQpOffDiagJacobian(jvar, node);

    accumulateTaggedNodalMatrix(true, local_ke, node, jvar);
  }

protected:
  // Holds the solution at current node
  VariableNodalValue _u;

private:
  // Whether default computeQpOffDiagJacobian is used
  const bool _default_offdiag;
};

} // namespace Kokkos
} // namespace Moose

#define usingKokkosNodalBCMembers(T)                                                               \
  usingKokkosNodalBCBaseMembers;                                                                   \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::NodalBC<T>::_u;                                                             \
                                                                                                   \
public:                                                                                            \
  using Moose::Kokkos::NodalBC<T>::operator();
