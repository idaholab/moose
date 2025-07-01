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

  // Constructor
  Kernel(const InputParameters & parameters)
    : KernelBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD), _u(_var), _grad_u(_var)
  {
    addMooseVariableDependency(&_var);
  }

  // Copy constructor
  Kernel(const Kernel<Derived> & object)
    : KernelBase(object), _u(object._u), _grad_u(object._grad_u)
  {
  }

  // Dispatch residual calculation to GPU
  virtual void computeResidual() override
  {
    setVariableDependency();

    ::Kokkos::RangePolicy<ResidualLoop, ::Kokkos::IndexType<size_t>> policy(0, numBlockElements());
    ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
    ::Kokkos::fence();

    setCacheFlags();
  }
  // Dispatch Jacobian calculation to GPU
  virtual void computeJacobian() override
  {
    if (!defaultJacobian() || !defaultOffDiagJacobian())
    {
      setVariableDependency();

      ::Kokkos::RangePolicy<JacobianLoop, ::Kokkos::IndexType<size_t>> policy(0,
                                                                              numBlockElements());
      ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
      ::Kokkos::fence();

      setCacheFlags();
    }

    if (!defaultOffDiagJacobian())
    {
      auto & sys = kokkosSystem(_kokkos_var.sys());

      _thread.resize({sys.getCoupling(_kokkos_var.var()).size(), numBlockElements()});

      ::Kokkos::RangePolicy<OffDiagJacobianLoop, ::Kokkos::IndexType<size_t>> policy(
          0, _thread.size());
      ::Kokkos::parallel_for(policy, *static_cast<Derived *>(this));
      ::Kokkos::fence();
    }
  }

  // Empty methods to prevent compile errors even when these methods were not hidden by the derived
  // class
  KOKKOS_FUNCTION Real computeQpJacobian(const unsigned int /* i */,
                                         const unsigned int /* j */,
                                         const unsigned int /* qp */,
                                         ResidualDatum & /* datum */) const
  {
    return 0;
  }
  KOKKOS_FUNCTION Real computeQpOffDiagJacobian(const unsigned int /* i */,
                                                const unsigned int /* j */,
                                                const unsigned int /* jvar */,
                                                const unsigned int /* qp */,
                                                ResidualDatum & /* datum */) const
  {
    return 0;
  }

  // Overloaded operators called by Kokkos::parallel_for
  KOKKOS_FUNCTION void operator()(ResidualLoop, const size_t tid) const
  {
    auto kernel = static_cast<const Derived *>(this);
    auto elem = blockElementID(tid);

    ResidualDatum datum(elem, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

    Real local_re[MAX_DOF];

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
      local_re[i] = 0;

    kernel->computeResidualInternal(kernel, datum, local_re);
  }
  KOKKOS_FUNCTION void operator()(JacobianLoop, const size_t tid) const
  {
    auto kernel = static_cast<const Derived *>(this);
    auto elem = blockElementID(tid);

    ResidualDatum datum(elem, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var());

    Real local_ke[MAX_DOF * MAX_DOF];

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        local_ke[i * datum.n_jdofs() + j] = 0;

    kernel->computeJacobianInternal(kernel, datum, local_ke);
  }
  KOKKOS_FUNCTION void operator()(OffDiagJacobianLoop, const size_t tid) const
  {
    auto kernel = static_cast<const Derived *>(this);
    auto elem = blockElementID(_thread(tid, 1));

    auto & sys = kokkosSystem(_kokkos_var.sys());
    auto jvar = sys.getCoupling(_kokkos_var.var())[_thread(tid, 0)];

    if (!sys.isVariableActive(jvar, kokkosMesh().getElementInfo(elem).subdomain))
      return;

    ResidualDatum datum(elem, kokkosAssembly(), kokkosSystems(), _kokkos_var, jvar);

    Real local_ke[MAX_DOF * MAX_DOF];

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        local_ke[i * datum.n_jdofs() + j] = 0;

    kernel->computeOffDiagJacobianInternal(kernel, datum, local_ke);
  }

  KOKKOS_FUNCTION void
  computeResidualInternal(const Derived * kernel, ResidualDatum & datum, Real * local_re) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int i = 0; i < datum.n_dofs(); ++i)
        local_re[i] += datum.JxW(qp) * kernel->computeQpResidual(i, qp, datum);
    }

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
      accumulateTaggedElementalResidual(local_re[i], datum.elem().id, i);
  }
  KOKKOS_FUNCTION void
  computeJacobianInternal(const Derived * kernel, ResidualDatum & datum, Real * local_ke) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int i = 0; i < datum.n_idofs(); ++i)
        for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
          local_ke[i * datum.n_jdofs() + j] +=
              datum.JxW(qp) * kernel->computeQpJacobian(i, j, qp, datum);
    }

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        accumulateTaggedElementalMatrix(
            local_ke[i * datum.n_jdofs() + j], datum.elem().id, i, j, datum.jvar());
  }
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const Derived * kernel,
                                                      ResidualDatum & datum,
                                                      Real * local_ke) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int i = 0; i < datum.n_idofs(); ++i)
        for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
          local_ke[i * datum.n_jdofs() + j] +=
              datum.JxW(qp) * kernel->computeQpOffDiagJacobian(i, j, datum.jvar(), qp, datum);
    }

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        accumulateTaggedElementalMatrix(
            local_ke[i * datum.n_jdofs() + j], datum.elem().id, i, j, datum.jvar());
  }

protected:
  // The current test function
  VariableTestValue _test;
  // Gradient of the test function
  VariableTestGradient _grad_test;
  // The current shape functions
  VariablePhiValue _phi;
  // Gradient of the shape function
  VariablePhiGradient _grad_phi;
  // Holds the solution at current quadrature points
  VariableValue _u;
  // Holds the solution gradient at the current quadrature points
  VariableGradient _grad_u;

protected:
  virtual bool defaultJacobian() const
  {
    return &Derived::computeQpJacobian == &Kernel::computeQpJacobian;
  }
  virtual bool defaultOffDiagJacobian() const
  {
    return &Derived::computeQpOffDiagJacobian == &Kernel::computeQpOffDiagJacobian;
  }
};

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
