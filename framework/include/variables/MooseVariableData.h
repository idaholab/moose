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
#include "libmesh/fe_type.h"
#include "DualRealOps.h"

#include <functional>
#include <vector>

namespace libMesh
{
class QBase;
class DofMap;
template <typename>
class DenseVector;
}

using libMesh::DenseVector;
using libMesh::DofMap;
using libMesh::QBase;
using libMesh::VectorValue;

class TimeIntegrator;
class Assembly;
class SubProblem;
template <typename>
class MooseVariableFE;
class SystemBase;

namespace Moose
{
enum SolutionState
{
  Current,
  Old,
  Older,
  PreviousNL
};
enum GeometryType
{
  Volume,
  Face
};
}

template <typename OutputType>
class MooseVariableData
{
public:
  // type for gradient, second and divergence of template class OutputType
  typedef typename TensorTools::IncrementRank<OutputType>::type OutputGradient;
  typedef typename TensorTools::IncrementRank<OutputGradient>::type OutputSecond;
  typedef typename TensorTools::DecrementRank<OutputType>::type OutputDivergence;

  // shortcut for types storing values on quadrature points
  typedef MooseArray<OutputType> FieldVariableValue;
  typedef MooseArray<OutputGradient> FieldVariableGradient;
  typedef MooseArray<OutputSecond> FieldVariableSecond;
  typedef MooseArray<OutputType> FieldVariableCurl;
  typedef MooseArray<OutputDivergence> FieldVariableDivergence;

  // shape function type for the template class OutputType
  typedef typename Moose::ShapeType<OutputType>::type OutputShape;

  // type for gradient, second and divergence of shape functions of template class OutputType
  typedef typename TensorTools::IncrementRank<OutputShape>::type OutputShapeGradient;
  typedef typename TensorTools::IncrementRank<OutputShapeGradient>::type OutputShapeSecond;
  typedef typename TensorTools::DecrementRank<OutputShape>::type OutputShapeDivergence;

  // shortcut for types storing shape function values on quadrature points
  typedef MooseArray<std::vector<OutputShape>> FieldVariablePhiValue;
  typedef MooseArray<std::vector<OutputShapeGradient>> FieldVariablePhiGradient;
  typedef MooseArray<std::vector<OutputShapeSecond>> FieldVariablePhiSecond;
  typedef MooseArray<std::vector<OutputShape>> FieldVariablePhiCurl;
  typedef MooseArray<std::vector<OutputShapeDivergence>> FieldVariablePhiDivergence;

  // shortcut for types storing test function values on quadrature points
  // Note: here we assume the types are the same as of shape functions.
  typedef MooseArray<std::vector<OutputShape>> FieldVariableTestValue;
  typedef MooseArray<std::vector<OutputShapeGradient>> FieldVariableTestGradient;
  typedef MooseArray<std::vector<OutputShapeSecond>> FieldVariableTestSecond;
  typedef MooseArray<std::vector<OutputShape>> FieldVariableTestCurl;
  typedef MooseArray<std::vector<OutputShapeDivergence>> FieldVariableTestDivergence;

  // DoF value type for the template class OutputType
  typedef typename Moose::DOFType<OutputType>::type OutputData;
  typedef MooseArray<OutputData> DoFValue;

  MooseVariableData(const MooseVariableFE<OutputType> & var,
                    const SystemBase & sys,
                    THREAD_ID tid,
                    Moose::ElementType element_type,
                    const QBase * const & qrule_in,
                    const QBase * const & qrule_face_in,
                    const Node * const & node,
                    const Elem * const & elem);

  /**
   * Returns whether this data structure needs automatic differentiation calculations
   */
  bool needsAD() const { return _need_ad; }

  /**
   * Set the geometry type before calculating variables values
   * @param gm_type The type type of geometry; either Volume or Face
   */
  void setGeometry(Moose::GeometryType gm_type);

  //////////////// Heavy lifting computational routines //////////////////////////////

  /**
   * compute the variable values
   */
  void computeValues();

  /**
   * compute the values for const monomial variables
   */
  void computeMonomialValues();

