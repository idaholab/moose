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

class GPUResidualObject : public ResidualObject,
                          public GPUMeshHolder,
                          public GPUAssemblyHolder,
                          public GPUSystemHolder
{
public:
  static InputParameters validParams();

  GPUResidualObject(const InputParameters & parameters,
                    Moose::VarFieldType field_type,
                    bool nodal = false);
  GPUResidualObject(const GPUResidualObject & object);

  // GPU function tags
  struct ResidualLoop
  {
  };
  struct JacobianLoop
  {
  };
  struct OffDiagJacobianLoop
  {
  };

  virtual const MooseVariableBase & variable() const override { return _var; }

  virtual void computeOffDiagJacobian(unsigned int) override final
  {
    mooseError("computeOffDiagJacobian() is not used for GPU residual objects.");
  }
  virtual void computeResidualAndJacobian() override final
  {
    computeResidual();
    computeJacobian();
  }

protected:
  // Reference to MooseVariableFieldBase
  MooseVariableFieldBase & _var;
  // GPU variable this object operates on
  GPUVariable _gpu_var;
  // GPU thread object
  GPUThread _thread;

protected:
  // TODO: Move to TransientInterface
  // Time
  GPUScalar<Real> _t;
  // Old time
  GPUScalar<const Real> _t_old;
  // The number of the time step
  GPUScalar<int> _t_step;
  // Time step size
  GPUScalar<Real> _dt;
  // Size of the old time step
  GPUScalar<Real> _dt_old;

private:
  // Tags this object operates on
  GPUArray<TagID> _vector_tags;
  GPUArray<TagID> _matrix_tags;
  // Whether the tags are extra
  GPUArray<bool> _is_extra_vector_tag;
  GPUArray<bool> _is_extra_matrix_tag;

protected:
  // Accumulate local residual to tagged vectors
  KOKKOS_FUNCTION void accumulateTaggedLocalResidual(Real local_re,
                                                     dof_id_type elem,
                                                     unsigned int i,
                                                     unsigned int comp = 0) const
  {
    if (!local_re)
      return;

    auto & sys = system(_gpu_var.sys());
    auto dof = sys.getElemLocalDofIndex(elem, i, _gpu_var.var(comp));

    for (size_t t = 0; t < _vector_tags.size(); ++t)
    {
      auto tag = _vector_tags[t];

      if (sys.isResidualTagActive(tag))
      {
        bool has_nodal_bc =
            _is_extra_vector_tag[t] ? sys.hasNodalBCResidualTag(dof, tag) : sys.hasNodalBC(dof);

        if (!has_nodal_bc)
          Kokkos::atomic_add(&sys.getVectorDofValue(dof, tag), local_re);
      }
    }
  }
  // Set local residual to tagged vectors
  KOKKOS_FUNCTION void
  setTaggedLocalResidual(bool add, Real local_re, dof_id_type node, unsigned int comp = 0) const
  {
    if (!local_re)
      return;

    auto & sys = system(_gpu_var.sys());
    auto dof = sys.getNodeLocalDofIndex(node, _gpu_var.var(comp));

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
  // Accumulate local Jacobian to tagged matrices
  KOKKOS_FUNCTION void accumulateTaggedLocalMatrix(Real local_ke,
                                                   dof_id_type elem,
                                                   unsigned int i,
                                                   unsigned int j,
                                                   unsigned int jvar,
                                                   unsigned int comp = 0) const
  {
    if (!local_ke)
      return;

    auto & sys = system(_gpu_var.sys());
    auto row = sys.getElemLocalDofIndex(elem, i, _gpu_var.var(comp));
    auto col = sys.getElemGlobalDofIndex(elem, j, jvar);

    for (size_t t = 0; t < _matrix_tags.size(); ++t)
    {
      auto tag = _matrix_tags[t];

      if (sys.isMatrixTagActive(tag))
      {
        bool has_nodal_bc =
            _is_extra_matrix_tag[t] ? sys.hasNodalBCMatrixTag(row, tag) : sys.hasNodalBC(row);

        if (!has_nodal_bc)
          Kokkos::atomic_add(&sys.getMatrixDofValue(row, col, tag), local_ke);
      }
    }
  }
  // Set local Jacobian to tagged matrices
  KOKKOS_FUNCTION void setTaggedLocalMatrix(
      bool add, Real local_ke, dof_id_type node, unsigned int jvar, unsigned int comp = 0) const
  {
    if (!local_ke)
      return;

    auto & sys = system(_gpu_var.sys());
    auto row = sys.getNodeLocalDofIndex(node, _gpu_var.var(comp));
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
};

#define usingGPUResidualObjectMembers                                                              \
public:                                                                                            \
  usingPostprocessorInterfaceMembers;                                                              \
                                                                                                   \
protected:                                                                                         \
  using GPUResidualObject::assembly;                                                               \
  using GPUResidualObject::systems;                                                                \
  using GPUResidualObject::system;                                                                 \
  using GPUResidualObject::accumulateTaggedLocalResidual;                                          \
  using GPUResidualObject::setTaggedLocalResidual;                                                 \
  using GPUResidualObject::accumulateTaggedLocalMatrix;                                            \
  using GPUResidualObject::setTaggedLocalMatrix;                                                   \
  using GPUResidualObject::_var;                                                                   \
  using GPUResidualObject::_gpu_var;                                                               \
  using GPUResidualObject::_thread;                                                                \
  using GPUResidualObject::_t;                                                                     \
  using GPUResidualObject::_t_old;                                                                 \
  using GPUResidualObject::_t_step;                                                                \
  using GPUResidualObject::_dt;                                                                    \
  using GPUResidualObject::_dt_old;                                                                \
                                                                                                   \
public:                                                                                            \
  using GPUResidualObject::ResidualLoop;                                                           \
  using GPUResidualObject::JacobianLoop;                                                           \
  using GPUResidualObject::OffDiagJacobianLoop;
