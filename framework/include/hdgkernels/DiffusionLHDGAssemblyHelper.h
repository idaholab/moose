//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"
#include "MooseArray.h"
#include "MooseFunctor.h"
#include "Function.h"
#include "Kernel.h"
#include "MooseVariableDependencyInterface.h"
#include "NonADFunctorInterface.h"

#include "libmesh/vector_value.h"
#include <vector>

template <typename>
class MooseVariableFE;
class MooseVariableScalar;
template <typename>
class MooseArray;
class SystemBase;
class MooseMesh;
class MooseObject;
class MaterialPropertyInterface;
class MooseVariableDependencyInterface;

/**
 * Implements all the methods for assembling a hybridized local discontinuous Galerkin (LDG-H),
 * which is a type of HDG method, discretization of the diffusion equation. These routines may be
 * called by both HDG kernels and integrated boundary conditions. The implementation here is based
 * (but not exactly based) on "A superconvergent LDG-hybridizable Galerkin method for second-order
 * elliptic problems" by Cockburn
 */
class DiffusionLHDGAssemblyHelper : public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  DiffusionLHDGAssemblyHelper(const MooseObject * const moose_obj,
                             MaterialPropertyInterface * const mpi,
                             MooseVariableDependencyInterface * const mvdi,
                             const TransientInterface * const ti,
                             const FEProblemBase & fe_problem,
                             SystemBase & sys,
                             const THREAD_ID tid);
  void checkCoupling();

