//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUVariableValue.h"
#include "GPUMaterialPropertyValue.h"

#include "MooseVariableBase.h"
#include "ResidualObject.h"

namespace Moose
{
namespace Kokkos
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
   * @param elem The element ID
   * @param i The test function index
   * @param comp The variable component
   */
  KOKKOS_FUNCTION inline void accumulateTaggedElementalResidual(Real local_re,
                                                                dof_id_type elem,
                                                                unsigned int i,
                                                                unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal residual contribution to tagged vectors
   * @param add The flag whether to add or set the local residual
   * @param local_re The local nodal residual contribution
   * @param node The node ID
   * @param comp The variable component
   */
  KOKKOS_FUNCTION inline void accumulateTaggedNodalResidual(bool add,
                                                            Real local_re,
                                                            dof_id_type node,
                                                            unsigned int comp = 0) const;
  /**
   * Accumulate local elemental Jacobian contribution to tagged matrices
   * @param local_ke The local elemental Jacobian contribution
   * @param elem The element ID
   * @param i The test function DOF index
   * @param j The trial function DOF index
   * @param jvar The variable number for column
   * @param comp The variable component
   */
  KOKKOS_FUNCTION inline void accumulateTaggedElementalMatrix(Real local_ke,
                                                              dof_id_type elem,
                                                              unsigned int i,
                                                              unsigned int j,
                                                              unsigned int jvar,
                                                              unsigned int comp = 0) const;
  /**
   * Accumulate or set local nodal Jacobian contribution to tagged matrices
   * @param add The flag whether to add or set the local residual
   * @param local_ke The local nodal Jacobian contribution
   * @param node The node ID
   * @param jvar The variable number for column
   * @param comp The Variable component
   */
  KOKKOS_FUNCTION inline void accumulateTaggedNodalMatrix(
      bool add, Real local_ke, dof_id_type node, unsigned int jvar, unsigned int comp = 0) const;

private:
  /**
   * Tags this object operates on
   */
  ///@{
  Array<TagID> _vector_tags;
  Array<TagID> _matrix_tags;
  ///@}
  /**
   * Flag whether each tag
   */
  ///@{
  Array<bool> _is_extra_vector_tag;
  Array<bool> _is_extra_matrix_tag;
  ///@}
};

KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedElementalResidual(Real local_re,
                                                  dof_id_type elem,
                                                  unsigned int i,
                                                  unsigned int comp) const
{
  if (!local_re)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto dof = sys.getElemLocalDofIndex(elem, i, _kokkos_var.var(comp));

  for (size_t t = 0; t < _vector_tags.size(); ++t)
  {
    auto tag = _vector_tags[t];

    if (sys.isResidualTagActive(tag))
    {
      bool has_nodal_bc =
          _is_extra_vector_tag[t] ? sys.hasNodalBCResidualTag(dof, tag) : sys.hasNodalBC(dof);

      if (!has_nodal_bc)
        ::Kokkos::atomic_add(&sys.getVectorDofValue(dof, tag), local_re);
    }
  }
}
KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedNodalResidual(bool add,
                                              Real local_re,
                                              dof_id_type node,
                                              unsigned int comp) const
{
  if (!local_re)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto dof = sys.getNodeLocalDofIndex(node, _kokkos_var.var(comp));

  for (size_t t = 0; t < _vector_tags.size(); ++t)
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
ResidualObject::accumulateTaggedElementalMatrix(Real local_ke,
                                                dof_id_type elem,
                                                unsigned int i,
                                                unsigned int j,
                                                unsigned int jvar,
                                                unsigned int comp) const
{
  if (!local_ke)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto row = sys.getElemLocalDofIndex(elem, i, _kokkos_var.var(comp));
  auto col = sys.getElemGlobalDofIndex(elem, j, jvar);

  for (size_t t = 0; t < _matrix_tags.size(); ++t)
  {
    auto tag = _matrix_tags[t];

    if (sys.isMatrixTagActive(tag))
    {
      bool has_nodal_bc =
          _is_extra_matrix_tag[t] ? sys.hasNodalBCMatrixTag(row, tag) : sys.hasNodalBC(row);

      if (!has_nodal_bc)
        ::Kokkos::atomic_add(&sys.getMatrixValue(row, col, tag), local_ke);
    }
  }
}
KOKKOS_FUNCTION inline void
ResidualObject::accumulateTaggedNodalMatrix(
    bool add, Real local_ke, dof_id_type node, unsigned int jvar, unsigned int comp) const
{
  if (!local_ke)
    return;

  auto & sys = kokkosSystem(_kokkos_var.sys());
  auto row = sys.getNodeLocalDofIndex(node, _kokkos_var.var(comp));
  auto col = sys.getNodeGlobalDofIndex(node, jvar);

  for (size_t t = 0; t < _matrix_tags.size(); ++t)
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

} // namespace Kokkos
} // namespace Moose

#define usingKokkosResidualObjectMembers                                                           \
public:                                                                                            \
  usingPostprocessorInterfaceMembers;                                                              \
                                                                                                   \
protected:                                                                                         \
  using Moose::Kokkos::ResidualObject::kokkosAssembly;                                             \
  using Moose::Kokkos::ResidualObject::kokkosSystems;                                              \
  using Moose::Kokkos::ResidualObject::kokkosSystem;                                               \
  using Moose::Kokkos::ResidualObject::accumulateTaggedElementalResidual;                          \
  using Moose::Kokkos::ResidualObject::accumulateTaggedNodalResidual;                              \
  using Moose::Kokkos::ResidualObject::accumulateTaggedElementalMatrix;                            \
  using Moose::Kokkos::ResidualObject::accumulateTaggedNodalMatrix;                                \
  using Moose::Kokkos::ResidualObject::_var;                                                       \
  using Moose::Kokkos::ResidualObject::_kokkos_var;                                                \
  using Moose::Kokkos::ResidualObject::_thread;                                                    \
  using Moose::Kokkos::ResidualObject::_t;                                                         \
  using Moose::Kokkos::ResidualObject::_t_old;                                                     \
  using Moose::Kokkos::ResidualObject::_t_step;                                                    \
  using Moose::Kokkos::ResidualObject::_dt;                                                        \
  using Moose::Kokkos::ResidualObject::_dt_old;                                                    \
                                                                                                   \
public:                                                                                            \
  using Moose::Kokkos::ResidualObject::ResidualLoop;                                               \
  using Moose::Kokkos::ResidualObject::JacobianLoop;                                               \
  using Moose::Kokkos::ResidualObject::OffDiagJacobianLoop;