  /**
   * compute AD things
   */
  void computeAD(const unsigned int num_dofs, const unsigned int nqp);

  /**
   * compute nodal things
   */
  void computeNodalValues();

  ///////////////////////////// Shape functions /////////////////////////////////////

  /**
   * phi getter
   */
  const FieldVariablePhiValue & phi() const { return *_phi; }

  /**
   * phi_face getter
   */
  const FieldVariablePhiValue & phiFace() const { return *_phi_face; }

  /**
   * grad_phi getter
   */
  const FieldVariablePhiGradient & gradPhi() const { return *_grad_phi; }

  /**
   * mapped_grad_phi getter
   */
  const MappedArrayVariablePhiGradient & arrayGradPhi() const
  {
    mooseAssert(_var.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY, "Not an array variable");
    return _mapped_grad_phi;
  }

  /**
   * grad_phi_face getter
   */
  const FieldVariablePhiGradient & gradPhiFace() const { return *_grad_phi_face; }

  /**
   * mapped_grad_phi_face getter
   */
  const MappedArrayVariablePhiGradient & arrayGradPhiFace() const
  {
    mooseAssert(_var.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY, "Not an array variable");
    return _mapped_grad_phi_face;
  }

  /**
   * second_phi getter
   */
  const FieldVariablePhiSecond & secondPhi() const;

  /**
   * second_phi_face getter
   */
  const FieldVariablePhiSecond & secondPhiFace() const;

  /**
   * curl_phi getter
   */
  const FieldVariablePhiCurl & curlPhi() const;

  /**
   * curl_phi_face getter
   */
  const FieldVariablePhiCurl & curlPhiFace() const;

  /**
   * ad_grad_phi getter
   */
  template <ComputeStage compute_stage>
  const typename VariableTestGradientType<OutputShape, compute_stage>::type & adGradPhi() const
  {
    return *_ad_grad_phi;
  }

  /**
   * ad_grad_phi_face getter
   */
  template <ComputeStage compute_stage>
  const typename VariableTestGradientType<OutputShape, compute_stage>::type & adGradPhiFace() const
  {
    return *_ad_grad_phi_face;
  }

  /**
   * Return phi size
   */
  size_t phiSize() const { return _phi->size(); }

  /**
   * Return phiFace size
   */
  size_t phiFaceSize() const { return _phi_face->size(); }

  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool usesSecondPhi() const
  {
    return _need_second || _need_second_old || _need_second_older || _need_second_previous_nl;
  }

  /**
   * Whether or not this variable is computing the curl
   */
  bool computingCurl() const { return _need_curl || _need_curl_old; }

  //////////////////////////////// Nodal stuff ///////////////////////////////////////////

  bool isNodal() const { return _is_nodal; }
  const Node * const & node() const { return _node; }
  const dof_id_type & nodalDofIndex() const { return _nodal_dof_index; }
  bool isNodalDefined() const { return _has_dof_indices; }

  /**
   * The current element
   */
  const Elem * const & currentElem() const { return _elem; }

  /**
   * The current side
   */
  const unsigned int & currentSide() const { return _current_side; }

  /**
   * prepare the initial condition
   */
  void prepareIC();

  //////////////////////////////////// Solution getters /////////////////////////////////////

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
   * Local time derivative of solution gradient getter
   */
  const FieldVariableGradient & gradSlnDot() const;

  /**
   * Local second time derivative of solution gradient getter
   */
  const FieldVariableGradient & gradSlnDotDot() const;

  /**
   * Local solution second spatial derivative getter
   * @param state The state of the simulation: current, old, older, previous nl
   */
  const FieldVariableSecond & secondSln(Moose::SolutionState state) const;

  /**
   * Local solution curl getter
   * @param state The state of the simulation: current, old, older
   */
  const FieldVariableCurl & curlSln(Moose::SolutionState state) const;

  template <ComputeStage compute_stage>
  const typename VariableValueType<OutputType, compute_stage>::type & adSln() const
  {
    _need_ad = _need_ad_u = true;
    return _ad_u;
  }

