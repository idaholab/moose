//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiffusionHDGAssemblyHelper.h"

class NavierStokesHDGAssemblyHelper : public DiffusionHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  NavierStokesHDGAssemblyHelper(const MooseObject * moose_obj,
                                MaterialPropertyInterface * mpi,
                                SystemBase & nl_sys,
                                SystemBase & aux_sys,
                                const MooseMesh & mesh,
                                const THREAD_ID tid);

protected:
  /**
   * Set the number of degree of freedom variables and resize the Eigen data structures
   */
  template <typename NSHDG>
  static void resizeData(NSHDG & obj);

  /**
   * @param u_sol The x-velocity solution, can correspond to either the volumetric or face velocity
   * @param v_sol The y-velocity solution, can correspond to either the volumetric or face velocity
   */
  static RealVectorValue velCrossVelResidual(const MooseArray<Number> & u_sol,
                                             const MooseArray<Number> & v_sol,
                                             const unsigned int qp,
                                             const unsigned int vel_component);

  /**
   * @param u_sol The x-velocity solution, can correspond to either the volumetric or face velocity
   * @param v_sol The y-velocity solution, can correspond to either the volumetric or face velocity
   */
  static RealVectorValue velCrossVelJacobian(const MooseArray<Number> & u_sol,
                                             const MooseArray<Number> & v_sol,
                                             const unsigned int qp,
                                             const unsigned int vel_component,
                                             const unsigned int vel_j_component,
                                             const MooseArray<std::vector<Real>> & phi,
                                             const unsigned int j);

  /**
   * Compute the volumetric contributions to a velocity residual for a provided velocity
   * gradient and stress
   * @param i_offset The local degree of freedom offset for the velocity component
   * @param vel_gradient The velocity gradient component
   * @param vel_component The velocity component
   */
  template <typename NSHDG>
  static void scalarVolumeResidual(NSHDG & obj,
                                   const unsigned int i_offset,
                                   const MooseArray<Gradient> & vel_gradient,
                                   const unsigned int vel_component,
                                   const Function & body_force);

  /**
   * Compute the volumetric contributions to a velocity Jacobian
   * @param i_offset The local degree of freedom offset for the velocity component
   * @param vel_gradient_j_offset The local degree of freedom offset for the associated velocity
   * gradient
   * @param p_j_offset The local degree of freedom offset for the pressure
   * @param vel_component The velocity component
   * @param u_j_offset The local degree of freedom offset for the x-component velocity
   * @param v_j_offset The local degree of freedom offset for the y-component velocity
   */
  template <typename NSHDG>
  static void scalarVolumeJacobian(NSHDG & obj,
                                   const unsigned int i_offset,
                                   const unsigned int vel_gradient_j_offset,
                                   const unsigned int p_j_offset,
                                   const unsigned int vel_component,
                                   const unsigned int u_j_offset,
                                   const unsigned int v_j_offset);

  /**
   * Compute the volumetric contributions to the pressure residual, e.g. the conservation of mass
   * equation
   * @param i_offset The local degree of freedom offset for the pressure
   * @param global_lm_i_offset The local degree of freedom offset for the global Lagrange multiplier
   * that removes the pressure nullspace
   */
  template <typename NSHDG>
  static void pressureVolumeResidual(NSHDG & obj,
                                     const unsigned int i_offset,
                                     const unsigned int global_lm_i_offset,
                                     const Function & pressure_mms_forcing_function);

  /**
   * Compute the volumetric contributions to the pressure Jacobian, e.g. the conservation of mass
   * equation
   * @param i_offset The local degree of freedom offset for the pressure
   * @param u_j_offset The local degree of freedom offset for the x-component of velocity
   * @param v_j_offset The local degree of freedom offset for the y-component of velocity
   * @param p_j_offset The local degree of freedom offset for the pressure
   * @param global_lm_i_offset The local degree of freedom offset for the global Lagrange multiplier
   * that removes the pressure nullspace
   */
  template <typename NSHDG>
  static void pressureVolumeJacobian(NSHDG & obj,
                                     const unsigned int i_offset,
                                     const unsigned int u_j_offset,
                                     const unsigned int v_j_offset,
                                     const unsigned int p_j_offset,
                                     const unsigned int global_lm_offset);

  //
  // Methods which are leveraged both on internal sides in the kernel and by the outflow bc
  //

  template <typename NSHDG>
  static void pressureFaceResidual(NSHDG & obj, const unsigned int i_offset);

  template <typename NSHDG>
  static void pressureFaceJacobian(NSHDG & obj,
                                   const unsigned int i_offset,
                                   const unsigned int lm_u_j_offset,
                                   const unsigned int lm_v_j_offset);

  template <typename NSHDG>
  static void scalarFaceResidual(NSHDG & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Gradient> & vector_sol,
                                 const MooseArray<Number> & scalar_sol,
                                 const MooseArray<Number> & lm_sol,
                                 const unsigned int vel_component);

  template <typename NSHDG>
  static void scalarFaceJacobian(NSHDG & obj,
                                 const unsigned int i_offset,
                                 const unsigned int vector_j_offset,
                                 const unsigned int scalar_j_offset,
                                 const unsigned int lm_j_offset,
                                 const unsigned int p_j_offset,
                                 const unsigned int vel_component,
                                 const unsigned int lm_u_j_offset,
                                 const unsigned int lm_v_j_offset);

  template <typename NSHDG>
  static void lmFaceResidual(NSHDG & obj,
                             const unsigned int i_offset,
                             const MooseArray<Gradient> & vector_sol,
                             const MooseArray<Number> & scalar_sol,
                             const MooseArray<Number> & lm_sol,
                             const unsigned int vel_component);

  template <typename NSHDG>
  static void lmFaceJacobian(NSHDG & obj,
                             const unsigned int i_offset,
                             const unsigned int vector_j_offset,
                             const unsigned int scalar_j_offset,
                             const unsigned int lm_j_offset,
                             const unsigned int p_j_offset,
                             const unsigned int vel_component,
                             const unsigned int lm_u_j_offset,
                             const unsigned int lm_v_j_offset);

  template <typename NSHDG>
  static void pressureDirichletResidual(NSHDG & obj,
                                        const unsigned int i_offset,
                                        const std::array<const Function *, 3> & dirichlet_vel);

  template <typename NSHDG>
  static void scalarDirichletResidual(NSHDG & obj,
                                      const unsigned int i_offset,
                                      const MooseArray<Gradient> & vector_sol,
                                      const MooseArray<Number> & scalar_sol,
                                      const unsigned int vel_component,
                                      const std::array<const Function *, 3> & dirichlet_vel);

  template <typename NSHDG>
  static void scalarDirichletJacobian(NSHDG & obj,
                                      const unsigned int i_offset,
                                      const unsigned int vector_j_offset,
                                      const unsigned int scalar_j_offset,
                                      const unsigned int p_j_offset,
                                      const unsigned int vel_component);

  const MooseVariableFE<Real> & _v_var;
  const MooseVariableFE<Real> * const _w_var;
  const MooseVariableFE<RealVectorValue> & _grad_v_var;
  const MooseVariableFE<RealVectorValue> * const _grad_w_var;
  const MooseVariableFE<Real> & _v_face_var;
  const MooseVariableFE<Real> * const _w_face_var;
  const MooseVariableFE<Real> & _pressure_var;
  const MooseVariableScalar * const _enclosure_lm_var;

  /// Containers for dof indices
  const std::vector<dof_id_type> & _qv_dof_indices;
  const std::vector<dof_id_type> & _v_dof_indices;
  const std::vector<dof_id_type> & _lm_v_dof_indices;
  const std::vector<dof_id_type> * const _qw_dof_indices;
  const std::vector<dof_id_type> * const _w_dof_indices;
  const std::vector<dof_id_type> * const _lm_w_dof_indices;
  const std::vector<dof_id_type> & _p_dof_indices;
  const std::vector<dof_id_type> * const _global_lm_dof_indices;

  /// local solutions at quadrature points
  const MooseArray<Gradient> & _qv_sol;
  const MooseArray<Number> & _v_sol;
  const MooseArray<Number> & _lm_v_sol;
  const MooseArray<Gradient> * const _qw_sol;
  const MooseArray<Number> * const _w_sol;
  const MooseArray<Number> * const _lm_w_sol;
  const MooseArray<Number> & _p_sol;
  const MooseArray<Number> * const _global_lm_dof_value;

  /// Number of dofs on elem
  std::size_t _p_n_dofs;
  std::size_t _global_lm_n_dofs;
};

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::resizeData(NSHDG & obj)
{
  obj._vector_n_dofs = obj._qu_dof_indices.size();
  obj._scalar_n_dofs = obj._u_dof_indices.size();
  obj._lm_n_dofs = obj._lm_u_dof_indices.size();
  obj._p_n_dofs = obj._p_dof_indices.size();
  libmesh_assert(obj._p_n_dofs == obj._scalar_n_dofs);

  libmesh_assert_equal_to(obj._vector_n_dofs, obj._vector_phi.size());
  libmesh_assert_equal_to(obj._scalar_n_dofs, obj._scalar_phi.size());

  obj._primal_size = 2 * (obj._vector_n_dofs + obj._scalar_n_dofs);
  obj._lm_size = 2 * obj._lm_n_dofs + obj._p_n_dofs + obj._global_lm_n_dofs;

  // prepare our matrix/vector data structures
  obj._PrimalMat.setZero(obj._primal_size, obj._primal_size);
  obj._PrimalVec.setZero(obj._primal_size);
  obj._LMMat.setZero(obj._lm_size, obj._lm_size);
  obj._LMVec.setZero(obj._lm_size);
  obj._PrimalLM.setZero(obj._primal_size, obj._lm_size);
  obj._LMPrimal.setZero(obj._lm_size, obj._primal_size);
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::scalarVolumeResidual(NSHDG & obj,
                                                    const unsigned int i_offset,
                                                    const MooseArray<Gradient> & vel_gradient,
                                                    const unsigned int vel_component,
                                                    const Function & body_force)
{
  DiffusionHDGAssemblyHelper::scalarVolumeResidual(obj, i_offset, vel_gradient, body_force);

  for (const auto qp : make_range(obj._qrule->n_points()))
  {
    const auto vel_cross_vel = velCrossVelResidual(obj._u_sol, obj._v_sol, qp, vel_component);
    Gradient qp_p;
    qp_p(vel_component) = obj._p_sol[qp];

    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      // Scalar equation dependence on pressure dofs
      obj._PrimalVec(i_offset + i) -= obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * qp_p);

      // Scalar equation dependence on scalar dofs
      obj._PrimalVec(i_offset + i) -= obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * vel_cross_vel);
    }
  }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::scalarVolumeJacobian(NSHDG & obj,
                                                    const unsigned int i_offset,
                                                    const unsigned int vel_gradient_j_offset,
                                                    const unsigned int p_j_offset,
                                                    const unsigned int vel_component,
                                                    const unsigned int u_j_offset,
                                                    const unsigned int v_j_offset)
{
  DiffusionHDGAssemblyHelper::scalarVolumeJacobian(obj, i_offset, vel_gradient_j_offset);

  for (const auto qp : make_range(obj._qrule->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      // Scalar equation dependence on pressure dofs
      for (const auto j : make_range(obj._p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = obj._scalar_phi[j][qp];
        obj._PrimalLM(i_offset + i, p_j_offset + j) -=
            obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * p_phi);
      }

      // Scalar equation dependence on scalar dofs
      for (const auto j : make_range(obj._scalar_n_dofs))
      {
        // derivatives wrt 0th component
        {
          const auto vel_cross_vel =
              velCrossVelJacobian(obj._u_sol, obj._v_sol, qp, vel_component, 0, obj._scalar_phi, j);
          obj._PrimalMat(i_offset + i, u_j_offset + j) -=
              obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * vel_cross_vel);
        }
        // derivatives wrt 1th component
        {
          const auto vel_cross_vel =
              velCrossVelJacobian(obj._u_sol, obj._v_sol, qp, vel_component, 1, obj._scalar_phi, j);
          obj._PrimalMat(i_offset + i, v_j_offset + j) -=
              obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * vel_cross_vel);
        }
      }
    }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::pressureVolumeResidual(
    NSHDG & obj,
    const unsigned int i_offset,
    const unsigned int global_lm_i_offset,
    const Function & pressure_mms_forcing_function)
{
  for (const auto qp : make_range(obj._qrule->n_points()))
  {
    // Prepare forcing function
    const auto f = pressure_mms_forcing_function.value(obj._t, obj._q_point[qp]);

    const Gradient vel(obj._u_sol[qp], obj._v_sol[qp]);
    for (const auto i : make_range(obj._p_n_dofs))
    {
      obj._LMVec(i_offset + i) -= obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * vel);

      // Pressure equation forcing function RHS
      obj._LMVec(i_offset + i) -= obj._JxW[qp] * obj._scalar_phi[i][qp] * f;

      if (obj._enclosure_lm_var)
      {
        mooseAssert(
            obj._global_lm_dof_value->size() == 1,
            "There should only be one degree of freedom for removing the pressure nullspace");
        obj._LMVec(i_offset + i) -=
            obj._JxW[qp] * obj._scalar_phi[i][qp] * (*obj._global_lm_dof_value)[0];
      }
    }

    if (obj._enclosure_lm_var)
      obj._LMVec(global_lm_i_offset) -= obj._JxW[qp] * obj._p_sol[qp];
  }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::pressureVolumeJacobian(NSHDG & obj,
                                                      const unsigned int i_offset,
                                                      const unsigned int u_j_offset,
                                                      const unsigned int v_j_offset,
                                                      const unsigned int p_j_offset,
                                                      const unsigned int global_lm_offset)
{
  for (const auto qp : make_range(obj._qrule->n_points()))
  {
    for (const auto i : make_range(obj._p_n_dofs))
    {
      for (const auto j : make_range(obj._scalar_n_dofs))
      {
        {
          const Gradient phi(obj._scalar_phi[j][qp], 0);
          obj._LMPrimal(i_offset + i, u_j_offset + j) -=
              obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * phi);
        }
        {
          const Gradient phi(0, obj._scalar_phi[j][qp]);
          obj._LMPrimal(i_offset + i, v_j_offset + j) -=
              obj._JxW[qp] * (obj._grad_scalar_phi[i][qp] * phi);
        }
      }
      if (obj._enclosure_lm_var)
        obj._LMMat(i_offset + i, global_lm_offset) -= obj._JxW[qp] * obj._scalar_phi[i][qp];
    }

    if (obj._enclosure_lm_var)
    {
      libmesh_assert(obj._scalar_n_dofs == obj._p_n_dofs);
      for (const auto j : make_range(obj._p_n_dofs))
        obj._LMMat(global_lm_offset, p_j_offset + j) -= obj._JxW[qp] * obj._scalar_phi[j][qp];
    }
  }
}

