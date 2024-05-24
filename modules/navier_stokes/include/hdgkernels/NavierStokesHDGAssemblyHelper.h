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

/**
 * Implements all the methods for assembling a hybridized local discontinuous Galerkin (LDG-H),
 * which is a type of HDG method, discretization of the incompressible Navier-Stokes equations.
 * These routines may be called by both HDG kernels and integrated boundary conditions. The
 * implementation here is based on "An implicit high-order hybridizable discontinuous Galerkin
 * method for the incompressible Navier-Stokes equations" by Nguyen and Cockburn
 */
class NavierStokesHDGAssemblyHelper : public DiffusionHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  NavierStokesHDGAssemblyHelper(const MooseObject * moose_obj,
                                MaterialPropertyInterface * mpi,
                                const TransientInterface * const ti,
                                SystemBase & nl_sys,
                                SystemBase & aux_sys,
                                const MooseMesh & mesh,
                                const THREAD_ID tid);

protected:
  virtual std::string physics() const override { return "incompressible Navier-Stokes"; }
  virtual std::set<const MooseVariableBase *> variables() const override;

  /**
   * Set the number of degree of freedom variables and resize the Eigen data structures
   */
  void resizeData();

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
  void scalarVolumeResidual(const unsigned int i_offset,
                            const MooseArray<Gradient> & vel_gradient,
                            const unsigned int vel_component,
                            const Moose::Functor<Real> & body_force,
                            const MooseArray<Real> & JxW,
                            const QBase & qrule,
                            const Elem * const current_elem,
                            const MooseArray<Point> & q_point);

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
  void scalarVolumeJacobian(const unsigned int i_offset,
                            const unsigned int vel_gradient_j_offset,
                            const unsigned int p_j_offset,
                            const unsigned int vel_component,
                            const unsigned int u_j_offset,
                            const unsigned int v_j_offset,
                            const MooseArray<Real> & JxW,
                            const QBase & qrule);

  /**
   * Compute the volumetric contributions to the pressure residual, e.g. the conservation of mass
   * equation
   * @param i_offset The local degree of freedom offset for the pressure
   * @param global_lm_i_offset The local degree of freedom offset for the global Lagrange multiplier
   * that removes the pressure nullspace
   */
  void pressureVolumeResidual(const unsigned int i_offset,
                              const unsigned int global_lm_i_offset,
                              const Moose::Functor<Real> & pressure_mms_forcing_function,
                              const MooseArray<Real> & JxW,
                              const QBase & qrule,
                              const Elem * const current_elem,
                              const MooseArray<Point> & q_point);

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
  void pressureVolumeJacobian(const unsigned int i_offset,
                              const unsigned int u_j_offset,
                              const unsigned int v_j_offset,
                              const unsigned int p_j_offset,
                              const unsigned int global_lm_offset,
                              const MooseArray<Real> & JxW,
                              const QBase & qrule);

  //
  // Methods which are leveraged both on internal sides in the kernel and by the outflow bc
  //

  void pressureFaceResidual(const unsigned int i_offset,
                            const MooseArray<Real> & JxW_face,
                            const QBase & qrule_face,
                            const MooseArray<Point> & normals);

  void pressureFaceJacobian(const unsigned int i_offset,
                            const unsigned int lm_u_j_offset,
                            const unsigned int lm_v_j_offset,
                            const MooseArray<Real> & JxW_face,
                            const QBase & qrule_face,
                            const MooseArray<Point> & normals);

  void scalarFaceResidual(const unsigned int i_offset,
                          const MooseArray<Gradient> & vector_sol,
                          const MooseArray<Number> & scalar_sol,
                          const MooseArray<Number> & lm_sol,
                          const unsigned int vel_component,
                          const MooseArray<Real> & JxW_face,
                          const QBase & qrule_face,
                          const MooseArray<Point> & normals);

  void scalarFaceJacobian(const unsigned int i_offset,
                          const unsigned int vector_j_offset,
                          const unsigned int scalar_j_offset,
                          const unsigned int lm_j_offset,
                          const unsigned int p_j_offset,
                          const unsigned int vel_component,
                          const unsigned int lm_u_j_offset,
                          const unsigned int lm_v_j_offset,
                          const MooseArray<Real> & JxW_face,
                          const QBase & qrule_face,
                          const MooseArray<Point> & normals);

  void lmFaceResidual(const unsigned int i_offset,
                      const MooseArray<Gradient> & vector_sol,
                      const MooseArray<Number> & scalar_sol,
                      const MooseArray<Number> & lm_sol,
                      const unsigned int vel_component,
                      const MooseArray<Real> & JxW_face,
                      const QBase & qrule_face,
                      const MooseArray<Point> & normals,
                      const Elem * const neigh);

  void lmFaceJacobian(const unsigned int i_offset,
                      const unsigned int vector_j_offset,
                      const unsigned int scalar_j_offset,
                      const unsigned int lm_j_offset,
                      const unsigned int p_j_offset,
                      const unsigned int vel_component,
                      const unsigned int lm_u_j_offset,
                      const unsigned int lm_v_j_offset,
                      const MooseArray<Real> & JxW_face,
                      const QBase & qrule_face,
                      const MooseArray<Point> & normals,
                      const Elem * const neigh);

  void pressureDirichletResidual(const unsigned int i_offset,
                                 const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
                                 const MooseArray<Real> & JxW_face,
                                 const QBase & qrule_face,
                                 const MooseArray<Point> & normals,
                                 const Elem * const current_elem,
                                 const unsigned int current_side,
                                 const MooseArray<Point> & q_point_face);

  void scalarDirichletResidual(const unsigned int i_offset,
                               const MooseArray<Gradient> & vector_sol,
                               const MooseArray<Number> & scalar_sol,
                               const unsigned int vel_component,
                               const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
                               const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face);

  void scalarDirichletJacobian(const unsigned int i_offset,
                               const unsigned int vector_j_offset,
                               const unsigned int scalar_j_offset,
                               const unsigned int p_j_offset,
                               const unsigned int vel_component,
                               const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals);

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

  /// Number of dofs on elem
  std::size_t _p_n_dofs;
  std::size_t _global_lm_n_dofs;
};