  template <ComputeStage compute_stage>
  const typename VariableGradientType<OutputType, compute_stage>::type & adGradSln() const
  {
    _need_ad = _need_ad_grad_u = true;
    return _ad_grad_u;
  }

  template <ComputeStage compute_stage>
  const typename VariableSecondType<OutputType, compute_stage>::type & adSecondSln() const
  {
    _need_ad = _need_ad_second_u = true;
    secondPhi();
    secondPhiFace();
    return _ad_second_u;
  }

  template <ComputeStage compute_stage>
  const typename VariableValueType<OutputType, compute_stage>::type & adUDot() const
  {
    _need_ad = _need_ad_u_dot = true;
    return _ad_u_dot;
  }

  const FieldVariableValue & uDot() const;

  const FieldVariableValue & uDotDot() const;

  const FieldVariableValue & uDotOld() const;

  const FieldVariableValue & uDotDotOld() const;

  const VariableValue & duDotDu() const
  {
    _need_du_dot_du = true;
    return _du_dot_du;
  }

  const VariableValue & duDotDotDu() const
  {
    _need_du_dotdot_du = true;
    return _du_dotdot_du;
  }

  ///////////////////////// Nodal value getters ///////////////////////////////////////////

  const OutputType & nodalValue(Moose::SolutionState state) const;
  const MooseArray<OutputType> & nodalValueArray(Moose::SolutionState state) const;
  const OutputType & nodalValueDot() const;
  const OutputType & nodalValueDotDot() const;
  const OutputType & nodalValueDotOld() const;
  const OutputType & nodalValueDotDotOld() const;
  const OutputType & nodalValueDuDotDu() const;
  const OutputType & nodalValueDuDotDotDu() const;

  template <ComputeStage compute_stage>
  const typename Moose::ValueType<OutputType, compute_stage>::type & adNodalValue() const;

  const DoFValue & nodalVectorTagValue(TagID tag) const;
  const DoFValue & nodalMatrixTagValue(TagID tag) const;

  /**
   * Set nodal value
   */
  void setNodalValue(const OutputType & value, unsigned int idx = 0);
  /**
   * Set local DOF values and evaluate the values on quadrature points
   */
  void setDofValues(const DenseVector<OutputData> & value);
  /**
   * Write a nodal value to the passed-in solution vector
   */
  void insertNodalValue(NumericVector<Number> & residual, const OutputData & v);
  OutputData getNodalValue(const Node & node, Moose::SolutionState state) const;
  OutputData
  getElementalValue(const Elem * elem, Moose::SolutionState state, unsigned int idx = 0) const;

  ///////////////////////////// dof indices ///////////////////////////////////////////////