inline RealVectorValue
NavierStokesHDGAssemblyHelper::velCrossVelResidual(const MooseArray<Number> & u_sol,
                                                   const MooseArray<Number> & v_sol,
                                                   const unsigned int qp,
                                                   const unsigned int vel_component)
{
  const RealVectorValue U(u_sol[qp], v_sol[qp]);
  return U * U(vel_component);
}

inline RealVectorValue
NavierStokesHDGAssemblyHelper::velCrossVelJacobian(const MooseArray<Number> & u_sol,
                                                   const MooseArray<Number> & v_sol,
                                                   const unsigned int qp,
                                                   const unsigned int vel_component,
                                                   const unsigned int vel_j_component,
                                                   const MooseArray<std::vector<Real>> & phi,
                                                   const unsigned int j)
{
  const RealVectorValue U(u_sol[qp], v_sol[qp]);
  RealVectorValue vector_phi;
  vector_phi(vel_j_component) = phi[j][qp];
  auto ret = vector_phi * U(vel_component);
  if (vel_component == vel_j_component)
    ret += U * phi[j][qp];
  return ret;
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::pressureFaceResidual(NSHDG & obj, const unsigned int i_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    const Gradient vel(obj._lm_u_sol[qp], obj._lm_v_sol[qp]);
    const auto vdotn = vel * obj._normals[qp];
    for (const auto i : make_range(obj._p_n_dofs))
      obj._LMVec(i_offset + i) += obj._JxW_face[qp] * vdotn * obj._scalar_phi_face[i][qp];
  }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::pressureFaceJacobian(NSHDG & obj,
                                                    const unsigned int i_offset,
                                                    const unsigned int lm_u_j_offset,
                                                    const unsigned int lm_v_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._p_n_dofs))
      for (const auto j : make_range(obj._lm_n_dofs))
      {
        {
          const Gradient phi(obj._lm_phi_face[j][qp], 0);
          obj._LMMat(i_offset + i, lm_u_j_offset + j) +=
              obj._JxW_face[qp] * phi * obj._normals[qp] * obj._scalar_phi_face[i][qp];
        }
        {
          const Gradient phi(0, obj._lm_phi_face[j][qp]);
          obj._LMMat(i_offset + i, lm_v_j_offset + j) +=
              obj._JxW_face[qp] * phi * obj._normals[qp] * obj._scalar_phi_face[i][qp];
        }
      }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::scalarFaceResidual(NSHDG & obj,
                                                  const unsigned int i_offset,
                                                  const MooseArray<Gradient> & vector_sol,
                                                  const MooseArray<Number> & scalar_sol,
                                                  const MooseArray<Number> & lm_sol,
                                                  const unsigned int vel_component)
{
  DiffusionHDGAssemblyHelper::scalarFaceResidual(obj, i_offset, vector_sol, scalar_sol, lm_sol);

  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = obj._p_sol[qp];
    const auto vel_cross_vel = velCrossVelResidual(obj._lm_u_sol, obj._lm_v_sol, qp, vel_component);

    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      if (obj._neigh)
        // pressure
        obj._PrimalVec(i_offset + i) +=
            obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * (qp_p * obj._normals[qp]);

      // lm from convection term
      obj._PrimalVec(i_offset + i) +=
          obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
    }
  }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::scalarFaceJacobian(NSHDG & obj,
                                                  const unsigned int i_offset,
                                                  const unsigned int vector_j_offset,
                                                  const unsigned int scalar_j_offset,
                                                  const unsigned int lm_j_offset,
                                                  const unsigned int p_j_offset,
                                                  const unsigned int vel_component,
                                                  const unsigned int lm_u_j_offset,
                                                  const unsigned int lm_v_j_offset)
{
  DiffusionHDGAssemblyHelper::scalarFaceJacobian(
      obj, i_offset, vector_j_offset, scalar_j_offset, lm_j_offset);

  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      if (obj._neigh)
        for (const auto j : make_range(obj._p_n_dofs))
        {
          Gradient p_phi;
          p_phi(vel_component) = obj._scalar_phi_face[j][qp];
          // pressure
          obj._PrimalLM(i_offset + i, p_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * (p_phi * obj._normals[qp]);
        }

      for (const auto j : make_range(obj._lm_n_dofs))
      {
        //
        // from convection term
        //

        // derivatives wrt 0th component
        {
          const auto vel_cross_vel = velCrossVelJacobian(
              obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 0, obj._lm_phi_face, j);
          obj._PrimalLM(i_offset + i, lm_u_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
        }
        // derivatives wrt 1th component
        {
          const auto vel_cross_vel = velCrossVelJacobian(
              obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 1, obj._lm_phi_face, j);
          obj._PrimalLM(i_offset + i, lm_v_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
        }
      }
    }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::lmFaceResidual(NSHDG & obj,
                                              const unsigned int i_offset,
                                              const MooseArray<Gradient> & vector_sol,
                                              const MooseArray<Number> & scalar_sol,
                                              const MooseArray<Number> & lm_sol,
                                              const unsigned int vel_component)
{
  DiffusionHDGAssemblyHelper::lmFaceResidual(obj, i_offset, vector_sol, scalar_sol, lm_sol);

  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = obj._p_sol[qp];
    const auto vel_cross_vel = velCrossVelResidual(obj._lm_u_sol, obj._lm_v_sol, qp, vel_component);

    for (const auto i : make_range(obj._lm_n_dofs))
    {
      // pressure
      obj._LMVec(i_offset + i) +=
          obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * (qp_p * obj._normals[qp]);

      // If we are an internal face we add the convective term. On the outflow boundary we do not
      // zero out the convection term, e.g. we are going to set q + p + tau * (u - u_hat) to zero
      if (obj._neigh)
        // lm from convection term
        obj._LMVec(i_offset + i) +=
            obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
    }
  }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::lmFaceJacobian(NSHDG & obj,
                                              const unsigned int i_offset,
                                              const unsigned int vector_j_offset,
                                              const unsigned int scalar_j_offset,
                                              const unsigned int lm_j_offset,
                                              const unsigned int p_j_offset,
                                              const unsigned int vel_component,
                                              const unsigned int lm_u_j_offset,
                                              const unsigned int lm_v_j_offset)
{
  DiffusionHDGAssemblyHelper::lmFaceJacobian(
      obj, i_offset, vector_j_offset, scalar_j_offset, lm_j_offset);

  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._lm_n_dofs))
    {
      for (const auto j : make_range(obj._p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = obj._scalar_phi_face[j][qp];
        obj._LMMat(i_offset + i, p_j_offset + j) +=
            obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * (p_phi * obj._normals[qp]);
      }

      for (const auto j : make_range(obj._lm_n_dofs))
        if (obj._neigh)
        {
          // derivatives wrt 0th component
          {
            const auto vel_cross_vel = velCrossVelJacobian(
                obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 0, obj._lm_phi_face, j);
            obj._LMMat(i_offset + i, lm_u_j_offset + j) +=
                obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
          }
          // derivatives wrt 1th component
          {
            const auto vel_cross_vel = velCrossVelJacobian(
                obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 1, obj._lm_phi_face, j);
            obj._LMMat(i_offset + i, lm_v_j_offset + j) +=
                obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
          }
        }
    }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::pressureDirichletResidual(
    NSHDG & obj, const unsigned int i_offset, const std::array<const Function *, 3> & dirichlet_vel)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    const RealVectorValue dirichlet_velocity(
        dirichlet_vel[0]->value(obj._t, obj._q_point_face[qp]),
        dirichlet_vel[1]->value(obj._t, obj._q_point_face[qp]),
        dirichlet_vel[2]->value(obj._t, obj._q_point_face[qp]));
    const auto vdotn = dirichlet_velocity * obj._normals[qp];
    for (const auto i : make_range(obj._p_n_dofs))
      obj._LMVec(i_offset + i) += obj._JxW_face[qp] * vdotn * obj._scalar_phi_face[i][qp];
  }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::scalarDirichletResidual(
    NSHDG & obj,
    const unsigned int i_offset,
    const MooseArray<Gradient> & vector_sol,
    const MooseArray<Number> & scalar_sol,
    const unsigned int vel_component,
    const std::array<const Function *, 3> & dirichlet_vel)
{
  DiffusionHDGAssemblyHelper::scalarDirichletResidual(
      obj, i_offset, vector_sol, scalar_sol, *dirichlet_vel[vel_component]);

  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = obj._p_sol[qp];

    const RealVectorValue dirichlet_velocity(
        dirichlet_vel[0]->value(obj._t, obj._q_point_face[qp]),
        dirichlet_vel[1]->value(obj._t, obj._q_point_face[qp]),
        dirichlet_vel[2]->value(obj._t, obj._q_point_face[qp]));
    const auto scalar_value = dirichlet_velocity(vel_component);

    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      // pressure
      obj._PrimalVec(i_offset + i) +=
          obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * (qp_p * obj._normals[qp]);

      // dirichlet lm from advection term
      obj._PrimalVec(i_offset + i) += obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] *
                                      (dirichlet_velocity * obj._normals[qp]) * scalar_value;
    }
  }
}

template <typename NSHDG>
void
NavierStokesHDGAssemblyHelper::scalarDirichletJacobian(NSHDG & obj,
                                                       const unsigned int i_offset,
                                                       const unsigned int vector_j_offset,
                                                       const unsigned int scalar_j_offset,
                                                       const unsigned int p_j_offset,
                                                       const unsigned int vel_component)
{
  DiffusionHDGAssemblyHelper::scalarDirichletJacobian(
      obj, i_offset, vector_j_offset, scalar_j_offset);

  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
      for (const auto j : make_range(obj._p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = obj._scalar_phi_face[j][qp];
        // pressure
        obj._PrimalLM(i_offset + i, p_j_offset + j) +=
            obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * (p_phi * obj._normals[qp]);
      }
}
