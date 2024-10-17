//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseArray.h"
#include "MooseTypes.h"
#include "MeshChangedInterface.h"
#include "MooseVariableDataBase.h"
#include "TheWarehouse.h"

#include "libmesh/tensor_tools.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/type_n_tensor.h"
#include "libmesh/fe_type.h"
#include "libmesh/dof_map.h"
#include "libmesh/enum_fe_family.h"
#include "SubProblem.h"
#include "MooseVariableDataFV.h"

#include <functional>
#include <vector>

class FaceInfo;
class SystemBase;
class TimeIntegrator;
class Assembly;

template <typename>
class MooseVariableFV;

namespace libMesh
{
class QBase;
}

/**
 * Class holding the data members for linear finite volume variables.
 * At the moment, this is only used when the user wants to use linear
 * finite volume variables in the postprocessor/userobject and auxiliary
 * systems. The solver-related functionalities rely on a different machinery.
 */
template <typename OutputType>
class MooseVariableDataLinearFV : public MooseVariableDataBase<OutputType>
{
public:
  // type for gradient, second and divergence of template class OutputType
  typedef typename libMesh::TensorTools::IncrementRank<OutputType>::type OutputGradient;
  typedef typename libMesh::TensorTools::IncrementRank<OutputGradient>::type OutputSecond;
  typedef typename libMesh::TensorTools::DecrementRank<OutputType>::type OutputDivergence;

  // shortcut for types storing values on quadrature points
  typedef MooseArray<OutputType> FieldVariableValue;
  typedef MooseArray<OutputGradient> FieldVariableGradient;
  typedef MooseArray<OutputSecond> FieldVariableSecond;
  typedef MooseArray<OutputType> FieldVariableCurl;
  typedef MooseArray<OutputDivergence> FieldVariableDivergence;

  // shape function type for the template class OutputType
  typedef typename Moose::ShapeType<OutputType>::type OutputShape;

  // type for gradient, second and divergence of shape functions of template class OutputType
  typedef typename libMesh::TensorTools::IncrementRank<OutputShape>::type OutputShapeGradient;
  typedef typename libMesh::TensorTools::IncrementRank<OutputShapeGradient>::type OutputShapeSecond;
  typedef typename libMesh::TensorTools::DecrementRank<OutputShape>::type OutputShapeDivergence;

  // DoF value type for the template class OutputType
  typedef typename Moose::DOFType<OutputType>::type OutputData;
  typedef MooseArray<OutputData> DoFValue;

  MooseVariableDataLinearFV(const MooseLinearVariableFV<OutputType> & var,
                            SystemBase & sys,
                            THREAD_ID tid,
                            Moose::ElementType element_type,
                            const Elem * const & elem);

  bool isNodal() const override { return false; }
  bool hasDoFsOnNodes() const override { return false; }
  libMesh::FEContinuity getContinuity() const override { return libMesh::DISCONTINUOUS; }

  /**
   * Set the geometry type before calculating variables values.
   * @param gm_type The type type of geometry; either Volume or Face
   */
  void setGeometry(Moose::GeometryType gm_type);

  /**
   * Compute the variable values.
   */
  void computeValues();

  /**
   * Set local DOF values to the entries of \p values .
   */
  void setDofValues(const DenseVector<OutputData> & values);

  /**
   * Set local DOF value at \p index to \p value .
   */
  void setDofValue(const OutputData & value, unsigned int index);

  /**
   * Get the dof indices for an element.
   * @param elem The element on which the dof indices shall be queried
   * @param dof_indices The container in which the dof indices will be copied
   */
  void getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices) const;

  /**
   * Get the dof indices of the current element.
   */
  const std::vector<dof_id_type> & dofIndices() const;

  /**
   * Get the number of dofs on the current element.
   */
  unsigned int numberOfDofs() const;

  /**
   * Clear the dof indices in the cache.
   */
  void clearDofIndices()
  {
    _dof_indices.clear();
    _prev_elem = nullptr;
  }

protected:
  /**
   * Get the corresponding variable.
   */
  virtual const MooseLinearVariableFV<OutputType> & var() const override { return _var; }

