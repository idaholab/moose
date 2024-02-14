//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseArray.h"
#include "Function.h"
#include "libmesh/vector_value.h"
#include <vector>

template <typename>
class MooseVariableFE;
class MooseVariableScalar;
template <typename>
class MooseArray;
class SystemBase;
class MooseMesh;
class HybridizedData;
class MooseObject;
class MaterialPropertyInterface;

class DiffusionHybridizedInterface
{
public:
  static InputParameters validParams();

  DiffusionHybridizedInterface(const MooseObject * moose_obj,
                               MaterialPropertyInterface * mpi,
                               SystemBase & nl_sys,
                               SystemBase & aux_sys,
                               const THREAD_ID tid);

protected:
  /**
   * Set the number of degree of freedom variables and resize the Eigen data structures
   */
  template <typename DiffusionHybridized>
  static void resizeData(DiffusionHybridized & obj);

  template <typename DiffusionHybridized>
  static void vectorVolumeResidual(DiffusionHybridized & obj,
                                   const unsigned int i_offset,
                                   const MooseArray<Gradient> & vector_sol,
                                   const MooseArray<Number> & scalar_sol);

  template <typename DiffusionHybridized>
  static void vectorVolumeJacobian(DiffusionHybridized & obj,
                                   const unsigned int i_offset,
                                   const unsigned int vector_j_offset,
                                   const unsigned int scalar_j_offset);

  template <typename DiffusionHybridized>
  static void scalarVolumeResidual(DiffusionHybridized & obj,
                                   const unsigned int i_offset,
                                   const MooseArray<Gradient> & vector_field,
                                   const Function & source);

  template <typename DiffusionHybridized>
  static void scalarVolumeJacobian(DiffusionHybridized & obj,
                                   const unsigned int i_offset,
                                   const unsigned int vector_field_j_offset);

  //
  // Methods which are leveraged both on internal sides in the kernel and by natural conditions
  //

  template <typename DiffusionHybridized>
  static void vectorFaceResidual(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Number> & lm_sol);

  template <typename DiffusionHybridized>
  static void vectorFaceJacobian(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const unsigned int lm_j_offset);

  template <typename DiffusionHybridized>
  static void scalarFaceResidual(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Gradient> & vector_sol,
                                 const MooseArray<Number> & scalar_sol,
                                 const MooseArray<Number> & lm_sol);

  template <typename DiffusionHybridized>
  static void scalarFaceJacobian(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const unsigned int vector_j_offset,
                                 const unsigned int scalar_j_offset,
                                 const unsigned int lm_j_offset);

  template <typename DiffusionHybridized>
  static void lmFaceResidual(DiffusionHybridized & obj,
                             const unsigned int i_offset,
                             const MooseArray<Gradient> & vector_sol,
                             const MooseArray<Number> & scalar_sol,
                             const MooseArray<Number> & lm_sol);

  template <typename DiffusionHybridized>
  static void lmFaceJacobian(DiffusionHybridized & obj,
                             const unsigned int i_offset,
                             const unsigned int vector_j_offset,
                             const unsigned int scalar_j_offset,
                             const unsigned int lm_j_offset);

  const MooseVariableFE<Real> & _u_var;
  const MooseVariableFE<RealVectorValue> & _grad_u_var;
  const MooseVariableFE<Real> & _u_face_var;

  // Containers for dof indices
  const std::vector<dof_id_type> & _qu_dof_indices;
  const std::vector<dof_id_type> & _u_dof_indices;
  const std::vector<dof_id_type> & _lm_u_dof_indices;

  // local solutions at quadrature points
  const MooseArray<Gradient> & _qu_sol;
  const MooseArray<Number> & _u_sol;
  const MooseArray<Number> & _lm_u_sol;

  // Element shape functions
  const MooseArray<std::vector<RealVectorValue>> & _vector_phi;
  const MooseArray<std::vector<Real>> & _scalar_phi;
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi;
  const MooseArray<std::vector<Real>> & _div_vector_phi;

  // Face shape functions
  const MooseArray<std::vector<RealVectorValue>> & _vector_phi_face;
  const MooseArray<std::vector<Real>> & _scalar_phi_face;
  const MooseArray<std::vector<Real>> & _lm_phi_face;

  // Material properties
  const MaterialProperty<Real> & _diff;

  // Number of dofs on elem
  std::size_t _vector_n_dofs;
  std::size_t _scalar_n_dofs;
  std::size_t _lm_n_dofs;

