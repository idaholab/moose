//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDispatcher.h"
#include "KokkosVariableValue.h"
#include "KokkosMaterialPropertyValue.h"

#include "MooseVariableBase.h"
#include "ResidualObject.h"

namespace Moose::Kokkos
{

/**
 * The base class for Kokkos residual objects
 */
class ResidualObject : public ::ResidualObject,
                       public MeshHolder,
                       public AssemblyHolder,
                       public SystemHolder
{
public:
  static InputParameters validParams();

  /**
   * Constructor
   * @param field_type The MOOSE variable field type
   * @param nodal Whether the residual object is a nodal residual object
   */
  ResidualObject(const InputParameters & parameters,
                 Moose::VarFieldType field_type,
                 bool nodal = false);
  /**
   * Copy constructor for parallel dispatch
   */
  ResidualObject(const ResidualObject & object);

  /**
   * Kokkos function tags
   */
  ///@{
  struct ResidualLoop
  {
  };
  struct JacobianLoop
  {
  };
  struct OffDiagJacobianLoop
  {
  };
  ///@}

  virtual const MooseVariableBase & variable() const override { return _var; }

  virtual void computeOffDiagJacobian(unsigned int) override final
  {
    mooseError("computeOffDiagJacobian() is not used for Kokkos residual objects.");
  }
  virtual void computeResidualAndJacobian() override final
  {
    computeResidual();
    computeJacobian();
  }

protected:
  /**
   * Reference of the MOOSE variable
   */
  MooseVariableFieldBase & _var;
  /**
   * Kokkos variable
   */
  Variable _kokkos_var;
  /**
   * Kokkos thread object
   */
  Thread _thread;
  /**
   * Kokkos functor dispatchers
   */
  ///@{
  std::unique_ptr<DispatcherBase> _residual_dispatcher;
  std::unique_ptr<DispatcherBase> _jacobian_dispatcher;
  std::unique_ptr<DispatcherBase> _offdiag_jacobian_dispatcher;
  ///@}

  /**
   * TODO: Move to TransientInterface
   */
  ///@{
  /**
   * Time
   */
  Scalar<Real> _t;
  /**
   * Old time
   */
  Scalar<const Real> _t_old;
  /**
   * The number of the time step
   */
  Scalar<int> _t_step;
  /**
   * Time step size
   */
  Scalar<Real> _dt;
  /**
   * Size of the old time step
   */
  Scalar<Real> _dt_old;
  ///@}

  /**
   * Accumulate local elemental residual contribution to tagged vectors
   * @param local_re The local elemental residual contribution
   * @param elem The contiguous element ID
   * @param i The test function index
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedElementalResidual(const Real local_re,
                                                         const ContiguousElementID elem,
                                                         const unsigned int i,
                                                         const unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal residual contribution to tagged vectors
   * @param add The flag whether to add or set the local residual
   * @param local_re The local nodal residual contribution
   * @param node The contiguous node ID
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedNodalResidual(const bool add,
                                                     const Real local_re,
                                                     const ContiguousNodeID node,
                                                     const unsigned int comp = 0) const;
  /**
   * Accumulate local elemental Jacobian contribution to tagged matrices
   * @param local_ke The local elemental Jacobian contribution
   * @param elem The contiguous element ID
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param jvar The variable number for column
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedElementalMatrix(const Real local_ke,
                                                       const ContiguousElementID elem,
                                                       const unsigned int i,
                                                       const unsigned int j,
                                                       const unsigned int jvar,
                                                       const unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal Jacobian contribution to tagged matrices
   * @param add The flag whether to add or set the local residual
   * @param local_ke The local nodal Jacobian contribution
   * @param node The contiguous node ID
   * @param jvar The variable number for column
   * @param comp The variable component
   */
  KOKKOS_FUNCTION void accumulateTaggedNodalMatrix(const bool add,
                                                   const Real local_ke,
                                                   const ContiguousNodeID node,
                                                   const unsigned int jvar,
                                                   const unsigned int comp = 0) const;

  /**
   * The common loop structure template for computing elemental residual
   * @param datum The AssemblyDatum object of the current thread
   * @param body The quadrature point loop body
   */
  template <typename function>
  KOKKOS_FUNCTION void computeResidualInternal(AssemblyDatum & datum, function body) const;
  /**
   * The common loop structure template for computing elemental Jacobian
   * @param datum The AssemblyDatum object of the current thread
   * @param body The quadrature point loop body
   */
  template <typename function>
  KOKKOS_FUNCTION void computeJacobianInternal(AssemblyDatum & datum, function body) const;

private:
  /**
   * Tags this object operates on
   */
  ///@{
  Array<TagID> _vector_tags;
  Array<TagID> _matrix_tags;
  ///@}
};

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedElementalResidual(const Real local_re,
                                                  const ContiguousElementID elem,
                                                  const unsigned int i,
                                                  const unsigned int comp) const
{
  if (!local_re)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto dof = sys.getElemLocalDofIndex(elem, i, _kokkos_var.var(comp));

  for (dof_id_type t = 0; t < _vector_tags.size(); ++t)
  {
    auto tag = _vector_tags[t];

    if (sys.isResidualTagActive(tag) && !sys.hasNodalBCResidualTag(dof, tag))
      ::Kokkos::atomic_add(&sys.getVectorDofValue(dof, tag), local_re);
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedNodalResidual(const bool add,
                                              const Real local_re,
                                              const ContiguousNodeID node,
                                              const unsigned int comp) const
{
  if (!local_re && add)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto dof = sys.getNodeLocalDofIndex(node, 0, _kokkos_var.var(comp));

  for (dof_id_type t = 0; t < _vector_tags.size(); ++t)
  {
    auto tag = _vector_tags[t];

    if (sys.isResidualTagActive(tag))
    {
      if (add)
        sys.getVectorDofValue(dof, tag) += local_re;
      else
        sys.getVectorDofValue(dof, tag) = local_re;
    }
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedElementalMatrix(const Real local_ke,
                                                const ContiguousElementID elem,
                                                const unsigned int i,
                                                const unsigned int j,
                                                const unsigned int jvar,
                                                const unsigned int comp) const
{
  if (!local_ke)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto row = sys.getElemLocalDofIndex(elem, i, _kokkos_var.var(comp));
  auto col = sys.getElemGlobalDofIndex(elem, j, jvar);

  for (dof_id_type t = 0; t < _matrix_tags.size(); ++t)
  {
    auto tag = _matrix_tags[t];

    if (sys.isMatrixTagActive(tag) && !sys.hasNodalBCMatrixTag(row, tag))
      ::Kokkos::atomic_add(&sys.getMatrixValue(row, col, tag), local_ke);
  }
}

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedNodalMatrix(const bool add,
                                            const Real local_ke,
                                            const ContiguousNodeID node,
                                            const unsigned int jvar,
                                            const unsigned int comp) const
{
  if (!local_ke && add)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys(comp));
  auto row = sys.getNodeLocalDofIndex(node, 0, _kokkos_var.var(comp));
  auto col = sys.getNodeGlobalDofIndex(node, jvar);

  for (dof_id_type t = 0; t < _matrix_tags.size(); ++t)
  {
    auto tag = _matrix_tags[t];

    if (sys.isMatrixTagActive(tag))
    {
      auto & matrix = sys.getMatrix(tag);

      if (add)
        matrix(row, col) += local_ke;
      else
      {
        matrix.zero(row);
        matrix(row, col) = local_ke;
      }
    }
  }
}

template <typename function>
KOKKOS_FUNCTION void
ResidualObject::computeResidualInternal(AssemblyDatum & datum, function body) const
{
  Real local_re[MAX_CACHED_DOF];

  unsigned int num_batches = datum.n_dofs() / MAX_CACHED_DOF;

  if (datum.n_dofs() % MAX_CACHED_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ib = batch * MAX_CACHED_DOF;
    unsigned int ie = ::Kokkos::min(ib + MAX_CACHED_DOF, datum.n_dofs());

    for (unsigned int i = ib; i < ie; ++i)
      local_re[i - ib] = 0;

    body(local_re - ib, ib, ie);

    for (unsigned int i = ib; i < ie; ++i)
      accumulateTaggedElementalResidual(local_re[i - ib], datum.elem().id, i);
  }
}

template <typename function>
KOKKOS_FUNCTION void
ResidualObject::computeJacobianInternal(AssemblyDatum & datum, function body) const
{
  Real local_ke[MAX_CACHED_DOF];

  unsigned int num_batches = datum.n_idofs() * datum.n_jdofs() / MAX_CACHED_DOF;

  if ((datum.n_idofs() * datum.n_jdofs()) % MAX_CACHED_DOF)
    ++num_batches;

  for (unsigned int batch = 0; batch < num_batches; ++batch)
  {
    unsigned int ijb = batch * MAX_CACHED_DOF;
    unsigned int ije = ::Kokkos::min(ijb + MAX_CACHED_DOF, datum.n_idofs() * datum.n_jdofs());

    for (unsigned int ij = ijb; ij < ije; ++ij)
      local_ke[ij - ijb] = 0;

    body(local_ke - ijb, ijb, ije);

    for (unsigned int ij = ijb; ij < ije; ++ij)
    {
      unsigned int i = ij % datum.n_jdofs();
      unsigned int j = ij / datum.n_jdofs();

      accumulateTaggedElementalMatrix(local_ke[ij - ijb], datum.elem().id, i, j, datum.jvar());
    }
  }
}

} // namespace Moose::Kokkos