  void getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices) const;
  const std::vector<dof_id_type> & dofIndices() const { return _dof_indices; }
  unsigned int numberOfDofs() const { return _dof_indices.size(); }
  void clearDofIndices() { _dof_indices.clear(); }

  /**
   * Get the dof indices corresponding to the current element
   */
  void prepare();

  /**
   * setter of _has_dof_values
   * @param has_dof_values The boolean value we are setting _has_dof_values to
   */
  void hasDofValues(bool has_dof_values) { _has_dof_values = has_dof_values; }

  /**
   * Prepare degrees of freedom for the current node
   */
  void reinitNode();

  /**
   * Prepare dof indices and solution values for elemental auxiliary variables
   */
  void prepareAux();

  /**
   * Prepare dof indices and solution values for elemental auxiliary variables
   */
  void reinitAux();

  /**
   * Set _dof_indices to the degrees of freedom existing on the passed-in nodes
   * @param nodes The nodes to grab the degrees of freedom for
   */
  void reinitNodes(const std::vector<dof_id_type> & nodes);

  /**
   * Set the current local DOF values to the input vector
   */
  void insert(NumericVector<Number> & residual);
  /**
   * Add the current local DOF values to the input vector
   */
  void add(NumericVector<Number> & residual);
  /**
   * Add passed in local DOF values to a solution vector
   */
  void addSolution(NumericVector<Number> & sol, const DenseVector<Number> & v) const;

  /////////////////////////// DoF value getters /////////////////////////////////////

  const DoFValue & dofValues() const;
  const DoFValue & dofValuesOld() const;
  const DoFValue & dofValuesOlder() const;
  const DoFValue & dofValuesPreviousNL() const;
  const DoFValue & dofValuesDot() const;
  const DoFValue & dofValuesDotOld() const;
  const DoFValue & dofValuesDotDot() const;
  const DoFValue & dofValuesDotDotOld() const;
  const MooseArray<Number> & dofValuesDuDotDu() const;
  const MooseArray<Number> & dofValuesDuDotDotDu() const;

  /**
   * Return the AD dof values
   */
  template <ComputeStage compute_stage>
  const MooseArray<typename Moose::RealType<compute_stage>::type> & adDofValues() const;

  /////////////////////////////// Increment stuff ///////////////////////////////////////

  /**
   * Increment getter
   * @return The increment
   */
  const FieldVariableValue & increment() const { return _increment; }

  /**
   * Compute and store incremental change in solution at QPs based on increment_vec
   */
  void computeIncrementAtQps(const NumericVector<Number> & increment_vec);

  /**
   * Compute and store incremental change at the current node based on increment_vec
   */
  void computeIncrementAtNode(const NumericVector<Number> & increment_vec);

  /////////////////////////////// Tags ///////////////////////////////////////////////////

  const FieldVariableValue & vectorTagValue(TagID tag)
  {
    _need_vector_tag_u[tag] = true;
    return _vector_tag_u[tag];
  }
  const FieldVariableValue & matrixTagValue(TagID tag)
  {
    _need_matrix_tag_u[tag] = true;
    return _matrix_tag_u[tag];
  }

