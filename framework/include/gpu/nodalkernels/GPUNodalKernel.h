//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUNodalKernelBase.h"

template <typename NodalKernel>
class GPUNodalKernel : public GPUNodalKernelBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = GPUNodalKernelBase::validParams();
    return params;
  }

  GPUNodalKernel(const InputParameters & parameters)
    : GPUNodalKernelBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
      _u(systems(), _var),
      _boundary_restricted(boundaryRestricted()),
      _default_diag(&NodalKernel::computeQpJacobian == &GPUNodalKernel::computeQpJacobian),
      _default_offdiag(&NodalKernel::computeQpOffDiagJacobian ==
                       &GPUNodalKernel::computeQpOffDiagJacobian)
  {
  }

  GPUNodalKernel(const GPUNodalKernel<NodalKernel> & object)
    : GPUNodalKernelBase(object),
      _u(object._u),
      _boundary_restricted(object._boundary_restricted),
      _default_diag(object._default_diag),
      _default_offdiag(object._default_offdiag)
  {
  }

  // Dispatch residual calculation to GPU
  virtual void computeResidual() override
  {
    if (!_var.isNodalDefined())
      return;

    Kokkos::RangePolicy<ResidualLoop, Kokkos::IndexType<size_t>> policy(
        0, _boundary_restricted ? numBoundaryNodes() : numBlockNodes());
    Kokkos::parallel_for(policy, *static_cast<NodalKernel *>(this));
    Kokkos::fence();
  }
  // Dispatch Jacobian calculation to GPU
  virtual void computeJacobian() override
  {
    if (!_var.isNodalDefined())
      return;

    if (!_default_diag)
    {
      Kokkos::RangePolicy<JacobianLoop, Kokkos::IndexType<size_t>> policy(
          0, _boundary_restricted ? numBoundaryNodes() : numBlockNodes());
      Kokkos::parallel_for(policy, *static_cast<NodalKernel *>(this));
      Kokkos::fence();
    }

    if (!_default_offdiag)
    {
      auto & sys = system(_gpu_var.sys());

      _thread.resize({sys.getCoupling(_gpu_var.var()).size(),
                      _boundary_restricted ? numBoundaryNodes() : numBlockNodes()});

      Kokkos::RangePolicy<OffDiagJacobianLoop, Kokkos::IndexType<size_t>> policy(0, _thread.size());
      Kokkos::parallel_for(policy, *static_cast<NodalKernel *>(this));
      Kokkos::fence();
    }
  }

  // Empty methods to prevent compile errors even when these methods were not hidden by the derived
  // class
  KOKKOS_FUNCTION Real computeQpJacobian(const dof_id_type /* node */) const { return 0; }
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* jvar */,
                                                const dof_id_type /* node */) const
  {
    return 0;
  }

  // Overloaded operators called by Kokkos::parallel_for
  KOKKOS_FUNCTION void operator()(ResidualLoop, const size_t tid) const
  {
    auto kernel = static_cast<const NodalKernel *>(this);
    auto node = _boundary_restricted ? boundaryNodeID(tid) : blockNodeID(tid);

    Real local_re = kernel->computeQpResidual(node);

    setTaggedLocalResidual(true, local_re, node);
  }
  KOKKOS_FUNCTION void operator()(JacobianLoop, const size_t tid) const
  {
    auto kernel = static_cast<const NodalKernel *>(this);
    auto node = _boundary_restricted ? boundaryNodeID(tid) : blockNodeID(tid);

    Real local_ke = kernel->computeQpJacobian(node);

    setTaggedLocalMatrix(true, local_ke, node, _gpu_var.var());
  }
  KOKKOS_FUNCTION void operator()(OffDiagJacobianLoop, const size_t tid) const
  {
    auto kernel = static_cast<const NodalKernel *>(this);
    auto node =
        _boundary_restricted ? boundaryNodeID(_thread(tid, 1)) : blockNodeID(_thread(tid, 1));

    auto & sys = system(_gpu_var.sys());
    auto jvar = sys.getCoupling(_gpu_var.var())[_thread(tid, 0)];

    Real local_ke = kernel->computeQpOffDiagJacobian(jvar, node);

    setTaggedLocalMatrix(true, local_ke, node, jvar);
  }

protected:
  // Value of the unknown variable this is acting on
  GPUVariableNodalValue _u;

private:
  // Whether this kernel is boundary-restricted
  const bool _boundary_restricted;
  // Whether default computeQpJacobian is used
  const bool _default_diag;
  // Whether default computeQpOffDiagJacobian is used
  const bool _default_offdiag;
};

#define usingGPUNodalKernelMembers(T)                                                              \
  usingGPUNodalKernelBaseMembers;                                                                  \
                                                                                                   \
protected:                                                                                         \
  using GPUNodalKernel<T>::_u;                                                                     \
                                                                                                   \
public:                                                                                            \
  using GPUNodalKernel<T>::operator();
