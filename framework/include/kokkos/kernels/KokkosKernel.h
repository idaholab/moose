//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelBase.h"
#include "KokkosPrecomputeHooks.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels.
 *
 * A Kokkos kernel computes its residual contribution as a sum of terms, each the product of a
 * coefficient computed once per quadrature point and one test function quantity:
 *
 *   R_i = sum_qp JxW (f_value psi_i + f_grad . grad psi_i)
 *
 * The user defines the coefficient hooks as inlined public template methods in their derived class
 * (not virtual overrides). The residual hooks, of which at least one must be defined, are
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION Real precomputeQpResidualValue(const unsigned int qp,
 *                                                AssemblyDatum & datum) const;
 * template <typename Derived>
 * KOKKOS_FUNCTION Real3 precomputeQpResidualGrad(const unsigned int qp,
 *                                                AssemblyDatum & datum) const;
 *
 * The Jacobian hooks follow the same pattern with additional arguments, and each is optional:
 * precomputeQpJacobian{Value,Grad}(j, qp, datum),
 * precomputeQpOffDiagJacobian{Value,Grad}(j, jvar, qp, datum), and
 * precomputeQpOffDiagJacobianScalar{Value,Grad}(j, jvar, qp, datum). Only the terms whose hooks
 * are defined are assembled. KokkosPrecomputeHookTraits.h documents the hook families.
 */
class Kernel : public KernelBase
{
public:
  static InputParameters validParams();

  /// Kernels assemble the products of precompute hooks with the test functions
  static constexpr bool uses_precompute_hooks = true;
  /// Kernels support the precomputeQpOffDiagJacobianScalar{Value,Grad}() hooks
  static constexpr bool supports_scalar_jacobian = true;

  /**
   * Constructor
   */
  Kernel(const InputParameters & parameters);

  /**
   * Dispatch residual calculation
   */
  virtual void computeResidual() override;
  /**
   * Dispatch diagonal and off-diagonal Jacobian calculation
   */
  virtual void computeJacobian() override;

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const;
  /**
   * The team operation called by Kokkos for each element, assembling the Jacobian with respect to a
   * displacement component of an object posed on the current configuration
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DisplacementJacobianLoop,
                                  const TeamMember<DisplacementJacobianLoop> & team,
                                  const Derived & kernel) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianScalarLoop, const ThreadID tid, const Derived & kernel) const;
  ///@}

  /**
   * The parallel computation bodies, which a derived class may hide by defining methods with the
   * same signature as inlined public methods
   */
  ///@{
  /**
   * Compute residual
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const;
  /**
   * Compute diagonal Jacobian
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const;
  /**
   * Compute off-diagonal Jacobian
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & kernel,
                                                      AssemblyDatum & datum) const;
  /**
   * Compute scalar off-diagonal Jacobian
   * @param kernel The kernel object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianScalarInternal(const Derived & kernel,
                                                            AssemblyDatum & datum) const;
  ///@}

protected:
  /**
   * Current test function
   */
  const VariableTestValue _test;
  /**
   * Gradient of the current test function
   */
  const VariableTestGradient _grad_test;
  /**
   * Current shape function
   */
  const VariablePhiValue _phi;
  /**
   * Gradient of the current shape function
   */
  const VariablePhiGradient _grad_phi;
  /**
   * Current solution at quadrature points
   */
  const VariableValue _u;
  /**
   * Gradient of the current solution at quadrature points
   */
  const VariableGradient _grad_u;
};

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::operator()(ResidualLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 1));

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  kernel.computeResidualInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::operator()(JacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 1));

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  kernel.computeJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 2));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getFieldCoupling(_kokkos_var.var())[_thread(tid, 1)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      jvar,
                      _configuration);

  datum.set_local_parallel(_thread(tid, 0), _thread.size(0));

  kernel.computeOffDiagJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::operator()(OffDiagJacobianScalarLoop, const ThreadID tid, const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(_thread(tid, 1));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getScalarCoupling(_kokkos_var.var())[_thread(tid, 0)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      jvar,
                      _configuration);

  kernel.computeOffDiagJacobianScalarInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::computeResidualInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(datum, precomputeResidualBody<false>(kernel, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::computeJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum, precomputeJacobianBody<false>(kernel, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::computeOffDiagJacobianInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(datum,
                                          precomputeOffDiagJacobianBody<false>(kernel, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::computeOffDiagJacobianScalarInternal(const Derived & kernel, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum, precomputeOffDiagJacobianScalarBody<false>(kernel, datum));
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel::operator()(DisplacementJacobianLoop,
                   const TeamMember<DisplacementJacobianLoop> & team,
                   const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(team.league_rank());
  const auto side = libMesh::invalid_uint;

  const auto & disp = _configuration.displacements();
  const auto c = _displacement_component;
  const auto jvar = disp.var(c);

  if (!kokkosSystem(_kokkos_var.sys())
           .isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar, _configuration);

  assembleDisplacementJacobian<false>(
      kernel,
      team,
      datum,
      c,
      [&](const Real local_ke, const unsigned int i, const unsigned int j)
      { accumulateTaggedElementalMatrix(local_ke, datum.elem().id, i, j, jvar); });
}

} // namespace Moose::Kokkos
