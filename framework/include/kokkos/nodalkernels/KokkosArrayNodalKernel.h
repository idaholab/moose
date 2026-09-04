//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "KokkosNodalKernelBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos nodal kernels on array variables.
 * Each device invocation operates on the component returned by AssemblyDatum::comp().
 */
class ArrayNodalKernel : public NodalKernelBase
{
public:
  static InputParameters validParams();

  ArrayNodalKernel(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    ::Kokkos::abort("Default computeQpJacobian() should never be called. Make sure you properly "
                    "redefined this method in your class without typos.");

    return 0;
  }

  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* jvar */,
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
    return &ArrayNodalKernel::computeQpJacobian<Derived>;
  }

  template <typename Derived>
  static auto defaultOffDiagJacobian()
  {
    return &ArrayNodalKernel::computeQpOffDiagJacobian<Derived>;
  }

  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const;

protected:
  const ArrayVariableValue _u;

  /// Number of components of the array variable
  const unsigned int _count;

private:
  const bool _boundary_restricted;
};

template <typename Derived>
KOKKOS_FUNCTION void
ArrayNodalKernel::operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto comp = _thread(tid, 0);
  const auto node = _boundary_restricted ? kokkosBoundaryNodeID(_thread(tid, 1))
                                         : kokkosBlockNodeID(_thread(tid, 1));
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));

  if (!sys.isNodalDefined(node, _kokkos_var.var(comp)))
    return;

  AssemblyDatum datum(
      node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var(comp), comp);

  const Real local_re = kernel.template computeQpResidual<Derived>(0, datum);

  accumulateTaggedNodalResidual(true, local_re, node, comp);
}

template <typename Derived>
KOKKOS_FUNCTION void
ArrayNodalKernel::operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto comp = _thread(tid, 0);
  const auto node = _boundary_restricted ? kokkosBoundaryNodeID(_thread(tid, 1))
                                         : kokkosBlockNodeID(_thread(tid, 1));
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));

  if (!sys.isNodalDefined(node, _kokkos_var.var(comp)))
    return;

  AssemblyDatum datum(
      node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var(comp), comp);

  const Real local_ke = kernel.template computeQpJacobian<Derived>(0, datum);

  accumulateTaggedNodalMatrix(true, local_ke, node, _kokkos_var.var(comp), comp);
}

template <typename Derived>
KOKKOS_FUNCTION void
ArrayNodalKernel::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  const auto comp = _thread(tid, 0);
  const auto node = _boundary_restricted ? kokkosBoundaryNodeID(_thread(tid, 2))
                                         : kokkosBlockNodeID(_thread(tid, 2));
  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  const auto & coupling = sys.getCoupling(_kokkos_var.var(comp));
  const auto coupling_index = _thread(tid, 1);

  if (coupling_index >= coupling.size() || !sys.isNodalDefined(node, _kokkos_var.var(comp)))
    return;

  const auto jvar = coupling[coupling_index];
  AssemblyDatum datum(node, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar, comp);

  const Real local_ke = kernel.template computeQpOffDiagJacobian<Derived>(jvar, 0, datum);

  accumulateTaggedNodalMatrix(true, local_ke, node, jvar, comp);
}

} // namespace Moose::Kokkos