private:
  /**
   * Helper methods for assigning nodal values from their corresponding solution values (dof
   * values as they're referred to here in this class). These methods are only truly meaningful
   * for nodal basis families
   */
  void assignADNodalValue(const DualReal & value, const unsigned int & component);
  void assignNodalValue();
  void fetchDoFValues();
  void fetchADDoFValues();
  void zeroSizeDofValues();
  inline void getArrayDoFValues(const NumericVector<Number> & sol,
                                unsigned int n,
                                MooseArray<RealEigenVector> & dof_values) const;

  /// A const reference to the owning MooseVariableFE object
  const MooseVariableFE<OutputType> & _var;

  const FEType & _fe_type;

  const unsigned int _var_num;

  const SystemBase & _sys;

  const SubProblem & _subproblem;

  THREAD_ID _tid;

  const Assembly & _assembly;

  const DofMap & _dof_map;

  /// The element type this object is storing data for. This is either Element, Neighbor, or Lower
  Moose::ElementType _element_type;

  /// Number of components of the associated variable
  unsigned int _count;

  /// The dof indices for the current element
  std::vector<dof_id_type> _dof_indices;

  mutable std::vector<bool> _need_vector_tag_dof_u;
  mutable std::vector<bool> _need_matrix_tag_dof_u;

  /// if variable is nodal
  bool _is_nodal;

  /// The dof index for the current node
  dof_id_type _nodal_dof_index;

  // Dof values of tagged vectors
  std::vector<DoFValue> _vector_tags_dof_u;
  // Dof values of the diagonal of tagged matrices
  std::vector<DoFValue> _matrix_tags_dof_u;

  std::vector<FieldVariableValue> _vector_tag_u;
  mutable std::vector<bool> _need_vector_tag_u;
  std::vector<FieldVariableValue> _matrix_tag_u;
  mutable std::vector<bool> _need_matrix_tag_u;

  /// Continuity type of the variable
  FEContinuity _continuity;

  /// Increment in the variable used in dampers
  FieldVariableValue _increment;

  /// Nodal values
  OutputType _nodal_value;
  OutputType _nodal_value_old;
  OutputType _nodal_value_older;
  OutputType _nodal_value_previous_nl;

  /// nodal values of u_dot
  OutputType _nodal_value_dot;
  /// nodal values of u_dotdot
  OutputType _nodal_value_dotdot;
  /// nodal values of u_dot_old
  OutputType _nodal_value_dot_old;
  /// nodal values of u_dotdot_old
  OutputType _nodal_value_dotdot_old;

  /// Nodal values as MooseArrays for use with AuxKernels
  MooseArray<OutputType> _nodal_value_array;
  MooseArray<OutputType> _nodal_value_old_array;
  MooseArray<OutputType> _nodal_value_older_array;

  /// AD nodal value
  typename Moose::ValueType<OutputType, JACOBIAN>::type _ad_nodal_value;

  /// A zero AD variable
  const DualReal _ad_zero;

  /// u flags
  mutable bool _need_u_old;
  mutable bool _need_u_older;
  mutable bool _need_u_previous_nl;

  /// u dot flags
  mutable bool _need_u_dot;
  mutable bool _need_ad_u_dot;
  mutable bool _need_u_dotdot;
  mutable bool _need_u_dot_old;
  mutable bool _need_u_dotdot_old;
  mutable bool _need_du_dot_du;
  mutable bool _need_du_dotdot_du;

  /// gradient flags
  mutable bool _need_grad_old;
  mutable bool _need_grad_older;
  mutable bool _need_grad_previous_nl;

  /// gradient dot flags
  mutable bool _need_grad_dot;
  mutable bool _need_grad_dotdot;

  /// SolutionState second_u flags
  mutable bool _need_second;
  mutable bool _need_second_old;
  mutable bool _need_second_older;
  mutable bool _need_second_previous_nl;

  /// curl flags
  mutable bool _need_curl;
  mutable bool _need_curl_old;
  mutable bool _need_curl_older;

  /// AD flags
  mutable bool _need_ad;
  mutable bool _need_ad_u;
  mutable bool _need_ad_grad_u;
  mutable bool _need_ad_second_u;

  /// local solution flags
  mutable bool _need_dof_values;
  mutable bool _need_dof_values_old;
  mutable bool _need_dof_values_older;
  mutable bool _need_dof_values_previous_nl;
  mutable bool _need_dof_values_dot;
  mutable bool _need_dof_values_dotdot;
  mutable bool _need_dof_values_dot_old;
  mutable bool _need_dof_values_dotdot_old;
  mutable bool _need_dof_du_dot_du;
  mutable bool _need_dof_du_dotdot_du;

  bool _has_dof_indices;
  bool _has_dof_values;

  /// local solution values
  DoFValue _dof_values;
  DoFValue _dof_values_old;
  DoFValue _dof_values_older;
  DoFValue _dof_values_previous_nl;

  /// nodal values of u_dot
  DoFValue _dof_values_dot;
  /// nodal values of u_dotdot
  DoFValue _dof_values_dotdot;
  /// nodal values of u_dot_old
  DoFValue _dof_values_dot_old;
  /// nodal values of u_dotdot_old
  DoFValue _dof_values_dotdot_old;
  /// nodal values of derivative of u_dot wrt u
  MooseArray<Number> _dof_du_dot_du;
  /// nodal values of derivative of u_dotdot wrt u
  MooseArray<Number> _dof_du_dotdot_du;

  /// u
  FieldVariableValue _u;
  FieldVariableValue _u_old;
  FieldVariableValue _u_older;
  FieldVariableValue _u_previous_nl;

  /// grad_u
  FieldVariableGradient _grad_u;
  FieldVariableGradient _grad_u_old;
  FieldVariableGradient _grad_u_older;
  FieldVariableGradient _grad_u_previous_nl;

  /// grad_u dots
  FieldVariableGradient _grad_u_dot;
  FieldVariableGradient _grad_u_dotdot;

  /// second_u
  FieldVariableSecond _second_u;
  FieldVariableSecond _second_u_old;
  FieldVariableSecond _second_u_older;
  FieldVariableSecond _second_u_previous_nl;

  /// curl_u
  FieldVariableCurl _curl_u;
  FieldVariableCurl _curl_u_old;
  FieldVariableCurl _curl_u_older;

  /// AD u
  typename VariableValueType<OutputShape, JACOBIAN>::type _ad_u;
  typename VariableGradientType<OutputShape, JACOBIAN>::type _ad_grad_u;
  typename VariableSecondType<OutputShape, JACOBIAN>::type _ad_second_u;
  MooseArray<DualReal> _ad_dof_values;
  MooseArray<DualReal> _ad_dofs_dot;
  typename VariableValueType<OutputShape, JACOBIAN>::type _ad_u_dot;

  // time derivatives

  /// u_dot (time derivative)
  FieldVariableValue _u_dot;

  /// u_dotdot (second time derivative)
  FieldVariableValue _u_dotdot, _u_dotdot_bak;

  /// u_dot_old (time derivative)
  FieldVariableValue _u_dot_old, _u_dot_old_bak;

  /// u_dotdot_old (second time derivative)
  FieldVariableValue _u_dotdot_old, _u_dotdot_old_bak;

  /// derivative of u_dot wrt u
  VariableValue _du_dot_du;

  /// derivative of u_dotdot wrt u
  VariableValue _du_dotdot_du, _du_dotdot_du_bak;

  /// The current qrule. This has to be a reference because the current qrule will be constantly
  /// changing. If we initialized this to point to one qrule, then in the next calculation we would
  /// be pointing to the wrong place!
  const QBase * const & _qrule;
  const QBase * const & _qrule_face;

  // Shape function values, gradients, second derivatives
  const FieldVariablePhiValue * _phi;
  const FieldVariablePhiGradient * _grad_phi;
  mutable const FieldVariablePhiSecond * _second_phi;
  mutable const FieldVariablePhiCurl * _curl_phi;

  // Mapped array phi
  MappedArrayVariablePhiGradient _mapped_grad_phi;
  MappedArrayVariablePhiGradient _mapped_grad_phi_face;
  MappedArrayVariablePhiGradient _mapped_grad_phi_neighbor;
  MappedArrayVariablePhiGradient _mapped_grad_phi_face_neighbor;

  // Values, gradients and second derivatives of shape function on faces
  const FieldVariablePhiValue * _phi_face;
  const FieldVariablePhiGradient * _grad_phi_face;
  mutable const FieldVariablePhiSecond * _second_phi_face;
  mutable const FieldVariablePhiCurl * _curl_phi_face;

  const typename VariableTestGradientType<OutputShape, JACOBIAN>::type * _ad_grad_phi;
  const typename VariableTestGradientType<OutputShape, JACOBIAN>::type * _ad_grad_phi_face;

  const QBase * _current_qrule;
  const FieldVariablePhiValue * _current_phi;
  const FieldVariablePhiGradient * _current_grad_phi;
  const FieldVariablePhiSecond * _current_second_phi;
  const FieldVariablePhiCurl * _current_curl_phi;
  const typename VariableTestGradientType<OutputShape, JACOBIAN>::type * _current_ad_grad_phi;

  std::function<const typename OutputTools<OutputType>::VariablePhiValue &(const Assembly &,
                                                                           FEType)>
      _phi_assembly_method;
  std::function<const typename OutputTools<OutputType>::VariablePhiValue &(const Assembly &,
                                                                           FEType)>
      _phi_face_assembly_method;

  std::function<const typename OutputTools<OutputType>::VariablePhiGradient &(const Assembly &,
                                                                              FEType)>
      _grad_phi_assembly_method;
  std::function<const typename OutputTools<OutputType>::VariablePhiGradient &(const Assembly &,
                                                                              FEType)>
      _grad_phi_face_assembly_method;

  std::function<const typename OutputTools<OutputType>::VariablePhiSecond &(const Assembly &,
                                                                            FEType)>
      _second_phi_assembly_method;
  std::function<const typename OutputTools<OutputType>::VariablePhiSecond &(const Assembly &,
                                                                            FEType)>
      _second_phi_face_assembly_method;

  std::function<const typename OutputTools<OutputType>::VariablePhiCurl &(const Assembly &, FEType)>
      _curl_phi_assembly_method;
  std::function<const typename OutputTools<OutputType>::VariablePhiCurl &(const Assembly &, FEType)>
      _curl_phi_face_assembly_method;

  std::function<const typename VariableTestGradientType<OutputType, ComputeStage::JACOBIAN>::type &(
      const Assembly &, FEType)>
      _ad_grad_phi_assembly_method;
  std::function<const typename VariableTestGradientType<OutputType, ComputeStage::JACOBIAN>::type &(
      const Assembly &, FEType)>
      _ad_grad_phi_face_assembly_method;

  /// Pointer to time integrator
  const TimeIntegrator * _time_integrator;

  /// The current node. This has to be a reference because the current node will be constantly
  /// changing. If we initialized this to point to one node, then in the next calculation we would
  /// be pointing to the wrong place!
  const Node * const & _node;

  /// The current elem. This has to be a reference because the current elem will be constantly
  /// changing. If we initialized this to point to one elem, then in the next calculation we would
  /// be pointing to the wrong place!
  const Elem * const & _elem;

  /// Whether this variable is being calculated on a displaced system
  const bool _displaced;

  /// The current element side
  const unsigned int & _current_side;
};

