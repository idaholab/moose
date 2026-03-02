//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCBase.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos integrated boundary conditions.
 *
 * The user should define computeQpResidual(), computeQpJacobian(), and computeQpOffDiagJacobian()
 * as inlined public methods in their derived class (not virtual override). The signature of
 * computeQpResidual() expected to be defined in the derived class is as follows:
 *
 * @param i The element-local DOF index
 * @param qp The local quadrature point index
 * @param datum The AssemblyDatum object of the current thread
 * @returns The residual contribution
 *
 * KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
 *                                        const unsigned int qp,
 *                                        AssemblyDatum & datum) const;
 *
 * The signatures of computeQpJacobian() and computeOffDiagQpJacobian() can be found in the code
 * below, and their definition in the derived class is optional. If they are defined in the derived
 * class, they will hide the default definitions in the base class.
 */
class IntegratedBC : public IntegratedBCBase
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   */
  IntegratedBC(const InputParameters & parameters);

  /**
   * Dispatch residual calculation
   */
  virtual void computeResidual() override;
  /**
   * Dispatch diagonal and off-diagonal Jacobian calculation
   */
  virtual void computeJacobian() override;

  /**
   * Default methods to prevent compile errors even when these methods were not defined in the
   * derived class
   */
  ///@{
  /**
   * Compute diagonal Jacobian contribution on a quadrature point
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param qp The local quadrature point index
   * @param datum The AssemblyDatum object of the current thread
   * @returns The diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* i */,
                                         const unsigned int /* j */,
                                         const unsigned int /* qp */,
                                         AssemblyDatum & /* datum */) const
  {
    return 0;
  }
  /**
   * Compute off-diagonal Jacobian contribution on a quadrature point
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param jvar The variable number for column
   * @param qp The local quadrature point index
   * @param datum The AssemblyDatum object of the current thread
   * @returns The off-diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* i */,
                                                const unsigned int /* j */,
                                                const unsigned int /* jvar */,
                                                const unsigned int /* qp */,
                                                AssemblyDatum & /* datum */) const
  {
    return 0;
  }
  /**
   * Get the function pointer of the default computeQpJacobian()
   * @returns The function pointer
   */
  static auto defaultJacobian() { return &IntegratedBC::computeQpJacobian; }
  /**
   * Get the function pointer of the default computeQpOffDiagJacobian()
   * @returns The function pointer
   */
  static auto defaultOffDiagJacobian() { return &IntegratedBC::computeQpOffDiagJacobian; }
  ///@}

  /**
   * Shims for hook methods that can be leveraged to implement static polymorphism
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpResidualShim(const Derived & bc,
                                             const unsigned int i,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
  {
    return bc.computeQpResidual(i, qp, datum);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpJacobianShim(const Derived & bc,
                                             const unsigned int i,
                                             const unsigned int j,
                                             const unsigned int qp,
                                             AssemblyDatum & datum) const
  {
    return bc.computeQpJacobian(i, j, qp, datum);
  }
  template <typename Derived>
  KOKKOS_FUNCTION Real computeQpOffDiagJacobianShim(const Derived & bc,
                                                    const unsigned int i,
                                                    const unsigned int j,
                                                    const unsigned int jvar,
                                                    const unsigned int qp,
                                                    AssemblyDatum & datum) const
  {
    return bc.computeQpOffDiagJacobian(i, j, jvar, qp, datum);
  }
  ///@}

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(JacobianLoop, const ThreadID tid, const Derived & bc) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & bc) const;
  ///@}

  /**
   * The parallel computation bodies that can be customized in the derived class by defining
   * them in the derived class with the same signature.
   * Make sure to define them as inlined public methods if to be defined in the derived class.
   */
  ///@{
  /**
   * Compute residual
   * @param bc The boundary condition object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const;
  /**
   * Compute diagonal Jacobian
   * @param bc The boundary condition object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const;
  /**
   * Compute off-diagonal Jacobian
   * @param bc The boundary condition object of the final derived type
   * @param datum The AssemblyDatum object of the current thread
   */
  template <typename Derived>
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived & bc,
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
IntegratedBC::operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(tid);

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  bc.computeResidualInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(JacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(tid);

  AssemblyDatum datum(
      elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  bc.computeJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::operator()(OffDiagJacobianLoop, const ThreadID tid, const Derived & bc) const
{
  auto [elem, side] = kokkosBoundaryElementSideID(_thread(tid, 1));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 0)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  AssemblyDatum datum(elem, side, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar);

  bc.computeOffDiagJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::computeResidualInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeResidualInternal(
      datum,
      [&](Real * local_re, const unsigned int ib, const unsigned int ie)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          for (unsigned int i = ib; i < ie; ++i)
            local_re[i] += datum.JxW(qp) * bc.computeQpResidualShim(bc, i, qp, datum);
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::computeJacobianInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ijb, const unsigned int ije)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          for (unsigned int ij = ijb; ij < ije; ++ij)
          {
            unsigned int i = ij % datum.n_jdofs();
            unsigned int j = ij / datum.n_jdofs();

            local_ke[ij] += datum.JxW(qp) * bc.computeQpJacobianShim(bc, i, j, qp, datum);
          }
        }
      });
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC::computeOffDiagJacobianInternal(const Derived & bc, AssemblyDatum & datum) const
{
  ResidualObject::computeJacobianInternal(
      datum,
      [&](Real * local_ke, const unsigned int ijb, const unsigned int ije)
      {
        for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
        {
          datum.reinit();

          for (unsigned int ij = ijb; ij < ije; ++ij)
          {
            unsigned int i = ij % datum.n_jdofs();
            unsigned int j = ij / datum.n_jdofs();

            local_ke[ij] +=
                datum.JxW(qp) * bc.computeQpOffDiagJacobianShim(bc, i, j, datum.jvar(), qp, datum);
          }
        }
      });
}

} // namespace Moose::Kokkos