  /// Local sizes of the systems
  std::size_t _primal_size, _lm_size;

  /// Our stabilization coefficient
  static constexpr Real _tau = 1;
};

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::resizeData(DiffusionHybridized & obj)
{
  obj._vector_n_dofs = obj._qu_dof_indices.size();
  obj._scalar_n_dofs = obj._u_dof_indices.size();
  obj._lm_n_dofs = obj._lm_u_dof_indices.size();

  libmesh_assert_equal_to(obj._vector_n_dofs, obj._vector_phi.size());
  libmesh_assert_equal_to(obj._scalar_n_dofs, obj._scalar_phi.size());

  obj._primal_size = obj._vector_n_dofs + obj._scalar_n_dofs;
  obj._lm_size = obj._lm_n_dofs;

  // prepare our matrix/vector data structures
  obj._PrimalMat.setZero(obj._primal_size, obj._primal_size);
  obj._PrimalVec.setZero(obj._primal_size);
  obj._LMMat.setZero(obj._lm_size, obj._lm_size);
  obj._LMVec.setZero(obj._lm_size);
  obj._PrimalLM.setZero(obj._primal_size, obj._lm_size);
  obj._LMPrimal.setZero(obj._lm_size, obj._primal_size);
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::vectorVolumeResidual(DiffusionHybridized & obj,
                                                   const unsigned int i_offset,
                                                   const MooseArray<Gradient> & vector_sol,
                                                   const MooseArray<Number> & scalar_sol)
{
  for (const auto qp : make_range(obj._qrule->n_points()))
    for (const auto i : make_range(obj._vector_n_dofs))
    {
      // Vector equation dependence on vector dofs
      obj._PrimalVec(i_offset + i) += obj._JxW[qp] * (obj._vector_phi[i][qp] * vector_sol[qp]);

      // Vector equation dependence on scalar dofs
      obj._PrimalVec(i_offset + i) += obj._JxW[qp] * (obj._div_vector_phi[i][qp] * scalar_sol[qp]);
    }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::vectorVolumeJacobian(DiffusionHybridized & obj,
                                                   const unsigned int i_offset,
                                                   const unsigned int vector_j_offset,
                                                   const unsigned int scalar_j_offset)
{
  for (const auto qp : make_range(obj._qrule->n_points()))
    for (const auto i : make_range(obj._vector_n_dofs))
    {
      // Vector equation dependence on vector dofs
      for (const auto j : make_range(obj._vector_n_dofs))
        obj._PrimalMat(i_offset + i, vector_j_offset + j) +=
            obj._JxW[qp] * (obj._vector_phi[i][qp] * obj._vector_phi[j][qp]);

      // Vector equation dependence on scalar dofs
      for (const auto j : make_range(obj._scalar_n_dofs))
        obj._PrimalMat(i_offset + i, scalar_j_offset + j) +=
            obj._JxW[qp] * (obj._div_vector_phi[i][qp] * obj._scalar_phi[j][qp]);
    }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::scalarVolumeResidual(DiffusionHybridized & obj,
                                                   const unsigned int i_offset,
                                                   const MooseArray<Gradient> & vector_field,
                                                   const Function & source)
{
  for (const auto qp : make_range(obj._qrule->n_points()))
  {
    // Evaluate source
    const auto f = source.value(obj._t, obj._q_point[qp]);

    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      obj._PrimalVec(i_offset + i) +=
          obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * obj._diff[qp] * vector_field[qp]);

      // Scalar equation RHS
      obj._PrimalVec(i_offset + i) -= obj._JxW[qp] * obj._scalar_phi[i][qp] * f;
    }
  }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::scalarVolumeJacobian(DiffusionHybridized & obj,
                                                   const unsigned int i_offset,
                                                   const unsigned int vector_field_j_offset)
{
  for (const auto qp : make_range(obj._qrule->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
      // Scalar equation dependence on vector dofs
      for (const auto j : make_range(obj._vector_n_dofs))
        obj._PrimalMat(i_offset + i, vector_field_j_offset + j) +=
            obj._JxW[qp] * obj._diff[qp] * (obj._grad_scalar_phi[i][qp] * obj._vector_phi[j][qp]);
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::vectorFaceResidual(DiffusionHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    // Vector equation dependence on LM dofs
    for (const auto i : make_range(obj._vector_n_dofs))
      obj._PrimalVec(i_offset + i) -=
          obj._JxW_face[qp] * (obj._vector_phi_face[i][qp] * obj._normals[qp]) * lm_sol[qp];
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::vectorFaceJacobian(DiffusionHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const unsigned int lm_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    // Vector equation dependence on LM dofs
    for (const auto i : make_range(obj._vector_n_dofs))
      for (const auto j : make_range(obj._lm_n_dofs))
        obj._PrimalLM(i_offset + i, lm_j_offset + j) -=
            obj._JxW_face[qp] * (obj._vector_phi_face[i][qp] * obj._normals[qp]) *
            obj._lm_phi_face[j][qp];
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::scalarFaceResidual(DiffusionHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const MooseArray<Gradient> & vector_sol,
                                                 const MooseArray<Number> & scalar_sol,
                                                 const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
      if (obj._neigh)
      {
        // vector
        obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._diff[qp] *
                                        obj._scalar_phi_face[i][qp] *
                                        (vector_sol[qp] * obj._normals[qp]);

        // scalar from stabilization term
        obj._PrimalVec(i_offset + i) += obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau *
                                        scalar_sol[qp] * obj._normals[qp] * obj._normals[qp];

        // lm from stabilization term
        obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau *
                                        lm_sol[qp] * obj._normals[qp] * obj._normals[qp];
      }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::scalarFaceJacobian(DiffusionHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const unsigned int vector_j_offset,
                                                 const unsigned int scalar_j_offset,
                                                 const unsigned int lm_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
      if (obj._neigh)
      {
        for (const auto j : make_range(obj._vector_n_dofs))
          obj._PrimalMat(i_offset + i, vector_j_offset + j) -=
              obj._JxW_face[qp] * obj._diff[qp] * obj._scalar_phi_face[i][qp] *
              (obj._vector_phi_face[j][qp] * obj._normals[qp]);

        for (const auto j : make_range(obj._scalar_n_dofs))
          obj._PrimalMat(i_offset + i, scalar_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau * obj._scalar_phi_face[j][qp] *
              obj._normals[qp] * obj._normals[qp];

        for (const auto j : make_range(obj._lm_n_dofs))
          // from stabilization term
          obj._PrimalLM(i_offset + i, lm_j_offset + j) -=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau * obj._lm_phi_face[j][qp] *
              obj._normals[qp] * obj._normals[qp];
      }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::lmFaceResidual(DiffusionHybridized & obj,
                                             const unsigned int i_offset,
                                             const MooseArray<Gradient> & vector_sol,
                                             const MooseArray<Number> & scalar_sol,
                                             const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._lm_n_dofs))
    {
      // vector
      obj._LMVec(i_offset + i) -= obj._JxW_face[qp] * obj._diff[qp] * obj._lm_phi_face[i][qp] *
                                  (vector_sol[qp] * obj._normals[qp]);

      // scalar from stabilization term
      obj._LMVec(i_offset + i) += obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau *
                                  scalar_sol[qp] * obj._normals[qp] * obj._normals[qp];

      // lm from stabilization term
      obj._LMVec(i_offset + i) -= obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau * lm_sol[qp] *
                                  obj._normals[qp] * obj._normals[qp];
    }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedInterface::lmFaceJacobian(DiffusionHybridized & obj,
                                             const unsigned int i_offset,
                                             const unsigned int vector_j_offset,
                                             const unsigned int scalar_j_offset,
                                             const unsigned int lm_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._lm_n_dofs))
    {
      for (const auto j : make_range(obj._vector_n_dofs))
        obj._LMPrimal(i_offset + i, vector_j_offset + j) -=
            obj._JxW_face[qp] * obj._diff[qp] * obj._lm_phi_face[i][qp] *
            (obj._vector_phi_face[j][qp] * obj._normals[qp]);

      for (const auto j : make_range(obj._scalar_n_dofs))
        obj._LMPrimal(i_offset + i, scalar_j_offset + j) +=
            obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau * obj._scalar_phi_face[j][qp] *
            obj._normals[qp] * obj._normals[qp];

      for (const auto j : make_range(obj._lm_n_dofs))
        // from stabilization term
        obj._LMMat(i_offset + i, lm_j_offset + j) -= obj._JxW_face[qp] * obj._lm_phi_face[i][qp] *
                                                     _tau * obj._lm_phi_face[j][qp] *
                                                     obj._normals[qp] * obj._normals[qp];
    }
}