/////////////////////// General template definitions //////////////////////////////////////

template <typename OutputType>
template <ComputeStage compute_stage>
const MooseArray<typename Moose::RealType<compute_stage>::type> &
MooseVariableData<OutputType>::adDofValues() const
{
  _need_ad = true;
  return _ad_dof_values;
}

template <typename OutputType>
template <ComputeStage compute_stage>
const typename Moose::ValueType<OutputType, compute_stage>::type &
MooseVariableData<OutputType>::adNodalValue() const
{
  _need_ad = true;
  return _ad_nodal_value;
}

////////////////////////// Forward declaration of fully specialized templates //////////////////

template <>
void MooseVariableData<RealEigenVector>::fetchDoFValues();

template <>
template <>
const VariableValue & MooseVariableData<Real>::adSln<RESIDUAL>() const;

template <>
template <>
const VariableGradient & MooseVariableData<Real>::adGradSln<RESIDUAL>() const;

template <>
template <>
const VariableSecond & MooseVariableData<Real>::adSecondSln<RESIDUAL>() const;

template <>
template <>
const VariableValue & MooseVariableData<Real>::adUDot<RESIDUAL>() const;

template <>
template <>
const VectorVariableValue & MooseVariableData<RealVectorValue>::adSln<RESIDUAL>() const;

