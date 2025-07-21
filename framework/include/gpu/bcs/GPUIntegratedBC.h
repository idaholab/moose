//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUIntegratedBCBase.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for a user to derive his own Kokkos integrated boundary conditions.
 *
 * The polymorphic design of the original MOOSE is reproduced statically by leveraging the Curiously
 * Recurring Template Pattern (CRTP), a programming idiom that involves a class template inheriting
 * from a template instantiation of itself. When the user derives his Kokkos object from this class,
 * the inheritance structure will look like:
 *
 * class UserIntegratedBC final : public Moose::Kokkos::IntegratedBC<UserIntegratedBC>
 *
 * It is important to note that the template argument should point to the last derived class.
 * Therefore, if the user wants to define a derived class that can be further inherited, the derived
 * class should be a class template as well. Otherwise, it is recommended to mark the derived class
 * as final to prevent its inheritence by mistake.
 *
 * The user is expected to define computeQpResidual(), computeQpJacobian(), and
 * computeQpOffDiagJacobian() as inlined public methods in his derived class (not virtual override).
 * The signature of computeQpResidual() expected to be defined in the derived class is as follows:
 *
 * @param i The element-local DOF index
 * @param qp The local quadrature point index
 * @param datum The ResidualDatum object of the current thread
 * @returns The residual contribution
 *
 * KOKKOS_FUNCTION Real computeQpResidual(const unsigned int i,
 *                                        const unsigned int qp,
 *                                        ResidualDatum & datum) const;
 *
 * The signatures of computeQpJacobian() and computeOffDiagQpJacobian() can be found in the code
 * below, and their definition in the derived class is optional. If they are defined in the derived
 * class, they will hide the default definitions in the base class.
 */
template <typename Derived>
class IntegratedBC : public IntegratedBCBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = IntegratedBCBase::validParams();

    return params;
  }

  /**
   * Constructor
   */
  IntegratedBC(const InputParameters & parameters)
    : IntegratedBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD), _u(_var), _grad_u(_var)
  {
    addMooseVariableDependency(&_var);
  }

  /**
   * Copy constructor for parallel dispatch
   */
  IntegratedBC(const IntegratedBC<Derived> & object)
    : IntegratedBCBase(object), _u(object._u), _grad_u(object._grad_u)
  {
  }

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
   * @param datum The ResidualDatum object of the current thread
   * @returns The diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* i */,
                                         const unsigned int /* j */,
                                         const unsigned int /* qp */,
                                         ResidualDatum & /* datum */) const
  {
    return 0;
  }
  /**
   * Compute off-diagonal Jacobian contribution on a quadrature point
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param jvar The variable number for column
   * @param qp The local quadrature point index
   * @param datum The ResidualDatum object of the current thread
   * @returns The off-diagonal Jacobian contribution
   */
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* i */,
                                                const unsigned int /* j */,
                                                const unsigned int /* jvar */,
                                                const unsigned int /* qp */,
                                                ResidualDatum & /* datum */) const
  {
    return 0;
  }
  ///@}

  /**
   * The parallel computation entry functions called by Kokkos
   */
  ///@{
  KOKKOS_FUNCTION void operator()(ResidualLoop, const size_t tid) const;
  KOKKOS_FUNCTION void operator()(JacobianLoop, const size_t tid) const;
  KOKKOS_FUNCTION void operator()(OffDiagJacobianLoop, const size_t tid) const;
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
   * @param datum The ResidualDatum object of the current thread
   */
  KOKKOS_FUNCTION void computeResidualInternal(const Derived * bc, ResidualDatum & datum) const;
  /**
   * Compute diagonal Jacobian
   * @param bc The boundary condition object of the final derived type
   * @param datum The ResidualDatum object of the current thread
   */
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived * bc, ResidualDatum & datum) const;
  /**
   * Compute off-diagonal Jacobian
   * @param bc The boundary condition object of the final derived type
   * @param datum The ResidualDatum object of the current thread
   */
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived * bc,
                                                      ResidualDatum & datum) const;
  ///@}

protected:
  /**
   * Current test function
   */
  VariableTestValue _test;
  /**
   * Gradient of the current test function
   */
  VariableTestGradient _grad_test;
  /**
   * Current shape function
   */
  VariablePhiValue _phi;
  /**
   * Gradient of the current shape function
   */
  VariablePhiGradient _grad_phi;
  /**
   * Current solution at quadrature points
   */
  VariableValue _u;
  /**
   * Gradient of the current solution at quadrature points
   */
  VariableGradient _grad_u;

protected:
  /**
   * Get whether computeQpJacobian() was not defined in the derived class
   * @returns Whether computeQpJacobian() was not defined in the derived class
   */
  virtual bool defaultJacobian() const
  {
    return &Derived::computeQpJacobian == &IntegratedBC::computeQpJacobian;
  }
  /**
   * Get whether computeQpOffDiagJacobian() was not defined in the derived class
   * @returns Whether computeQpOffDiagJacobian() was not defined in the derived class
   */
  virtual bool defaultOffDiagJacobian() const
  {
    return &Derived::computeQpOffDiagJacobian == &IntegratedBC::computeQpOffDiagJacobian;
  }
};

template <typename Derived>
void
IntegratedBC<Derived>::computeResidual()
{
  ::Kokkos::RangePolicy<ResidualLoop, ExecSpace, ::Kokkos::IndexType<size_t>> policy(
      0, numBoundarySides());
  ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
  ::Kokkos::fence();
}

