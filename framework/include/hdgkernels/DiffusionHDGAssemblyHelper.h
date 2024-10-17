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
#include "HDGData.h"
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
class DiffusionHDGAssemblyHelper : virtual public HDGData
{
public:
  static InputParameters validParams();

  DiffusionHDGAssemblyHelper(const MooseObject * const moose_obj,
                             MaterialPropertyInterface * const mpi,
                             const TransientInterface * const ti,
                             SystemBase & nl_sys,
                             SystemBase & aux_sys,
                             const THREAD_ID tid);

protected:
  virtual std::string physics() const override { return "diffusion"; }
  virtual std::set<const MooseVariableBase *> variables() const override;

  /**
   * Set the number of degree of freedom variables and resize the Eigen data structures
   */
  void resizeData();

  /**
   * Computes a local residual vector for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient of u, v are its associated test
   * functions, and u is the diffused scalar field
   */
  void vectorVolumeResidual(const unsigned int i_offset,
                            const MooseArray<libMesh::Gradient> & vector_sol,
                            const MooseArray<Number> & scalar_sol,
                            const MooseArray<Real> & JxW,
                            const QBase & qrule);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient, v are its associated test functions, and
   * u is the scalar field
   */
  void vectorVolumeJacobian(const unsigned int i_offset,
                            const unsigned int vector_j_offset,
                            const unsigned int scalar_j_offset,
                            const MooseArray<Real> & JxW,
                            const QBase & qrule);

  /**
   * Computes a local residual vector for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  void scalarVolumeResidual(const unsigned int i_offset,
                            const MooseArray<libMesh::Gradient> & vector_field,
                            const Moose::Functor<Real> & source,
                            const MooseArray<Real> & JxW,
                            const QBase & qrule,
                            const Elem * const current_elem,
                            const MooseArray<Point> & q_point);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  void scalarVolumeJacobian(const unsigned int i_offset,
                            const unsigned int vector_field_j_offset,
                            const MooseArray<Real> & JxW,
                            const QBase & qrule);

  //
  // Methods which can be leveraged both on internal sides in the kernel and by boundary conditions
  //

  /**
   * Computes a local residual vector for the weak form:
   * -<\hat{u}, n*v>
   * where \hat{u} is the trace of the scalar field, n is the normal vector, and v are the test
   * functions associated with the gradient field
   */
  void vectorFaceResidual(const unsigned int i_offset,
                          const MooseArray<Number> & lm_sol,
                          const MooseArray<Real> & JxW_face,
                          const QBase & qrule_face,
                          const MooseArray<Point> & normals);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<\hat{u}, n*v>
   * where \hat{u} is the trace of the scalar field, n is the normal vector, and v are the test
   * functions associated with the gradient field
   */
  void vectorFaceJacobian(const unsigned int i_offset,
                          const unsigned int lm_j_offset,
                          const MooseArray<Real> & JxW_face,
                          const QBase & qrule_face,
                          const MooseArray<Point> & normals);

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  void scalarFaceResidual(const unsigned int i_offset,
                          const MooseArray<libMesh::Gradient> & vector_sol,
                          const MooseArray<Number> & scalar_sol,
                          const MooseArray<Number> & lm_sol,
                          const MooseArray<Real> & JxW_face,
                          const QBase & qrule_face,
                          const MooseArray<Point> & normals);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  void scalarFaceJacobian(const unsigned int i_offset,
                          const unsigned int vector_j_offset,
                          const unsigned int scalar_j_offset,
                          const unsigned int lm_j_offset,
                          const MooseArray<Real> & JxW_face,
                          const QBase & qrule_face,
                          const MooseArray<Point> & normals);

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  void lmFaceResidual(const unsigned int i_offset,
                      const MooseArray<libMesh::Gradient> & vector_sol,
                      const MooseArray<Number> & scalar_sol,
                      const MooseArray<Number> & lm_sol,
                      const MooseArray<Real> & JxW_face,
                      const QBase & qrule_face,
                      const MooseArray<Point> & normals);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  void lmFaceJacobian(const unsigned int i_offset,
                      const unsigned int vector_j_offset,
                      const unsigned int scalar_j_offset,
                      const unsigned int lm_j_offset,
                      const MooseArray<Real> & JxW_face,
                      const QBase & qrule_face,
                      const MooseArray<Point> & normals);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the vector (gradient) equation
   */
  void vectorDirichletResidual(const unsigned int i_offset,
                               const Moose::Functor<Real> & dirichlet_value,
                               const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the scalar field equation
   */
  void scalarDirichletResidual(const unsigned int i_offset,
                               const MooseArray<libMesh::Gradient> & vector_sol,
                               const MooseArray<Number> & scalar_sol,
                               const Moose::Functor<Real> & dirichlet_value,
                               const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face);

  /**
   * Computes the Jacobian for a Dirichlet condition for the scalar field in the scalar field
   * equation
   */
  void scalarDirichletJacobian(const unsigned int i_offset,
                               const unsigned int vector_j_offset,
                               const unsigned int scalar_j_offset,
                               const MooseArray<Real> & JxW_face,
                               const QBase & qrule_face,
                               const MooseArray<Point> & normals);

  const MooseVariableFE<Real> & _u_var;
  const MooseVariableFE<RealVectorValue> & _grad_u_var;
  const MooseVariableFE<Real> & _u_face_var;

  // Containers for dof indices
  const std::vector<dof_id_type> & _qu_dof_indices;
  const std::vector<dof_id_type> & _u_dof_indices;
  const std::vector<dof_id_type> & _lm_u_dof_indices;

  // local solutions at quadrature points
  const MooseArray<libMesh::Gradient> & _qu_sol;
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

  /// The diffusivity
  const MaterialProperty<Real> & _diff;

  // Number of dofs on elem
  std::size_t _vector_n_dofs;
  std::size_t _scalar_n_dofs;
  std::size_t _lm_n_dofs;

  /// Reference to transient interface
  const TransientInterface & _ti;

  /// Local sizes of the systems
  std::size_t _primal_size, _lm_size;

  /// Our stabilization coefficient
  const Real _tau;
};