private:
  void initializeSolnVars();

  /// A const reference to the owning MooseLinearVariableFV object
  const MooseLinearVariableFV<OutputType> & _var;

  /// Reference to the variable's finite element type
  const libMesh::FEType & _fe_type;

  /// The index of the variable in the system
  const unsigned int _var_num;

  /// Reference to the system assembly of the variable
  const Assembly & _assembly;

  /// The element type this object is storing data for. This is either Element, Neighbor, or Lower
  Moose::ElementType _element_type;

  /// Pointer to time integrator
  const TimeIntegrator * const _time_integrator;

  /// The current elem. This has to be a reference because the current elem will be constantly
  /// changing. If we initialized this to point to one elem, then in the next calculation we would
  /// be pointing to the wrong place!
  const Elem * const & _elem;

  /// used to keep track of when dof indices are out of date
  mutable const Elem * _prev_elem = nullptr;

  /**
   * Fetch and return the dof indices of this variable on the current element.
   */
  const std::vector<dof_id_type> & initDofIndices();

  /// Whether this variable is being calculated on a displaced system
  const bool _displaced;

  /// Pointer to the quadrature rule
  const libMesh::QBase * _qrule;

  using MooseVariableDataBase<OutputType>::_sys;
  using MooseVariableDataBase<OutputType>::_subproblem;
  using MooseVariableDataBase<OutputType>::_need_vector_tag_dof_u;
  using MooseVariableDataBase<OutputType>::_need_matrix_tag_dof_u;
  using MooseVariableDataBase<OutputType>::_vector_tags_dof_u;
  using MooseVariableDataBase<OutputType>::_matrix_tags_dof_u;
  using MooseVariableDataBase<OutputType>::_vector_tag_u;
  using MooseVariableDataBase<OutputType>::_need_vector_tag_u;
  using MooseVariableDataBase<OutputType>::_vector_tag_grad;
  using MooseVariableDataBase<OutputType>::_need_vector_tag_grad;
  using MooseVariableDataBase<OutputType>::_matrix_tag_u;
  using MooseVariableDataBase<OutputType>::_need_matrix_tag_u;
  using MooseVariableDataBase<OutputType>::_dof_indices;
  using MooseVariableDataBase<OutputType>::_has_dof_values;
  using MooseVariableDataBase<OutputType>::fetchDoFValues;
  using MooseVariableDataBase<OutputType>::assignNodalValue;
  using MooseVariableDataBase<OutputType>::zeroSizeDofValues;
  using MooseVariableDataBase<OutputType>::_solution_tag;
  using MooseVariableDataBase<OutputType>::_old_solution_tag;
  using MooseVariableDataBase<OutputType>::_older_solution_tag;
  using MooseVariableDataBase<OutputType>::_previous_nl_solution_tag;
  using MooseVariableDataBase<OutputType>::_dof_map;
  using MooseVariableDataBase<OutputType>::_need_u_dot;
  using MooseVariableDataBase<OutputType>::_need_u_dotdot;
  using MooseVariableDataBase<OutputType>::_need_u_dot_old;
  using MooseVariableDataBase<OutputType>::_need_u_dotdot_old;
  using MooseVariableDataBase<OutputType>::_need_du_dot_du;
  using MooseVariableDataBase<OutputType>::_need_du_dotdot_du;
  using MooseVariableDataBase<OutputType>::_need_grad_dot;
  using MooseVariableDataBase<OutputType>::_need_grad_dotdot;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dot;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dotdot;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dot_old;
  using MooseVariableDataBase<OutputType>::_need_dof_values_dotdot_old;
  using MooseVariableDataBase<OutputType>::_need_dof_du_dot_du;
  using MooseVariableDataBase<OutputType>::_need_dof_du_dotdot_du;
  using MooseVariableDataBase<OutputType>::_dof_values_dot;
  using MooseVariableDataBase<OutputType>::_dof_values_dotdot;
  using MooseVariableDataBase<OutputType>::_dof_values_dot_old;
  using MooseVariableDataBase<OutputType>::_dof_values_dotdot_old;
  using MooseVariableDataBase<OutputType>::_dof_du_dot_du;
  using MooseVariableDataBase<OutputType>::_dof_du_dotdot_du;
  using MooseVariableDataBase<OutputType>::_tid;
  using MooseVariableDataBase<OutputType>::_nodal_value_dot;
  using MooseVariableDataBase<OutputType>::_nodal_value_dotdot;
  using MooseVariableDataBase<OutputType>::_nodal_value_dot_old;
  using MooseVariableDataBase<OutputType>::_nodal_value_dotdot_old;
  using MooseVariableDataBase<OutputType>::_required_vector_tags;

  friend void Moose::initDofIndices<>(MooseVariableDataLinearFV<OutputType> &, const Elem &);
};

/////////////////////// General template definitions //////////////////////////////////////

template <typename OutputType>
const std::vector<dof_id_type> &
MooseVariableDataLinearFV<OutputType>::dofIndices() const
{
  return const_cast<MooseVariableDataLinearFV<OutputType> *>(this)->initDofIndices();
}

template <typename OutputType>
unsigned int
MooseVariableDataLinearFV<OutputType>::numberOfDofs() const
{
  return dofIndices().size();
}