template <typename Derived>
void
IntegratedBC<Derived>::computeJacobian()
{
  if (!defaultJacobian())
  {
    ::Kokkos::RangePolicy<JacobianLoop, ExecSpace, ::Kokkos::IndexType<size_t>> policy(
        0, numBoundarySides());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();
  }

  if (!defaultOffDiagJacobian())
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    _thread.resize({sys.getCoupling(_kokkos_var.var()).size(), numBoundarySides()});

    ::Kokkos::RangePolicy<OffDiagJacobianLoop, ExecSpace, ::Kokkos::IndexType<size_t>> policy(
        0, _thread.size());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC<Derived>::operator()(ResidualLoop, const size_t tid) const
{
  auto bc = static_cast<const Derived *>(this);
  auto elem = boundaryElementSideID(tid);

  ResidualDatum datum(
      elem.first, elem.second, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  bc->computeResidualInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC<Derived>::operator()(JacobianLoop, const size_t tid) const
{
  auto bc = static_cast<const Derived *>(this);
  auto elem = boundaryElementSideID(tid);

  ResidualDatum datum(
      elem.first, elem.second, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  bc->computeJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC<Derived>::operator()(OffDiagJacobianLoop, const size_t tid) const
{
  auto bc = static_cast<const Derived *>(this);
  auto elem = boundaryElementSideID(_thread(tid, 1));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 0)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem.first).subdomain))
    return;

  ResidualDatum datum(
      elem.first, elem.second, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar);

  bc->computeOffDiagJacobianInternal(bc, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC<Derived>::computeResidualInternal(const Derived * bc, ResidualDatum & datum) const
{
  Real local_re[MAX_DOF];

  unsigned int num_batches = datum.n_dofs() / MAX_DOF;

  if (datum.n_dofs() % MAX_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ib = batch * MAX_DOF;
    unsigned int ie = ::Kokkos::min(ib + MAX_DOF, datum.n_dofs());

    for (unsigned int i = ib; i < ie; ++i)
      local_re[i - ib] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      for (unsigned int i = ib; i < ie; ++i)
        local_re[i - ib] += datum.JxW(qp) * bc->computeQpResidual(i, qp, datum);
    }

    for (unsigned int i = ib; i < ie; ++i)
      accumulateTaggedElementalResidual(local_re[i - ib], datum.elem().id, i);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC<Derived>::computeJacobianInternal(const Derived * bc, ResidualDatum & datum) const
{
  Real local_ke[MAX_DOF];

  unsigned int num_batches = datum.n_idofs() * datum.n_jdofs() / MAX_DOF;

  if ((datum.n_idofs() * datum.n_jdofs()) % MAX_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ijb = batch * MAX_DOF;
    unsigned int ije = ::Kokkos::min(ijb + MAX_DOF, datum.n_idofs() * datum.n_jdofs());

    for (unsigned int ij = ijb; ij < ije; ++ij)
      local_ke[ij - ijb] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      for (unsigned int ij = ijb; ij < ije; ++ij)
      {
        unsigned int i = ij % datum.n_jdofs();
        unsigned int j = ij / datum.n_jdofs();

        local_ke[ij - ijb] += datum.JxW(qp) * bc->computeQpJacobian(i, j, qp, datum);
      }
    }

    for (unsigned int ij = ijb; ij < ije; ++ij)
    {
      unsigned int i = ij % datum.n_jdofs();
      unsigned int j = ij / datum.n_jdofs();

      accumulateTaggedElementalMatrix(local_ke[ij - ijb], datum.elem().id, i, j, datum.jvar());
    }
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
IntegratedBC<Derived>::computeOffDiagJacobianInternal(const Derived * bc,
                                                      ResidualDatum & datum) const
{
  Real local_ke[MAX_DOF];

  unsigned int num_batches = datum.n_idofs() * datum.n_jdofs() / MAX_DOF;

  if ((datum.n_idofs() * datum.n_jdofs()) % MAX_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ijb = batch * MAX_DOF;
    unsigned int ije = ::Kokkos::min(ijb + MAX_DOF, datum.n_idofs() * datum.n_jdofs());

    for (unsigned int ij = ijb; ij < ije; ++ij)
      local_ke[ij - ijb] = 0;

    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit();

      for (unsigned int ij = ijb; ij < ije; ++ij)
      {
        unsigned int i = ij % datum.n_jdofs();
        unsigned int j = ij / datum.n_jdofs();

        local_ke[ij - ijb] +=
            datum.JxW(qp) * bc->computeQpOffDiagJacobian(i, j, datum.jvar(), qp, datum);
      }
    }

    for (unsigned int ij = ijb; ij < ije; ++ij)
    {
      unsigned int i = ij % datum.n_jdofs();
      unsigned int j = ij / datum.n_jdofs();

      accumulateTaggedElementalMatrix(local_ke[ij - ijb], datum.elem().id, i, j, datum.jvar());
    }
  }
}

} // namespace Kokkos
} // namespace Moose

#define usingKokkosIntegratedBCMembers(T)                                                          \
  usingKokkosIntegratedBCBaseMembers;                                                              \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::IntegratedBC<T>::_test;                                                     \
  using Moose::Kokkos::IntegratedBC<T>::_grad_test;                                                \
  using Moose::Kokkos::IntegratedBC<T>::_phi;                                                      \
  using Moose::Kokkos::IntegratedBC<T>::_grad_phi;                                                 \
  using Moose::Kokkos::IntegratedBC<T>::_u;                                                        \
  using Moose::Kokkos::IntegratedBC<T>::_grad_u;                                                   \
                                                                                                   \
public:                                                                                            \
  using Moose::Kokkos::IntegratedBC<T>::operator();
