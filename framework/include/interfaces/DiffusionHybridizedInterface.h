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
