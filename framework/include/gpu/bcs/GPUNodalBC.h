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

#include "MooseVariableInterface.h"

template <typename NodalBC>
class GPUNodalBC : public GPUNodalBCBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = GPUNodalBCBase::validParams();

    return params;
  }

  GPUNodalBC(const InputParameters & parameters)
    : GPUNodalBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
      _u(systems(), _var),
      _default_offdiag(&NodalBC::computeQpOffDiagJacobian == &GPUNodalBC::computeQpOffDiagJacobian)
  {
    addMooseVariableDependency(&_var);
  }

  GPUNodalBC(const GPUNodalBC<NodalBC> & object)
    : GPUNodalBCBase(object), _u(object._u), _default_offdiag(object._default_offdiag)
  {
  }

  // Dispatch residual calculation to GPU
  virtual void computeResidual() override
  {
    Kokkos::RangePolicy<ResidualLoop, Kokkos::IndexType<size_t>> policy(0, numBoundaryNodes());
    Kokkos::parallel_for(policy, *static_cast<NodalBC *>(this));
    Kokkos::fence();
  }
  // Dispatch diagonal Jacobian calculation to GPU
  virtual void computeJacobian() override
  {
    Kokkos::RangePolicy<JacobianLoop, Kokkos::IndexType<size_t>> policy(0, numBoundaryNodes());
    Kokkos::parallel_for(policy, *static_cast<NodalBC *>(this));
    Kokkos::fence();

    if (!_default_offdiag)
    {
      auto & sys = system(_gpu_var.sys());

      _thread.resize({sys.getCoupling(_gpu_var.var()).size(), numBoundaryNodes()});

      Kokkos::RangePolicy<OffDiagJacobianLoop, Kokkos::IndexType<size_t>> policy(0, _thread.size());
      Kokkos::parallel_for(policy, *static_cast<NodalBC *>(this));
      Kokkos::fence();
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
    auto bc = static_cast<const NodalBC *>(this);
    auto node = boundaryNodeID(tid);

    Real local_re = bc->computeQpResidual(node);

    setTaggedLocalResidual(false, local_re, node);
  }
  KOKKOS_FUNCTION void operator()(JacobianLoop, const size_t tid) const
  {
    auto bc = static_cast<const NodalBC *>(this);
    auto node = boundaryNodeID(tid);

    Real local_ke = bc->computeQpJacobian(node);

    // This initializes the row to zero except the diagonal
    setTaggedLocalMatrix(false, local_ke, node, _gpu_var.var());
  }
  KOKKOS_FUNCTION void operator()(OffDiagJacobianLoop, const size_t tid) const
  {
    auto bc = static_cast<const NodalBC *>(this);
    auto node = boundaryNodeID(_thread(tid, 1));

    auto & sys = system(_gpu_var.sys());
    auto jvar = sys.getCoupling(_gpu_var.var())[_thread(tid, 0)];

    Real local_ke = bc->computeQpOffDiagJacobian(jvar, node);

    setTaggedLocalMatrix(true, local_ke, node, jvar);
  }

protected:
  // Holds the solution at current node
  GPUVariableNodalValue _u;

private:
  // Whether default computeQpOffDiagJacobian is used
  const bool _default_offdiag;
};

#define usingGPUNodalBCMembers(T)                                                                  \
  usingGPUNodalBCBaseMembers;                                                                      \
                                                                                                   \
protected:                                                                                         \
  using GPUNodalBC<T>::_u;                                                                         \
                                                                                                   \
public:                                                                                            \
  using GPUNodalBC<T>::operator();
