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

#include "libmesh/tensor_tools.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/type_n_tensor.h"
#include "libmesh/enum_fe_family.h"

#include <vector>

template <typename>
class MooseVariableField;
class SubProblem;
class SystemBase;
namespace libMesh
{
class DofMap;
}

template <typename OutputType>
class MooseVariableDataBase
{
public:
  // type for gradient, second and divergence of template class OutputType
  typedef typename libMesh::TensorTools::IncrementRank<OutputType>::type OutputGradient;
  typedef typename libMesh::TensorTools::IncrementRank<OutputGradient>::type OutputSecond;
  typedef typename libMesh::TensorTools::DecrementRank<OutputType>::type OutputDivergence;

  // shortcut for types storing values on quadrature points
  typedef MooseArray<OutputType> FieldVariableValue;
  typedef MooseArray<OutputGradient> FieldVariableGradient;

  // DoF value type for the template class OutputType
  typedef typename Moose::DOFType<OutputType>::type OutputData;
  typedef MooseArray<OutputData> DoFValue;

  MooseVariableDataBase(const MooseVariableField<OutputType> & var,
                        SystemBase & sys,
                        THREAD_ID tid);

  virtual ~MooseVariableDataBase() = default;

  /**
   * @return Whether this data is associated with a nodal variable
   */
  virtual bool isNodal() const = 0;

  /**
   * Whether this data is associated with a variable that has DoFs on nodes
   */
  virtual bool hasDoFsOnNodes() const = 0;

  /**
   * Return the variable continuity
   */
  virtual libMesh::FEContinuity getContinuity() const = 0;

  /**
   * Local solution getter
   * @param state The state of the simulation: current, old, older, previous nl
   */
  const FieldVariableValue & sln(Moose::SolutionState state) const;

  /**
   * Local solution gradient getter
   * @param state The state of the simulation: current, old, older, previous nl
   */
  const FieldVariableGradient & gradSln(Moose::SolutionState state) const;

  /**
   * The oldest solution state that is requested for this variable
   * (0 = current, 1 = old, 2 = older, etc).
   */
  unsigned int oldestSolutionStateRequested() const;

  /**
   * Set nodal value
   */
  void setNodalValue(const OutputType & value, unsigned int idx = 0);

  /**
   * Set the current local DOF values to the input vector
   */
  void insert(libMesh::NumericVector<libMesh::Number> & residual);

  /**
   * Add the current local DOF values to the input vector
   */
  void add(libMesh::NumericVector<libMesh::Number> & residual);

  /**
   * prepare the initial condition
   */
  void prepareIC();

  /////////////////////////// DoF value getters /////////////////////////////////////

  const DoFValue & dofValues() const;
  const DoFValue & dofValuesOld() const;
  const DoFValue & dofValuesOlder() const;
  const DoFValue & dofValuesPreviousNL() const;

  ///////////////////////// Nodal value getters ///////////////////////////////////////////

  const OutputType & nodalValue(Moose::SolutionState state) const;
  const MooseArray<OutputType> & nodalValueArray(Moose::SolutionState state) const;

  /////////////////////////////// Tags ///////////////////////////////////////////////////

  const FieldVariableValue & vectorTagValue(TagID tag) const;
  const FieldVariableGradient & vectorTagGradient(TagID tag) const;
  const FieldVariableValue & matrixTagValue(TagID tag) const;
  const DoFValue & nodalVectorTagValue(TagID tag) const;
  const DoFValue & nodalMatrixTagValue(TagID tag) const;
  const DoFValue & vectorTagDofValue(TagID tag) const;
  const DoFValue & vectorTagDofValue(Moose::SolutionState state) const;

  /**
   * Set the active vector tags
   * @param vtags Additional vector tags that this variable will need to query at dof indices for,
   * in addition to our own required solution tags
   */
  void setActiveTags(const std::set<TagID> & vtags);

  /**
   * Clear aux state
   */
  void prepareAux() { _has_dof_values = false; }

protected:
  /**
   * @returns The variable to which the data in this class belongs to
   */
  virtual const MooseVariableField<OutputType> & var() const { return _var; }

  /**
   * insert a solution tag into our tag containers
   */
  void insertSolutionTag(TagID tag_id);

  /**
   * Request that we have at least \p state number of older solution states/vectors
   */
  void needSolutionState(unsigned int state);

  /**
   * Helper methods for assigning dof values from their corresponding solution values
   */
  void fetchDoFValues();
  void zeroSizeDofValues();
  void getArrayDoFValues(const libMesh::NumericVector<libMesh::Number> & sol,
                         unsigned int n,
                         MooseArray<RealEigenVector> & dof_values) const;
  void assignNodalValue();

  /**
   * Helper method that converts a \p SolutionState argument into a corresponding tag ID,
   * potentially requesting necessary additional solution states and assigning tag id data members,
   * and then calls the provided \p functor with the tag ID
   */
  template <typename ReturnType, typename Functor>
  const ReturnType & stateToTagHelper(Moose::SolutionState state, Functor functor);

  /**
   * resize the vector tag need flags and data containers to accomodate this tag index
   */
  void resizeVectorTagData(TagID tag);