protected:
  /**
   * Computes a local residual vector for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient of u, v are its associated test
   * functions, and u is the diffused scalar field
   */
  void vectorVolumeResidual(const MooseArray<Gradient> & vector_sol,
                            const MooseArray<Number> & scalar_sol,
                            const MooseArray<Real> & JxW,
                            const libMesh::QBase & qrule,
                            DenseVector<Number> & vector_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient, v are its associated test functions, and
   * u is the scalar field
   */
  void vectorVolumeJacobian(const MooseArray<Real> & JxW,
                            const libMesh::QBase & qrule,
                            DenseMatrix<Number> & vector_vector_jac,
                            DenseMatrix<Number> & vector_scalar_jac);

  /**
   * Computes a local residual vector for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  void scalarVolumeResidual(const MooseArray<Gradient> & vector_field,
                            const Moose::Functor<Real> & source,
                            const MooseArray<Real> & JxW,
                            const libMesh::QBase & qrule,
                            const Elem * const current_elem,
                            const MooseArray<Point> & q_point,
                            DenseVector<Number> & scalar_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  void scalarVolumeJacobian(const MooseArray<Real> & JxW,
                            const libMesh::QBase & qrule,
                            DenseMatrix<Number> & scalar_vector_jac);

  //
  // Methods which can be leveraged both on internal sides in the kernel and by boundary conditions
  //

  /**
   * Computes a local residual vector for the weak form:
   * -<\hat{u}, n*v>
   * where \hat{u} is the trace of the scalar field, n is the normal vector, and v are the test
   * functions associated with the gradient field
   */
  void vectorFaceResidual(const MooseArray<Number> & lm_sol,
                          const MooseArray<Real> & JxW_face,
                          const libMesh::QBase & qrule_face,
                          const MooseArray<Point> & normals,
                          DenseVector<Number> & vector_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<\hat{u}, n*v>
   * where \hat{u} is the trace of the scalar field, n is the normal vector, and v are the test
   * functions associated with the gradient field
   */
  void vectorFaceJacobian(const MooseArray<Real> & JxW_face,
                          const libMesh::QBase & qrule_face,
                          const MooseArray<Point> & normals,
                          DenseMatrix<Number> & vector_lm_jac);

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  void scalarFaceResidual(const MooseArray<Gradient> & vector_sol,
                          const MooseArray<Number> & scalar_sol,
                          const MooseArray<Number> & lm_sol,
                          const MooseArray<Real> & JxW_face,
                          const libMesh::QBase & qrule_face,
                          const MooseArray<Point> & normals,
                          DenseVector<Number> & scalar_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  void scalarFaceJacobian(const MooseArray<Real> & JxW_face,
                          const libMesh::QBase & qrule_face,
                          const MooseArray<Point> & normals,
                          DenseMatrix<Number> & scalar_vector_jac,
                          DenseMatrix<Number> & scalar_scalar_jac,
                          DenseMatrix<Number> & scalar_lm_jac);

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  void lmFaceResidual(const MooseArray<Gradient> & vector_sol,
                      const MooseArray<Number> & scalar_sol,
                      const MooseArray<Number> & lm_sol,
                      const MooseArray<Real> & JxW_face,
                      const libMesh::QBase & qrule_face,
                      const MooseArray<Point> & normals,
                      DenseVector<Number> & lm_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  void lmFaceJacobian(const MooseArray<Real> & JxW_face,
                      const libMesh::QBase & qrule_face,
                      const MooseArray<Point> & normals,
                      DenseMatrix<Number> & lm_vec_jac,
                      DenseMatrix<Number> & lm_scalar_jac,
                      DenseMatrix<Number> & lm_lm_jac);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the vector (gradient) equation
   */
  void vectorDirichletResidual(const Moose::Functor<Real> & dirichlet_value,
                               const MooseArray<Real> & JxW_face,
                               const libMesh::QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face,
                               DenseVector<Number> & vector_re);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the scalar field equation
   */
  void scalarDirichletResidual(const MooseArray<Gradient> & vector_sol,
                               const MooseArray<Number> & scalar_sol,
                               const Moose::Functor<Real> & dirichlet_value,
                               const MooseArray<Real> & JxW_face,
                               const libMesh::QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               const Elem * const current_elem,
                               const unsigned int current_side,
                               const MooseArray<Point> & q_point_face,
                               DenseVector<Number> & scalar_re);

  /**
   * Computes the Jacobian for a Dirichlet condition for the scalar field in the scalar field
   * equation
   */
  void scalarDirichletJacobian(const MooseArray<Real> & JxW_face,
                               const libMesh::QBase & qrule_face,
                               const MooseArray<Point> & normals,
                               DenseMatrix<Number> & scalar_vector_jac,
                               DenseMatrix<Number> & scalar_scalar_jac);

  /**
   * Creates residuals corresponding to the weak form (v, \hat{u}), or stated simply this routine
   * can be used to drive Lagrange multiplier values on the boundary to zero. This should be used on
   * boundaries where there are Dirichlet conditions for the primal variables such that there is no
   * need for the Lagrange multiplier variables
   */
  void createIdentityResidual(const MooseArray<Real> & JxW,
                              const libMesh::QBase & qrule,
                              const MooseArray<std::vector<Real>> & phi,
                              const MooseArray<Number> & sol,
                              DenseVector<Number> & re);

  /**
   * As above, but for the Jacobians
   */
  void createIdentityJacobian(const MooseArray<Real> & JxW,
                              const libMesh::QBase & qrule,
                              const MooseArray<std::vector<Real>> & phi,
                              DenseMatrix<Number> & ke);

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

  /// Reference to transient interface
  const TransientInterface & _ti;

  /// Our stabilization coefficient
  const Real _tau;

  /// A data member used for determining when to compute the Jacobian
  const Elem * _my_elem;

  // Local residual vectors
  DenseVector<Number> _vector_re, _scalar_re, _lm_re;

  // Local Jacobian matrices
  DenseMatrix<Number> _vector_vector_jac, _vector_scalar_jac, _scalar_vector_jac,
      _scalar_scalar_jac, _scalar_lm_jac, _lm_scalar_jac, _lm_lm_jac, _vector_lm_jac,
      _lm_vector_jac;

private:
  /// A reference to our associated MooseObject for error reporting
  const MooseObject & _moose_obj;

  /// A reference to the finite element problem used for coupling checks
  const FEProblemBase & _dhah_fe_problem;

  /// A reference to the nonlinear system used for coupling checks
  const SystemBase & _dhah_sys;
};
