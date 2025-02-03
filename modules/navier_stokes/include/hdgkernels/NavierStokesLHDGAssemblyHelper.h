//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiffusionLHDGAssemblyHelper.h"

/**
 * Implements all the methods for assembling a hybridized local discontinuous Galerkin (LDG-H),
 * which is a type of HDG method, discretization of the incompressible Navier-Stokes equations.
 * These routines may be called by both HDG kernels and integrated boundary conditions. The
 * implementation here is based on "An implicit high-order hybridizable discontinuous Galerkin
 * method for the incompressible Navier-Stokes equations" by Nguyen and Cockburn
 */
class NavierStokesLHDGAssemblyHelper : public DiffusionLHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  NavierStokesLHDGAssemblyHelper(const MooseObject * moose_obj,
                                MaterialPropertyInterface * mpi,
                                MooseVariableDependencyInterface * mvdi,
                                const TransientInterface * const ti,
                                const FEProblemBase & fe_problem,
                                SystemBase & sys,
                                const MooseMesh & mesh,
                                const THREAD_ID tid);

protected:
  /**
   * @param u_sol The x-velocity solution, can correspond to either the volumetric or face velocity
   * @param v_sol The y-velocity solution, can correspond to either the volumetric or face velocity
   */
  RealVectorValue rhoVelCrossVelResidual(const MooseArray<Number> & u_sol,
                                         const MooseArray<Number> & v_sol,
                                         const unsigned int qp,
                                         const unsigned int vel_component);

  /**
   * @param u_sol The x-velocity solution, can correspond to either the volumetric or face velocity
   * @param v_sol The y-velocity solution, can correspond to either the volumetric or face velocity
   */
  RealVectorValue rhoVelCrossVelJacobian(const MooseArray<Number> & u_sol,
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
  void scalarVolumeResidual(const MooseArray<Gradient> & vel_gradient,
                            const unsigned int vel_component,
                            const Moose::Functor<Real> & body_force,
                            const MooseArray<Real> & JxW,
                            const libMesh::QBase & qrule,
                            const Elem * const current_elem,
                            const MooseArray<Point> & q_point,
                            DenseVector<Number> & scalar_re);

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
  void scalarVolumeJacobian(const unsigned int vel_component,
                            const MooseArray<Real> & JxW,
                            const libMesh::QBase & qrule,
                            DenseMatrix<Number> & scalar_vector_jac,
                            DenseMatrix<Number> & scalar_u_vel_jac,
                            DenseMatrix<Number> & scalar_v_vel_jac,
                            DenseMatrix<Number> & scalar_p_jac);

  /**
   * Compute the volumetric contributions to the pressure residual, e.g. the conservation of
   * mass equation
   * @param i_offset The local degree of freedom offset for the pressure
   * @param global_lm_i_offset The local degree of freedom offset for the global Lagrange
   * multiplier that removes the pressure nullspace
   */
  void pressureVolumeResidual(const Moose::Functor<Real> & pressure_mms_forcing_function,
                              const MooseArray<Real> & JxW,
                              const libMesh::QBase & qrule,
                              const Elem * const current_elem,
                              const MooseArray<Point> & q_point,
                              DenseVector<Number> & pressure_re,
                              DenseVector<Number> & global_lm_re);

  /**
   * Compute the volumetric contributions to the pressure Jacobian, e.g. the conservation of
   * mass equation
   * @param i_offset The local degree of freedom offset for the pressure
   * @param u_j_offset The local degree of freedom offset for the x-component of velocity
   * @param v_j_offset The local degree of freedom offset for the y-component of velocity
   * @param p_j_offset The local degree of freedom offset for the pressure
   * @param global_lm_i_offset The local degree of freedom offset for the global Lagrange
   * multiplier that removes the pressure nullspace
   */
  void pressureVolumeJacobian(const MooseArray<Real> & JxW,
                              const libMesh::QBase & qrule,
                              DenseMatrix<Number> & p_u_vel_jac,
                              DenseMatrix<Number> & p_v_vel_jac,
                              DenseMatrix<Number> & p_global_lm_jac,
                              DenseMatrix<Number> & global_lm_p_jac);

  //
  // Methods which are leveraged both on internal sides in the kernel and by the outflow bc
  //

  void pressureFaceResidual(const MooseArray<Real> & JxW_face,
                            const libMesh::QBase & qrule_face,
                            const MooseArray<Point> & normals,
                            DenseVector<Number> & pressure_re);

  void pressureFaceJacobian(const MooseArray<Real> & JxW_face,
                            const libMesh::QBase & qrule_face,
                            const MooseArray<Point> & normals,
                            DenseMatrix<Number> & p_lm_u_vel_jac,
                            DenseMatrix<Number> & p_lm_v_vel_jac);

  void scalarFaceResidual(const MooseArray<Gradient> & vector_sol,
                          const MooseArray<Number> & scalar_sol,
                          const MooseArray<Number> & lm_sol,
                          const unsigned int vel_component,
                          const MooseArray<Real> & JxW_face,
                          const libMesh::QBase & qrule_face,
                          const MooseArray<Point> & normals,
                          DenseVector<Number> & scalar_re);

  void scalarFaceJacobian(const unsigned int vel_component,
                          const MooseArray<Real> & JxW_face,
                          const libMesh::QBase & qrule_face,
                          const MooseArray<Point> & normals,
                          DenseMatrix<Number> & scalar_vector_jac,
                          DenseMatrix<Number> & scalar_scalar_jac,
                          DenseMatrix<Number> & scalar_lm_jac,
                          DenseMatrix<Number> & scalar_p_jac,
                          DenseMatrix<Number> & scalar_lm_u_vel_jac,
                          DenseMatrix<Number> & scalar_lm_v_vel_jac);

  void lmFaceResidual(const MooseArray<Gradient> & vector_sol,
                      const MooseArray<Number> & scalar_sol,
                      const MooseArray<Number> & lm_sol,
                      const unsigned int vel_component,
                      const MooseArray<Real> & JxW_face,
                      const libMesh::QBase & qrule_face,
                      const MooseArray<Point> & normals,
                      const Elem * const neigh,
                      DenseVector<Number> & lm_re);

  void lmFaceJacobian(const unsigned int vel_component,
                      const MooseArray<Real> & JxW_face,
                      const libMesh::QBase & qrule_face,
                      const MooseArray<Point> & normals,
                      const Elem * const neigh,
                      DenseMatrix<Number> & lm_vec_jac,
                      DenseMatrix<Number> & lm_scalar_jac,
                      DenseMatrix<Number> & lm_lm_jac,
                      DenseMatrix<Number> & lm_p_jac,
                      DenseMatrix<Number> & lm_lm_u_vel_jac,
                      DenseMatrix<Number> & lm_lm_v_vel_jac);

  void pressureDirichletResidual(const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
                                 const MooseArray<Real> & JxW_face,
                                 const libMesh::QBase & qrule_face,
                                 const MooseArray<Point> & normals,
                                 const Elem * const current_elem,
                                 const unsigned int current_side,
                                 const MooseArray<Point> & q_point_face,
                                 DenseVector<Number> & pressure_re);

  void scalarDirichletResidual(const MooseArray<Gradient> & vector_sol,
                               const MooseArray<Number> & scalar_sol,
                               const unsigned int vel_component,
                               const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
                               const MooseArray<Real> & JxW_face,
                               const libMesh::QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face,
                               DenseVector<Number> & scalar_re);

  void scalarDirichletJacobian(const unsigned int vel_component,
                               const MooseArray<Real> & JxW_face,
                               const libMesh::QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               DenseMatrix<Number> & scalar_vector_jac,
                               DenseMatrix<Number> & scalar_scalar_jac,
                               DenseMatrix<Number> & scalar_pressure_jac);

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

  /// The density
  const Real _rho;

  DenseVector<Number> _grad_u_vel_re, _grad_v_vel_re, _u_vel_re, _v_vel_re, _lm_u_vel_re,
      _lm_v_vel_re, _p_re, _global_lm_re;
  DenseMatrix<Number> _grad_u_grad_u_jac, _grad_u_u_jac, _grad_v_grad_v_jac, _grad_v_v_jac,
      _u_grad_u_jac, _v_grad_v_jac, _u_u_jac, _u_v_jac, _v_u_jac, _v_v_jac, _u_p_jac, _v_p_jac,
      _p_u_jac, _p_v_jac, _p_global_lm_jac, _global_lm_p_jac, _grad_u_lm_u_jac, _grad_v_lm_v_jac,
      _u_lm_u_jac, _v_lm_v_jac, _u_lm_v_jac, _v_lm_u_jac, _lm_u_grad_u_jac, _lm_v_grad_v_jac,
      _lm_u_u_jac, _lm_v_v_jac, _lm_u_lm_u_jac, _lm_v_lm_v_jac, _lm_u_p_jac, _lm_v_p_jac,
      _lm_u_lm_v_jac, _lm_v_lm_u_jac, _p_lm_u_jac, _p_lm_v_jac;
};