template <>
template <>
const VectorVariableGradient & MooseVariableData<RealVectorValue>::adGradSln<RESIDUAL>() const;

template <>
template <>
const VectorVariableSecond & MooseVariableData<RealVectorValue>::adSecondSln<RESIDUAL>() const;

template <>
template <>
const VectorVariableValue & MooseVariableData<RealVectorValue>::adUDot<RESIDUAL>() const;

template <>
template <>
const MooseArray<Real> & MooseVariableData<Real>::adDofValues<RESIDUAL>() const;

template <>
template <>
const MooseArray<Real> & MooseVariableData<RealVectorValue>::adDofValues<RESIDUAL>() const;

template <>
template <>
const Real & MooseVariableData<Real>::adNodalValue<RESIDUAL>() const;

template <>
template <>
const RealVectorValue & MooseVariableData<RealVectorValue>::adNodalValue<RESIDUAL>() const;

////////////////////// Definitions of fully specialized templates (must be inlined) //////////

template <>
template <>
inline const typename VariableTestGradientType<Real, RESIDUAL>::type &
MooseVariableData<Real>::adGradPhi<RESIDUAL>() const
{
  return *_grad_phi;
}

template <>
template <>
inline const typename VariableTestGradientType<RealVectorValue, RESIDUAL>::type &
MooseVariableData<RealVectorValue>::adGradPhi<RESIDUAL>() const
{
  return *_grad_phi;
}

template <>
template <>
inline const typename VariableTestGradientType<Real, RESIDUAL>::type &
MooseVariableData<Real>::adGradPhiFace<RESIDUAL>() const
{
  return *_grad_phi_face;
}

template <>
template <>
inline const typename VariableTestGradientType<RealVectorValue, RESIDUAL>::type &
MooseVariableData<RealVectorValue>::adGradPhiFace<RESIDUAL>() const
{
  return *_grad_phi_face;
}
