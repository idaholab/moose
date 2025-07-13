//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUKernelBase.h"

namespace Moose
{
namespace Kokkos
{

/**
 * The base class for a user to derive his own Kokkos kernels.
 *
 * The polymorphic design of the original MOOSE is reproduced statically by leveraging the Curiously
 * Recurring Template Pattern (CRTP), a programming idiom that involves a class template inheriting
 * from a template instantiation of itself. When the user derives his Kokkos object from this class,
 * the inheritance structure will look like:
 *
 * class UserKernel final : public Moose::Kokkos::Kernel<UserKernel>
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
class Kernel : public KernelBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = KernelBase::validParams();
    params.registerBase("Kernel");
    return params;
  }

  /**
   * Constructor
   */
  Kernel(const InputParameters & parameters)
    : KernelBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD), _u(_var), _grad_u(_var)
  {
    addMooseVariableDependency(&_var);
  }

  /**
   * Copy constructor for parallel dispatch
   */
  Kernel(const Kernel<Derived> & object)
    : KernelBase(object), _u(object._u), _grad_u(object._grad_u)
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
   * @param kernel The kernel object of the final derived type
   * @param datum The ResidualDatum object of the current thread
   */
  KOKKOS_FUNCTION void computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const;
  /**
   * Compute diagonal Jacobian
   * @param kernel The kernel object of the final derived type
   * @param datum The ResidualDatum object of the current thread
   */
  KOKKOS_FUNCTION void computeJacobianInternal(const Derived * kernel, ResidualDatum & datum) const;
  /**
   * Compute off-diagonal Jacobian
   * @param kernel The kernel object of the final derived type
   * @param datum The ResidualDatum object of the current thread
   */
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived * kernel,
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

  /**
   * Get whether computeQpJacobian() was not defined in the derived class
   * @returns Whether computeQpJacobian() was not defined in the derived class
   */
  virtual bool defaultJacobian() const
  {
    return &Derived::computeQpJacobian == &Kernel::computeQpJacobian;
  }
  /**
   * Get whether computeQpOffDiagJacobian() was not defined in the derived class
   * @returns Whether computeQpOffDiagJacobian() was not defined in the derived class
   */
  virtual bool defaultOffDiagJacobian() const
  {
    return &Derived::computeQpOffDiagJacobian == &Kernel::computeQpOffDiagJacobian;
  }
};

template <typename Derived>
void
Kernel<Derived>::computeResidual()
{
  ::Kokkos::RangePolicy<ResidualLoop, ::Kokkos::IndexType<size_t>> policy(0, numBlockElements());
  ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
  ::Kokkos::fence();
}

template <typename Derived>
void
Kernel<Derived>::computeJacobian()
{
  if (!defaultJacobian())
  {
    ::Kokkos::RangePolicy<JacobianLoop, ::Kokkos::IndexType<size_t>> policy(0, numBlockElements());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();
  }

  if (!defaultOffDiagJacobian())
  {
    auto & sys = kokkosSystem(_kokkos_var.sys());

    _thread.resize({sys.getCoupling(_kokkos_var.var()).size(), numBlockElements()});

    ::Kokkos::RangePolicy<OffDiagJacobianLoop, ::Kokkos::IndexType<size_t>> policy(0,
                                                                                   _thread.size());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel<Derived>::operator()(ResidualLoop, const size_t tid) const
{
  auto kernel = static_cast<const Derived *>(this);
  auto elem = blockElementID(tid);

  ResidualDatum datum(elem, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  kernel->computeResidualInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel<Derived>::operator()(JacobianLoop, const size_t tid) const
{
  auto kernel = static_cast<const Derived *>(this);
  auto elem = blockElementID(tid);

  ResidualDatum datum(elem, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

  kernel->computeJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel<Derived>::operator()(OffDiagJacobianLoop, const size_t tid) const
{
  auto kernel = static_cast<const Derived *>(this);
  auto elem = blockElementID(_thread(tid, 1));

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 0)];

  if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
    return;

  ResidualDatum datum(elem, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar);

  kernel->computeOffDiagJacobianInternal(kernel, datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel<Derived>::computeResidualInternal(const Derived * kernel, ResidualDatum & datum) const
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
        local_re[i - ib] += datum.JxW(qp) * kernel->computeQpResidual(i, qp, datum);
    }

    for (unsigned int i = ib; i < ie; ++i)
      accumulateTaggedElementalResidual(local_re[i - ib], datum.elem().id, i);
  }
}

template <typename Derived>
KOKKOS_FUNCTION void
Kernel<Derived>::computeJacobianInternal(const Derived * kernel, ResidualDatum & datum) const
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

        local_ke[ij - ijb] += datum.JxW(qp) * kernel->computeQpJacobian(i, j, qp, datum);
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
Kernel<Derived>::computeOffDiagJacobianInternal(const Derived * kernel, ResidualDatum & datum) const
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
            datum.JxW(qp) * kernel->computeQpOffDiagJacobian(i, j, datum.jvar(), qp, datum);
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

#define usingKokkosKernelMembers(T)                                                                \
  usingKokkosKernelBaseMembers;                                                                    \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::Kernel<T>::_test;                                                           \
  using Moose::Kokkos::Kernel<T>::_grad_test;                                                      \
  using Moose::Kokkos::Kernel<T>::_phi;                                                            \
  using Moose::Kokkos::Kernel<T>::_grad_phi;                                                       \
  using Moose::Kokkos::Kernel<T>::_u;                                                              \
  using Moose::Kokkos::Kernel<T>::_grad_u;                                                         \
                                                                                                   \
public:                                                                                            \
  using Moose::Kokkos::Kernel<T>::operator();
