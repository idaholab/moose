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
#include "Kernel.h"
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
class HDGData;
class MooseObject;
class MaterialPropertyInterface;

#define constructDiffusion()                                                                       \
  _u_var(_sys.getFieldVariable<Real>(_tid, getParam<NonlinearVariableName>("u"))),                 \
      _grad_u_var(_sys.getFieldVariable<RealVectorValue>(                                          \
          _tid, getParam<NonlinearVariableName>("grad_u"))),                                       \
      _u_face_var(_sys.getFieldVariable<Real>(_tid, getParam<NonlinearVariableName>("face_u"))),   \
      _qu_dof_indices(_grad_u_var.dofIndices()), _u_dof_indices(_u_var.dofIndices()),              \
      _lm_u_dof_indices(_u_face_var.dofIndices()), _qu_sol(_grad_u_var.sln()),                     \
      _u_sol(_u_var.sln()), _lm_u_sol(_u_face_var.sln()), _vector_phi(_grad_u_var.phi()),          \
      _scalar_phi(_u_var.phi()), _grad_scalar_phi(_u_var.gradPhi()),                               \
      _div_vector_phi(_grad_u_var.divPhi()), _vector_phi_face(_grad_u_var.phiFace()),              \
      _scalar_phi_face(_u_var.phiFace()), _lm_phi_face(_u_face_var.phiFace()),                     \
      _diff(getMaterialProperty<Real>("diffusivity")), _tau(getParam<Real>("tau")),                \
      _my_elem(nullptr)

#define declareDiffusionMembers                                                                    \
protected:                                                                                         \
  const MooseVariableFE<Real> & _u_var;                                                            \
  const MooseVariableFE<RealVectorValue> & _grad_u_var;                                            \
  const MooseVariableFE<Real> & _u_face_var;                                                       \
  const std::vector<dof_id_type> & _qu_dof_indices;                                                \
  const std::vector<dof_id_type> & _u_dof_indices;                                                 \
  const std::vector<dof_id_type> & _lm_u_dof_indices;                                              \
  const MooseArray<libMesh::Gradient> & _qu_sol;                                                   \
  const MooseArray<Number> & _u_sol;                                                               \
  const MooseArray<Number> & _lm_u_sol;                                                            \
  const MooseArray<std::vector<RealVectorValue>> & _vector_phi;                                    \
  const MooseArray<std::vector<Real>> & _scalar_phi;                                               \
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi;                               \
  const MooseArray<std::vector<Real>> & _div_vector_phi;                                           \
  const MooseArray<std::vector<RealVectorValue>> & _vector_phi_face;                               \
  const MooseArray<std::vector<Real>> & _scalar_phi_face;                                          \
  const MooseArray<std::vector<Real>> & _lm_phi_face;                                              \
  const MaterialProperty<Real> & _diff;                                                            \
  const Real _tau;                                                                                 \
  DenseVector<Number> _vector_re, _scalar_re, _lm_re;                                              \
  DenseMatrix<Number> _vector_vector_jac, _vector_scalar_jac, _scalar_vector_jac,                  \
      _scalar_scalar_jac, _scalar_lm_jac, _lm_scalar_jac, _lm_lm_jac, _vector_lm_jac,              \
      _lm_vector_jac;                                                                              \
                                                                                                   \
private:                                                                                           \
  const Elem * _my_elem

/**
 * Implements all the methods for assembling a hybridized local discontinuous Galerkin (LDG-H),
 * which is a type of HDG method, discretization of the diffusion equation. These routines may be
 * called by both HDG kernels and integrated boundary conditions. The implementation here is based
 * (but not exactly based) on "A superconvergent LDG-hybridizable Galerkin method for second-order
 * elliptic problems" by Cockburn
 */
class DiffusionHDGKernel : public Kernel, public NonADFunctorInterface
{
public:
  static InputParameters validParams();
  static InputParameters diffusionParams();

  DiffusionHDGKernel(const InputParameters & params);
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void initialSetup() override;
  virtual void jacobianSetup() override;

  virtual std::set<std::string> additionalVariablesCovered() override;

