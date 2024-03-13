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
#include "MooseFunctor.h"
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
class HDGData;
class MooseObject;
class MaterialPropertyInterface;

/**
 * Implements all the methods for assembling a hybridized local discontinuous Galerkin (LDG-H),
 * which is a type of HDG method, discretization of the diffusion equation. These routines may be
 * called by both HDG kernels and integrated boundary conditions. The implementation here is based
 * (but not exactly based) on "A superconvergent LDG-hybridizable Galerkin method for second-order
 * elliptic problems" by Cockburn
 */
class DiffusionHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  DiffusionHDGAssemblyHelper(const MooseObject * const moose_obj,
                             MaterialPropertyInterface * const mpi,
                             SystemBase & nl_sys,
                             SystemBase & aux_sys,
                             const THREAD_ID tid);

protected:
  /**
   * Set the number of degree of freedom variables and resize the Eigen data structures
   */
  template <typename DiffusionHDG>
  static void resizeData(DiffusionHDG & obj);

  /**
   * Implements a residual for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient, v are its associated test functions, and
   * u is the scalar field
   */
  template <typename DiffusionHDG>
  static void vectorVolumeResidual(DiffusionHDG & obj,
                                   const unsigned int i_offset,
                                   const MooseArray<Gradient> & vector_sol,
                                   const MooseArray<Number> & scalar_sol);

  /**
   * Implements a Jacobian for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient, v are its associated test functions, and
   * u is the scalar field
   */
  template <typename DiffusionHDG>
  static void vectorVolumeJacobian(DiffusionHDG & obj,
                                   const unsigned int i_offset,
                                   const unsigned int vector_j_offset,
                                   const unsigned int scalar_j_offset);

  /**
   * Implements a residual for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  template <typename DiffusionHDG>
  static void scalarVolumeResidual(DiffusionHDG & obj,
                                   const unsigned int i_offset,
                                   const MooseArray<Gradient> & vector_field,
                                   const Function & source);

  /**
   * Implements a Jacobian for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  template <typename DiffusionHDG>
  static void scalarVolumeJacobian(DiffusionHDG & obj,
                                   const unsigned int i_offset,
                                   const unsigned int vector_field_j_offset);

  //
  // Methods which can be leveraged both on internal sides in the kernel and by boundary conditions
  //

  /**
   * Implements a residual for the weak form:
   * -<\hat{u}, n*v>
   * where \hat{u} is the trace of the scalar field, n is the normal vector, and v are the test
   * functions associated with the gradient field
   */
  template <typename DiffusionHDG>
  static void vectorFaceResidual(DiffusionHDG & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Number> & lm_sol);

  /**
   * Implements a Jacobian for the weak form:
   * -<\hat{u}, n*v>
   * where \hat{u} is the trace of the scalar field, n is the normal vector, and v are the test
   * functions associated with the gradient field
   */
  template <typename DiffusionHDG>
  static void vectorFaceJacobian(DiffusionHDG & obj,
                                 const unsigned int i_offset,
                                 const unsigned int lm_j_offset);

  /**
   * Implements a residual for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  template <typename DiffusionHDG>
  static void scalarFaceResidual(DiffusionHDG & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Gradient> & vector_sol,
                                 const MooseArray<Number> & scalar_sol,
                                 const MooseArray<Number> & lm_sol);

  /**
   * Implements a Jacobian for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  template <typename DiffusionHDG>
  static void scalarFaceJacobian(DiffusionHDG & obj,
                                 const unsigned int i_offset,
                                 const unsigned int vector_j_offset,
                                 const unsigned int scalar_j_offset,
                                 const unsigned int lm_j_offset);

  /**
   * Implements a residual for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  template <typename DiffusionHDG>
  static void lmFaceResidual(DiffusionHDG & obj,
                             const unsigned int i_offset,
                             const MooseArray<Gradient> & vector_sol,
                             const MooseArray<Number> & scalar_sol,
                             const MooseArray<Number> & lm_sol);

  /**
   * Implements a Jacobian for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  template <typename DiffusionHDG>
  static void lmFaceJacobian(DiffusionHDG & obj,
                             const unsigned int i_offset,
                             const unsigned int vector_j_offset,
                             const unsigned int scalar_j_offset,
                             const unsigned int lm_j_offset);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the vector (gradient) equation
   */
  template <typename DiffusionHDG>
  static void vectorDirichletResidual(DiffusionHDG & obj,
                                      const unsigned int i_offset,
                                      const Moose::Functor<Real> & dirichlet_value);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the scalar field equation
   */
  template <typename DiffusionHDG>
  static void scalarDirichletResidual(DiffusionHDG & obj,
                                      const unsigned int i_offset,
                                      const MooseArray<Gradient> & vector_sol,
                                      const MooseArray<Number> & scalar_sol,
                                      const Moose::Functor<Real> & dirichlet_value);

  /**
   * Computes the Jacobian for a Dirichlet condition for the scalar field in the scalar field
   * equation
   */
  template <typename DiffusionHDG>
  static void scalarDirichletJacobian(DiffusionHDG & obj,
                                      const unsigned int i_offset,
                                      const unsigned int vector_j_offset,
                                      const unsigned int scalar_j_offset);

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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::resizeData(DiffusionHDG & obj)
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::vectorVolumeResidual(DiffusionHDG & obj,
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::vectorVolumeJacobian(DiffusionHDG & obj,
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::scalarVolumeResidual(DiffusionHDG & obj,
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::scalarVolumeJacobian(DiffusionHDG & obj,
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::vectorFaceResidual(DiffusionHDG & obj,
                                               const unsigned int i_offset,
                                               const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    // Vector equation dependence on LM dofs
    for (const auto i : make_range(obj._vector_n_dofs))
      obj._PrimalVec(i_offset + i) -=
          obj._JxW_face[qp] * (obj._vector_phi_face[i][qp] * obj._normals[qp]) * lm_sol[qp];
}

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::vectorFaceJacobian(DiffusionHDG & obj,
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::scalarFaceResidual(DiffusionHDG & obj,
                                               const unsigned int i_offset,
                                               const MooseArray<Gradient> & vector_sol,
                                               const MooseArray<Number> & scalar_sol,
                                               const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::scalarFaceJacobian(DiffusionHDG & obj,
                                               const unsigned int i_offset,
                                               const unsigned int vector_j_offset,
                                               const unsigned int scalar_j_offset,
                                               const unsigned int lm_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::lmFaceResidual(DiffusionHDG & obj,
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::lmFaceJacobian(DiffusionHDG & obj,
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

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::vectorDirichletResidual(DiffusionHDG & obj,
                                                    const unsigned int i_offset,
                                                    const Moose::Functor<Real> & dirichlet_value)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{
            obj._current_elem, obj._current_side, qp, obj._qrule_face, obj._q_point_face[qp]},
        obj.determineState());

    // External boundary -> Dirichlet faces -> Vector equation RHS
    for (const auto i : make_range(obj._vector_n_dofs))
      obj._PrimalVec(i_offset + i) -=
          obj._JxW_face[qp] * (obj._vector_phi_face[i][qp] * obj._normals[qp]) * scalar_value;
  }
}

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::scalarDirichletResidual(DiffusionHDG & obj,
                                                    const unsigned int i_offset,
                                                    const MooseArray<Gradient> & vector_sol,
                                                    const MooseArray<Number> & scalar_sol,
                                                    const Moose::Functor<Real> & dirichlet_value)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    const auto scalar_value = dirichlet_value(
        Moose::ElemSideQpArg{
            obj._current_elem, obj._current_side, qp, obj._qrule_face, obj._q_point_face[qp]},
        obj.determineState());

    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      // vector
      obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._diff[qp] *
                                      obj._scalar_phi_face[i][qp] *
                                      (vector_sol[qp] * obj._normals[qp]);

      // scalar from stabilization term
      obj._PrimalVec(i_offset + i) += obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * obj._tau *
                                      scalar_sol[qp] * obj._normals[qp] * obj._normals[qp];

      // dirichlet lm from stabilization term
      obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * obj._tau *
                                      scalar_value * obj._normals[qp] * obj._normals[qp];
    }
  }
}

template <typename DiffusionHDG>
void
DiffusionHDGAssemblyHelper::scalarDirichletJacobian(DiffusionHDG & obj,
                                                    const unsigned int i_offset,
                                                    const unsigned int vector_j_offset,
                                                    const unsigned int scalar_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      for (const auto j : make_range(obj._vector_n_dofs))
        obj._PrimalMat(i_offset + i, vector_j_offset + j) -=
            obj._JxW_face[qp] * obj._diff[qp] * obj._scalar_phi_face[i][qp] *
            (obj._vector_phi_face[j][qp] * obj._normals[qp]);

      for (const auto j : make_range(obj._scalar_n_dofs))
        obj._PrimalMat(i_offset + i, scalar_j_offset + j) +=
            obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * obj._tau *
            obj._scalar_phi_face[j][qp] * obj._normals[qp] * obj._normals[qp];
    }
}