  /// The MOOSE system which ultimately holds the vectors and matrices relevant to this variable
  /// data
  SystemBase & _sys;

  /// The subproblem which we can query for information related to tagged vectors and matrices
  const SubProblem & _subproblem;

  /// The thread ID that this object is on
  const THREAD_ID _tid;

  /// The degree of freedom map from libMesh
  const libMesh::DofMap & _dof_map;

  /// Number of components of the associated variable
  unsigned int _count;

  /// Whether we currently have degree of freedom values stored in our local containers
  /// (corresponding to the current element)
  bool _has_dof_values;

  /// The maximum number of older solution states our variable needs
  unsigned int _max_state;

  /// The vector tag ID corresponding to the solution vector
  TagID _solution_tag;

  /// The vector tag ID corresponding to the old solution vector
  TagID _old_solution_tag;

  /// The vector tag ID corresponding to the older solution vector
  TagID _older_solution_tag;

  /// The vector tag ID corresponding to the previous nonlinear iteration's solution vector
  TagID _previous_nl_solution_tag;

  /// The dof indices for the current element
  std::vector<dof_id_type> _dof_indices;

  mutable std::vector<bool> _need_vector_tag_dof_u;
  mutable std::vector<bool> _need_matrix_tag_dof_u;

  // Dof values of tagged vectors
  std::vector<DoFValue> _vector_tags_dof_u;
  // Dof values of the diagonal of tagged matrices
  std::vector<DoFValue> _matrix_tags_dof_u;

  std::vector<FieldVariableValue> _vector_tag_u;
  mutable std::vector<bool> _need_vector_tag_u;
  std::vector<FieldVariableGradient> _vector_tag_grad;
  mutable std::vector<bool> _need_vector_tag_grad;
  std::vector<FieldVariableValue> _matrix_tag_u;
  mutable std::vector<bool> _need_matrix_tag_u;

  /// Nodal values
  OutputType _nodal_value;
  OutputType _nodal_value_old;
  OutputType _nodal_value_older;
  OutputType _nodal_value_previous_nl;

  /// Nodal values as MooseArrays for use with AuxKernels
  MooseArray<OutputType> _nodal_value_array;
  MooseArray<OutputType> _nodal_value_old_array;
  MooseArray<OutputType> _nodal_value_older_array;

  /// u dot flags
  mutable bool _need_u_dot;
  mutable bool _need_u_dotdot;
  mutable bool _need_u_dot_old;
  mutable bool _need_u_dotdot_old;
  mutable bool _need_du_dot_du;
  mutable bool _need_du_dotdot_du;

  /// gradient dot flags
  mutable bool _need_grad_dot;
  mutable bool _need_grad_dotdot;

  /// local solution flags
  mutable bool _need_dof_values_dot;
  mutable bool _need_dof_values_dotdot;
  mutable bool _need_dof_values_dot_old;
  mutable bool _need_dof_values_dotdot_old;
  mutable bool _need_dof_du_dot_du;
  mutable bool _need_dof_du_dotdot_du;

  /// time derivative of the solution values
  DoFValue _dof_values_dot;
  /// second time derivative of the solution values
  DoFValue _dof_values_dotdot;
  /// the previous time step's solution value time derivative
  DoFValue _dof_values_dot_old;
  /// the previous time step's solution value second time derivative
  DoFValue _dof_values_dotdot_old;
  /// derivatives of the solution value time derivative with respect to the degrees of freedom
  MooseArray<libMesh::Number> _dof_du_dot_du;
  /// derivatives of the solution value second time derivative with respect to the degrees of
  /// freedom
  MooseArray<libMesh::Number> _dof_du_dotdot_du;

  /// nodal values of u_dot
  OutputType _nodal_value_dot;
  /// nodal values of u_dotdot
  OutputType _nodal_value_dotdot;
  /// nodal values of u_dot_old
  OutputType _nodal_value_dot_old;
  /// nodal values of u_dotdot_old
  OutputType _nodal_value_dotdot_old;

  /// The set of vector tags (residual + solution) we need to evaluate
  std::set<TagID> _required_vector_tags;

  /// The set of solution tags we need to evaluate
  std::set<TagID> _solution_tags;

private:
  /// A const reference to the owning MooseVariableField object
  const MooseVariableField<OutputType> & _var;
};

template <>
void MooseVariableDataBase<RealEigenVector>::fetchDoFValues();

template <>
void MooseVariableDataBase<RealVectorValue>::assignNodalValue();

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::setActiveTags(const std::set<TagID> & vtags)
{
  _required_vector_tags = _solution_tags;
  for (const auto tag : vtags)
    _required_vector_tags.insert(tag);

  if (!_required_vector_tags.empty())
  {
    const auto largest_tag_id = *_required_vector_tags.rbegin();
    if (largest_tag_id >= _need_vector_tag_dof_u.size())
      resizeVectorTagData(largest_tag_id);
  }
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::insertSolutionTag(const TagID tag_id)
{
  _solution_tags.insert(tag_id);
  _required_vector_tags.insert(tag_id);
}
