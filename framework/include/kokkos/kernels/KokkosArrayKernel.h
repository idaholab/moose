//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosKernelBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels on array variables.
 *
 * Each device invocation operates on the component returned by AssemblyDatum::comp(). Derived
 * classes define the same scalar-valued hooks as Kokkos kernels on scalar variables, and the array
 * value and gradient wrappers automatically access that component.
 */
class ArrayKernel : public KernelBase
{
public:
  static InputParameters validParams();

  ArrayKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* i */,
                                         const unsigned int /* j */,
                                         const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort("Default computeQpJacobian() should never be called. Make sure you properly "
                    "redefined this method in your class without typos.");

    return 0;
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* i */,
                                                const unsigned int /* j */,
                                                const unsigned int /* jvar */,
                                                const unsigned int /* qp */,
                                                AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort(
        "Default computeQpOffDiagJacobian() should never be called. Make sure you properly "
        "redefined this method in your class without typos.");

    return 0;
  }

  template <typename Derived>
  static auto defaultJacobian()
  {
    return &ArrayKernel::computeQpJacobian<Derived>;
  }

  template <typename Derived>
  static auto defaultOffDiagJacobian()
  {
    return &ArrayKernel::computeQpOffDiagJacobian<Derived>;
  }

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const;

  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const;
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & kernel,
                                                      AssemblyDatum & datum) const;

protected:
  const ArrayVariableTestValue _test;
  const ArrayVariableTestGradient _grad_test;
  const ArrayVariablePhiValue _phi;
  const ArrayVariablePhiGradient _grad_phi;
  const ArrayVariableValue _u;
  const ArrayVariableGradient _grad_u;

  /// Number of components of the array variable
  const unsigned int _count;
};

template <typename Derived>
KOKKOS_FUNCTION void
ArrayKernel::operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto comp = _thread(tid, 0);
  const auto elem = kokkosBlockElementID(_thread(tid, 2));

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(comp),
                      comp);

  datum.set_local_parallel(_thread(tid, 1), _thread.size(1));

  kernel.computeResidualInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ArrayKernel::operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto comp = _thread(tid, 0);
  const auto elem = kokkosBlockElementID(_thread(tid, 2));

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(comp),
                      comp);

  datum.set_local_parallel(_thread(tid, 1), _thread.size(1));

  kernel.computeJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ArrayKernel::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto comp = _thread(tid, 0);
  const auto elem = kokkosBlockElementID(_thread(tid, 3));
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  const auto & coupling = sys.getCoupling(_kokkos_var.var(comp));
  const auto coupling_index = _thread(tid, 2);

  if (coupling_index >= coupling.size())
    return;

  const auto jvar = coupling[coupling_index];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(
      elem, libMesh::invalid_uint, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar, comp);

  datum.set_local_parallel(_thread(tid, 1), _thread.size(1));

  kernel.computeOffDiagJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ArrayKernel::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += datum.JxW(qp) * kernel.template computeQpResidual<Derived>(i, qp, datum);
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
ArrayKernel::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] +=
                datum.JxW(qp) * kernel.template computeQpJacobian<Derived>(i, j, qp, datum);
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
ArrayKernel::computeOffDiagJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ib, const unsigned int ie, const unsigned int j)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
          for (unsigned int i = ib; i < ie; ++i)
            local_ke[i] += datum.JxW(qp) * kernel.template computeQpOffDiagJacobian<Derived>(
                                               i, j, datum.jvar(), qp, datum);
      });
}

} // namespace Moose::Kokkos
