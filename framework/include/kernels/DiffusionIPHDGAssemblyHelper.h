//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
 * Implements all the methods for assembling a hybridized interior penalty discontinuous Galerkin
 * (IPDG-H), which is a type of HDG method, discretization of the diffusion equation. These routines
 * may be called by both HDG kernels and integrated boundary conditions.
 */
class DiffusionIPHDGAssemblyHelper : public ADFunctorInterface
{
public:
  static InputParameters validParams();

  DiffusionIPHDGAssemblyHelper(const MooseObject * const moose_obj,
                               MaterialPropertyInterface * const mpi,
                               MooseVariableDependencyInterface * const mvdi,
                               const TransientInterface * const ti,
                               SystemBase & sys,
                               const Assembly & assembly,
                               const THREAD_ID tid);

protected:
  /**
   * Computes a local residual vector for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  void scalarVolume(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                    const Moose::Functor<Real> & source,
                    const MooseArray<Real> & JxW,
                    const QBase & qrule,
                    const Elem * const current_elem,
                    const MooseArray<Point> & q_point,
                    DenseVector<ADReal> & scalar_re);
  //
  // Methods which can be leveraged both on internal sides in the kernel and by boundary conditions
  //

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  void scalarFace(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                  const MooseArray<ADReal> & scalar_sol,
                  const MooseArray<ADReal> & lm_sol,
                  const MooseArray<Real> & JxW_face,
                  const QBase & qrule_face,
                  const MooseArray<Point> & normals,
                  DenseVector<ADReal> & scalar_re);

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, \mu> + <\tau * (u - \hat{u}) * n * n, \mu>
   */
  void lmFace(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
              const MooseArray<ADReal> & scalar_sol,
              const MooseArray<ADReal> & lm_sol,
              const MooseArray<Real> & JxW_face,
              const QBase & qrule_face,
              const MooseArray<Point> & normals,
              DenseVector<ADReal> & lm_re);

  /**
   * Weakly imposes a Dirichlet condition for the scalar field in the scalar field equation
   */
  void scalarDirichlet(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                       const MooseArray<ADReal> & lm_sol,
                       const Moose::Functor<Real> & dirichlet_value,
                       const MooseArray<Real> & JxW_face,
                       const QBase & qrule_face,
                       const MooseArray<Point> & normals,
                       const Elem * const current_elem,
                       const unsigned int current_side,
                       const MooseArray<Point> & q_point_face,
                       DenseVector<ADReal> & scalar_re);

  /**
   * Adds the LM residual for Dirichlet boundary faces
   */
  void lmDirichlet(const MooseArray<ADRealVectorValue> & grad_scalar_sol,
                   const MooseArray<ADReal> & lm_sol,
                   const Moose::Functor<Real> & dirichlet_value,
                   const MooseArray<Real> & JxW_face,
                   const QBase & qrule_face,
                   const MooseArray<Point> & normals,
                   const Elem * const current_elem,
                   const unsigned int current_side,
                   const MooseArray<Point> & q_point_face,
                   DenseVector<ADReal> & lm_re);

  const MooseVariableFE<Real> & _u_var;
  const MooseVariableFE<Real> & _u_face_var;

  // Containers for dof indices
  const std::vector<dof_id_type> & _u_dof_indices;
  const std::vector<dof_id_type> & _lm_u_dof_indices;

  // local solutions at quadrature points
  const MooseArray<ADReal> & _u_sol;
  const MooseArray<ADRealVectorValue> _grad_u_sol;
  const MooseArray<ADReal> & _lm_u_sol;

  // Element shape functions
  const MooseArray<std::vector<Real>> & _scalar_phi;
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi;

  // Face shape functions
  const MooseArray<std::vector<Real>> & _scalar_phi_face;
  const MooseArray<std::vector<RealVectorValue>> & _grad_scalar_phi_face;
  const MooseArray<std::vector<Real>> & _lm_phi_face;

  /// The diffusivity
  const MaterialProperty<Real> & _diff;

  /// Reference to transient interface
  const TransientInterface & _ti;

  /// Our stabilization coefficient
  const Real _alpha;

  /// The current element volume
  const Real & _elem_volume;

  /// The current element side area
  const Real & _side_area;

  /// A data member used for determining when to compute the Jacobian
  const Elem * _my_elem;

  // Local residual vectors
  DenseVector<ADReal> _scalar_re, _lm_re;
};