  /**
   * Computes a local residual vector for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient of u, v are its associated test
   * functions, and u is the diffused scalar field
   */
  static void vectorVolumeResidual(const MooseArray<Gradient> & vector_sol,
                                   const MooseArray<Number> & scalar_sol,
                                   const MooseArray<Real> & JxW,
                                   const QBase & qrule,
                                   const MooseArray<std::vector<RealVectorValue>> & vector_phi,
                                   const MooseArray<std::vector<Real>> & div_vector_phi,
                                   DenseVector<Number> & vector_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * (q, v) + (u, div(v))
   * where q is the vector field representing the gradient, v are its associated test functions, and
   * u is the scalar field
   */
  static void vectorVolumeJacobian(const MooseArray<Real> & JxW,
                                   const QBase & qrule,
                                   const MooseArray<std::vector<RealVectorValue>> & vector_phi,
                                   const MooseArray<std::vector<Real>> & div_vector_phi,
                                   const MooseArray<std::vector<Real>> & scalar_phi,
                                   DenseMatrix<Number> & vector_vector_jac,
                                   DenseMatrix<Number> & vector_scalar_jac);

  /**
   * Computes a local residual vector for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  static void scalarVolumeResidual(const MooseArray<Gradient> & vector_field,
                                   const Moose::Functor<Real> & source,
                                   const MooseArray<Real> & JxW,
                                   const QBase & qrule,
                                   const Elem * const current_elem,
                                   const MooseArray<Point> & q_point,
                                   const MooseArray<std::vector<RealVectorValue>> & grad_scalar_phi,
                                   const MooseArray<std::vector<Real>> & scalar_phi,
                                   const MaterialProperty<Real> & diff,
                                   const TransientInterface & ti,
                                   DenseVector<Number> & scalar_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  static void scalarVolumeJacobian(const MooseArray<Real> & JxW,
                                   const QBase & qrule,
                                   const MooseArray<std::vector<RealVectorValue>> & grad_scalar_phi,
                                   const MooseArray<std::vector<RealVectorValue>> & vector_phi,
                                   const MaterialProperty<Real> & diff,
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
  static void vectorFaceResidual(const MooseArray<Number> & lm_sol,
                                 const MooseArray<Real> & JxW_face,
                                 const QBase & qrule_face,
                                 const MooseArray<Point> & normals,
                                 const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
                                 DenseVector<Number> & vector_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<\hat{u}, n*v>
   * where \hat{u} is the trace of the scalar field, n is the normal vector, and v are the test
   * functions associated with the gradient field
   */
  static void vectorFaceJacobian(const MooseArray<Real> & JxW_face,
                                 const QBase & qrule_face,
                                 const MooseArray<Point> & normals,
                                 const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
                                 const MooseArray<std::vector<Real>> & lm_phi_face,
                                 DenseMatrix<Number> & vector_lm_jac);

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  static void scalarFaceResidual(const MooseArray<Gradient> & vector_sol,
                                 const MooseArray<Number> & scalar_sol,
                                 const MooseArray<Number> & lm_sol,
                                 const MooseArray<Real> & JxW_face,
                                 const QBase & qrule_face,
                                 const MooseArray<Point> & normals,
                                 const MooseArray<std::vector<Real>> & scalar_phi_face,
                                 const MaterialProperty<Real> & diff,
                                 const Real tau,
                                 DenseVector<Number> & scalar_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  static void scalarFaceJacobian(const MooseArray<Real> & JxW_face,
                                 const QBase & qrule_face,
                                 const MooseArray<Point> & normals,
                                 const MooseArray<std::vector<Real>> & scalar_phi_face,
                                 const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
                                 const MooseArray<std::vector<Real>> & lm_phi_face,
                                 const MaterialProperty<Real> & diff,
                                 const Real tau,
                                 DenseMatrix<Number> & scalar_vector_jac,
                                 DenseMatrix<Number> & scalar_scalar_jac,
                                 DenseMatrix<Number> & scalar_lm_jac);

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  static void lmFaceResidual(const MooseArray<Gradient> & vector_sol,
                             const MooseArray<Number> & scalar_sol,
                             const MooseArray<Number> & lm_sol,
                             const MooseArray<Real> & JxW_face,
                             const QBase & qrule_face,
                             const MooseArray<Point> & normals,
                             const MooseArray<std::vector<Real>> & lm_phi_face,
                             const MaterialProperty<Real> & diff,
                             const Real tau,
                             DenseVector<Number> & lm_re);

  /**
   * Computes a local Jacobian matrix for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  static void lmFaceJacobian(const MooseArray<Real> & JxW_face,
                             const QBase & qrule_face,
                             const MooseArray<Point> & normals,
                             const MooseArray<std::vector<Real>> & lm_phi_face,
                             const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
                             const MooseArray<std::vector<Real>> & scalar_phi_face,
                             const MaterialProperty<Real> & diff,
                             const Real tau,
                             DenseMatrix<Number> & lm_vec_jac,
                             DenseMatrix<Number> & lm_scalar_jac,
                             DenseMatrix<Number> & lm_lm_jac);

  static void checkCoupling(const FEProblemBase & fe_problem,
                            const unsigned int nl_sys_num,
                            const MooseObject & diffusion_obj);

  declareDiffusionMembers;

protected:
  virtual Real computeQpResidual() override { mooseError("this will never be called"); }

  /// optional source
  const Moose::Functor<Real> & _source;

  /// Neighbor element pointer
  const Elem * _neigh;

  /// The face quadrature rule
  const QBase * const & _qrule_face;

  /// The physical locations of the quadrature points on the face
  const MooseArray<Point> & _q_point_face;

  /// transformed Jacobian weights on the current element face
  const MooseArray<Real> & _JxW_face;

  /// coordinate transformation on the face
  const MooseArray<Real> & _coord_face;

  /// face normals
  const MooseArray<Point> & _normals;

  /// The current side index
  const unsigned int & _current_side;
};
