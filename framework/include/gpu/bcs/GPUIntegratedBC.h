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

#include "MooseVariableInterface.h"

template <typename IntegratedBC>
class GPUIntegratedBC : public GPUIntegratedBCBase
{
public:
  static InputParameters validParams()
  {
    InputParameters params = GPUIntegratedBCBase::validParams();

    return params;
  }

  GPUIntegratedBC(const InputParameters & parameters)
    : GPUIntegratedBCBase(parameters, Moose::VarFieldType::VAR_FIELD_STANDARD),
      _test(assembly()),
      _grad_test(assembly()),
      _phi(assembly()),
      _grad_phi(assembly()),
      _u(systems(), _var),
      _grad_u(systems(), _var)
  {
    addMooseVariableDependency(&_var);
  }

  GPUIntegratedBC(const GPUIntegratedBC<IntegratedBC> & object)
    : GPUIntegratedBCBase(object),
      _test(object._test),
      _grad_test(object._grad_test),
      _phi(object._phi),
      _grad_phi(object._grad_phi),
      _u(object._u),
      _grad_u(object._grad_u)
  {
  }

  // Dispatch residual calculation to GPU
  virtual void computeResidual() override
  {
    Kokkos::RangePolicy<ResidualLoop, Kokkos::IndexType<size_t>> policy(0, numBoundarySides());
    Kokkos::parallel_for(policy, *static_cast<IntegratedBC *>(this));
    Kokkos::fence();
  }
  // Dispatch diagonal Jacobian calculation to GPU
  virtual void computeJacobian() override
  {
    if (!defaultJacobian())
    {
      Kokkos::RangePolicy<JacobianLoop, Kokkos::IndexType<size_t>> policy(0, numBoundarySides());
      Kokkos::parallel_for(policy, *static_cast<IntegratedBC *>(this));
      Kokkos::fence();
    }

    if (!defaultOffDiagJacobian())
    {
      auto & sys = system(_gpu_var.sys());

      _thread.resize({sys.getCoupling(_gpu_var.var()).size(), numBoundarySides()});

      Kokkos::RangePolicy<OffDiagJacobianLoop, Kokkos::IndexType<size_t>> policy(0, _thread.size());
      Kokkos::parallel_for(policy, *static_cast<IntegratedBC *>(this));
      Kokkos::fence();
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
    auto bc = static_cast<const IntegratedBC *>(this);
    auto elem = boundaryElementSideID(tid);

    ResidualDatum datum(elem.first, elem.second, assembly(), systems(), _gpu_var, _gpu_var.var());

    Real local_re[MAX_DOF];

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
      local_re[i] = 0;

    computeResidualInternal(bc, datum, local_re);
  }
  KOKKOS_FUNCTION void operator()(JacobianLoop, const size_t tid) const
  {
    auto bc = static_cast<const IntegratedBC *>(this);
    auto elem = boundaryElementSideID(tid);

    ResidualDatum datum(elem.first, elem.second, assembly(), systems(), _gpu_var, _gpu_var.var());

    Real local_ke[MAX_DOF * MAX_DOF];

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        local_ke[i * datum.n_jdofs() + j] = 0;

    computeJacobianInternal(bc, datum, local_ke);
  }
  KOKKOS_FUNCTION void operator()(OffDiagJacobianLoop, const size_t tid) const
  {
    auto bc = static_cast<const IntegratedBC *>(this);
    auto elem = boundaryElementSideID(_thread(tid, 1));

    auto & sys = system(_gpu_var.sys());
    auto jvar = sys.getCoupling(_gpu_var.var())[_thread(tid, 0)];

    if (!sys.isVariableActive(jvar, mesh().getElementInfo(elem.first).subdomain))
      return;

    ResidualDatum datum(elem.first, elem.second, assembly(), systems(), _gpu_var, jvar);

    Real local_ke[MAX_DOF * MAX_DOF];

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        local_ke[i * datum.n_jdofs() + j] = 0;

    computeOffDiagJacobianInternal(bc, datum, local_ke);
  }

  KOKKOS_FUNCTION void
  computeResidualInternal(const IntegratedBC * bc, ResidualDatum & datum, Real * local_re) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int i = 0; i < datum.n_dofs(); ++i)
        local_re[i] += datum.JxW(qp) * bc->computeQpResidual(i, qp, datum);
    }

    for (unsigned int i = 0; i < datum.n_dofs(); ++i)
      accumulateTaggedLocalResidual(local_re[i], datum.elem().id, i);
  }
  KOKKOS_FUNCTION void
  computeJacobianInternal(const IntegratedBC * bc, ResidualDatum & datum, Real * local_ke) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int i = 0; i < datum.n_idofs(); ++i)
        for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
          local_ke[i * datum.n_jdofs() + j] +=
              datum.JxW(qp) * bc->computeQpJacobian(i, j, qp, datum);
    }

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        accumulateTaggedLocalMatrix(
            local_ke[i * datum.n_jdofs() + j], datum.elem().id, i, j, datum.jvar());
  }
  KOKKOS_FUNCTION void computeOffDiagJacobianInternal(const IntegratedBC * bc,
                                                      ResidualDatum & datum,
                                                      Real * local_ke) const
  {
    for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
    {
      datum.reinit(qp);

      for (unsigned int i = 0; i < datum.n_idofs(); ++i)
        for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
          local_ke[i * datum.n_jdofs() + j] +=
              datum.JxW(qp) * bc->computeQpOffDiagJacobian(i, j, datum.jvar(), qp, datum);
    }

    for (unsigned int i = 0; i < datum.n_idofs(); ++i)
      for (unsigned int j = 0; j < datum.n_jdofs(); ++j)
        accumulateTaggedLocalMatrix(
            local_ke[i * datum.n_jdofs() + j], datum.elem().id, i, j, datum.jvar());
  }

protected:
  // The current test function
  GPUVariableTestValue _test;
  // Gradient of the test function
  GPUVariableTestGradient _grad_test;
  // The current shape functions
  GPUVariablePhiValue _phi;
  // Gradient of the shape function
  GPUVariablePhiGradient _grad_phi;
  // Holds the solution at current quadrature points
  GPUVariableValue _u;
  // Holds the solution gradient at the current quadrature points
  GPUVariableGradient _grad_u;

protected:
  virtual bool defaultJacobian() const
  {
    return &IntegratedBC::computeQpJacobian == &GPUIntegratedBC::computeQpJacobian;
  }
  virtual bool defaultOffDiagJacobian() const
  {
    return &IntegratedBC::computeQpOffDiagJacobian == &GPUIntegratedBC::computeQpOffDiagJacobian;
  }
};

#define usingGPUIntegratedBCMembers(T)                                                             \
  usingGPUIntegratedBCBaseMembers;                                                                 \
                                                                                                   \
protected:                                                                                         \
  using GPUIntegratedBC<T>::_test;                                                                 \
  using GPUIntegratedBC<T>::_grad_test;                                                            \
  using GPUIntegratedBC<T>::_phi;                                                                  \
  using GPUIntegratedBC<T>::_grad_phi;                                                             \
  using GPUIntegratedBC<T>::_u;                                                                    \
  using GPUIntegratedBC<T>::_grad_u;                                                               \
                                                                                                   \
public:                                                                                            \
  using GPUIntegratedBC<T>::operator();
