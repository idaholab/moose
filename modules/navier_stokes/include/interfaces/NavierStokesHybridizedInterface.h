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
#include "libmesh/vector_value.h"
#include <vector>

#include <Eigen/Dense>

#ifdef LIBMESH_USE_COMPLEX_NUMBERS
typedef Eigen::MatrixXcd EigenMatrix;
typedef Eigen::VectorXcd EigenVector;
#else
typedef Eigen::MatrixXd EigenMatrix;
typedef Eigen::VectorXd EigenVector;
#endif

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
namespace libMesh
{
class Elem;
class Point;
class QBase;
}

class NavierStokesHybridizedInterface
{
public:
  static InputParameters validParams();

  NavierStokesHybridizedInterface(const MooseObject * moose_obj,
                                  MaterialPropertyInterface * mpi,
                                  SystemBase & nl_sys,
                                  SystemBase & aux_sys,
                                  const MooseMesh & mesh,
                                  const THREAD_ID tid);

protected:
  /**
   * Set the number of degree of freedom variables and resize the Eigen data structures
   */
  template <typename NSHybridized>
  static void resizeData(NSHybridized & obj);

  const MooseVariableFE<Real> & _u_var;
  const MooseVariableFE<Real> & _v_var;
  const MooseVariableFE<Real> * const _w_var;
  const MooseVariableFE<RealVectorValue> & _grad_u_var;
  const MooseVariableFE<RealVectorValue> & _grad_v_var;
  const MooseVariableFE<RealVectorValue> * const _grad_w_var;
  const MooseVariableFE<Real> & _u_face_var;
  const MooseVariableFE<Real> & _v_face_var;
  const MooseVariableFE<Real> * const _w_face_var;
  const MooseVariableFE<Real> & _pressure_var;
  const MooseVariableScalar * const _enclosure_lm_var;

  // Containers for dof indices
  const std::vector<dof_id_type> & _qu_dof_indices;
  const std::vector<dof_id_type> & _u_dof_indices;
  const std::vector<dof_id_type> & _lm_u_dof_indices;
  const std::vector<dof_id_type> & _qv_dof_indices;
  const std::vector<dof_id_type> & _v_dof_indices;
  const std::vector<dof_id_type> & _lm_v_dof_indices;
  const std::vector<dof_id_type> * const _qw_dof_indices;
  const std::vector<dof_id_type> * const _w_dof_indices;
  const std::vector<dof_id_type> * const _lm_w_dof_indices;
  const std::vector<dof_id_type> & _p_dof_indices;
  const std::vector<dof_id_type> * const _global_lm_dof_indices;

  // local solutions at quadrature points
  const MooseArray<Gradient> & _qu_sol;
  const MooseArray<Number> & _u_sol;
  const MooseArray<Number> & _lm_u_sol;
  const MooseArray<Gradient> & _qv_sol;
  const MooseArray<Number> & _v_sol;
  const MooseArray<Number> & _lm_v_sol;
  const MooseArray<Gradient> * const _qw_sol;
  const MooseArray<Number> * const _w_sol;
  const MooseArray<Number> * const _lm_w_sol;
  const MooseArray<Number> & _p_sol;
  const MooseArray<Number> * const _global_lm_dof_value;

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
  const MaterialProperty<Real> & _nu;

  // Number of dofs on elem
  std::size_t _vector_n_dofs;
  std::size_t _scalar_n_dofs;
  std::size_t _lm_n_dofs;
  std::size_t _p_n_dofs;
  std::size_t _global_lm_n_dofs;

  /// Our stabilization coefficient
  static constexpr Real _tau = 1;
};

template <typename NSHybridized>
void
NavierStokesHybridizedInterface::resizeData(NSHybridized & obj)
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
