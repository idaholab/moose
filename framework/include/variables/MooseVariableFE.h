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
#include "MooseVariableFEBase.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseVariableData.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dense_vector.h"

class TimeIntegrator;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<RealVectorValue> VectorMooseVariable;
typedef MooseVariableFE<RealEigenVector> ArrayMooseVariable;

template <>
InputParameters validParams<MooseVariable>();
template <>
InputParameters validParams<VectorMooseVariable>();
template <>
InputParameters validParams<ArrayMooseVariable>();

/**
 * Class for stuff related to variables
 *
 * Each variable can compute nodal or elemental (at QPs) values.
 *
 * OutputType          OutputShape           OutputData
 * ----------------------------------------------------
 * Real                Real                  Real
 * RealVectorValue     RealVectorValue       Real
 * RealEigenVector      Real                  RealEigenVector
 *
 */
template <typename OutputType>
class MooseVariableFE : public MooseVariableFEBase
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

  MooseVariableFE(const InputParameters & parameters);

  void clearDofIndices() override;

  void prepare() override;
  void prepareNeighbor() override;
  void prepareLowerD() override;
  virtual void prepareIC() override;

  void prepareAux() override;

  void reinitNode() override;
  void reinitAux() override;
  void reinitAuxNeighbor() override;

  void reinitNodes(const std::vector<dof_id_type> & nodes) override;
  void reinitNodesNeighbor(const std::vector<dof_id_type> & nodes) override;

  /**
   * Whether or not this variable is actually using the shape function value.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesPhi() const { return true; }
  /**
   * Whether or not this variable is actually using the shape function gradient.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesGradPhi() const { return true; }
  /**
   * Whether or not this variable is actually using the shape function value.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesPhiNeighbor() const { return true; }
  /**
   * Whether or not this variable is actually using the shape function gradient.
   *
   * Currently hardcoded to true because we always compute the value.
   */
  bool usesGradPhiNeighbor() const { return true; }
  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool usesSecondPhi() const;

  /**
   * Whether or not this variable is actually using the shape function second derivative on a
   * neighbor.
   */
  bool usesSecondPhiNeighbor() const;

  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool computingSecond() const { return usesSecondPhi(); }

  /**
   * Whether or not this variable is computing the curl
   */
  bool computingCurl() const;

  const std::set<SubdomainID> & activeSubdomains() const override;
  bool activeOnSubdomain(SubdomainID subdomain) const override;

  bool isNodal() const override { return _element_data->isNodal(); }
  Moose::VarFieldType fieldType() const override;
  bool isVector() const override;
  const Node * const & node() const { return _element_data->node(); }
  const dof_id_type & nodalDofIndex() const override { return _element_data->nodalDofIndex(); }
  bool isNodalDefined() const;

  const Node * const & nodeNeighbor() const { return _neighbor_data->node(); }
  const dof_id_type & nodalDofIndexNeighbor() const override
  {
    return _neighbor_data->nodalDofIndex();
  }
  bool isNodalNeighborDefined() const;

  const Elem * const & currentElem() const override { return _element_data->currentElem(); }

  /**
   * Current side this variable is being evaluated on
   */
  const unsigned int & currentSide() const { return _element_data->currentSide(); }

  /**
   * Current neighboring element
   */
  const Elem * const & neighbor() const { return _neighbor_data->currentElem(); }

  void getDofIndices(const Elem * elem, std::vector<dof_id_type> & dof_indices) const override;
  const std::vector<dof_id_type> & dofIndices() const final { return _element_data->dofIndices(); }
  unsigned int numberOfDofs() const final { return _element_data->numberOfDofs(); }
  const std::vector<dof_id_type> & dofIndicesNeighbor() const final
  {
    return _neighbor_data->dofIndices();
  }
  const std::vector<dof_id_type> & dofIndicesLower() const final
  {
    return _lower_data->dofIndices();
  }

  unsigned int numberOfDofsNeighbor() override { return _neighbor_data->dofIndices().size(); }

  const FieldVariablePhiValue & phi() const { return _element_data->phi(); }
  const FieldVariablePhiGradient & gradPhi() const { return _element_data->gradPhi(); }
  const MappedArrayVariablePhiGradient & arrayGradPhi() const
  {
    return _element_data->arrayGradPhi();
  }
  const FieldVariablePhiSecond & secondPhi() const;
  const FieldVariablePhiCurl & curlPhi() const;

  const FieldVariablePhiValue & phiFace() const { return _element_data->phiFace(); }
  const FieldVariablePhiGradient & gradPhiFace() const { return _element_data->gradPhiFace(); }
  const MappedArrayVariablePhiGradient & arrayGradPhiFace() const
  {
    return _element_data->arrayGradPhiFace();
  }
  const FieldVariablePhiSecond & secondPhiFace() const;
  const FieldVariablePhiCurl & curlPhiFace() const;

  const FieldVariablePhiValue & phiNeighbor() const { return _neighbor_data->phi(); }
  const FieldVariablePhiGradient & gradPhiNeighbor() const { return _neighbor_data->gradPhi(); }
  const MappedArrayVariablePhiGradient & arrayGradPhiNeighbor() const
  {
    return _neighbor_data->arrayGradPhi();
  }
  const FieldVariablePhiSecond & secondPhiNeighbor() const;
  const FieldVariablePhiCurl & curlPhiNeighbor() const;

  const FieldVariablePhiValue & phiFaceNeighbor() const { return _neighbor_data->phiFace(); }
  const FieldVariablePhiGradient & gradPhiFaceNeighbor() const
  {
    return _neighbor_data->gradPhiFace();
  }
  const MappedArrayVariablePhiGradient & arrayGradPhiFaceNeighbor() const
  {
    return _neighbor_data->arrayGradPhiFace();
  }
  const FieldVariablePhiSecond & secondPhiFaceNeighbor() const;
  const FieldVariablePhiCurl & curlPhiFaceNeighbor() const;

  const FieldVariablePhiValue & phiLower() const { return _lower_data->phi(); }
  const FieldVariablePhiGradient & gradPhiLower() const { return _lower_data->gradPhi(); }

  template <ComputeStage compute_stage>
  const typename VariableTestGradientType<OutputType, compute_stage>::type & adGradPhi()
  {
    return _element_data->template adGradPhi<compute_stage>();
  }

  template <ComputeStage compute_stage>
  const typename VariableTestGradientType<OutputType, compute_stage>::type & adGradPhiFace()
  {
    return _element_data->template adGradPhiFace<compute_stage>();
  }

  // damping
  const FieldVariableValue & increment() const { return _element_data->increment(); }

  const FieldVariableValue & vectorTagValue(TagID tag)
  {
    return _element_data->vectorTagValue(tag);
  }
  const FieldVariableValue & matrixTagValue(TagID tag)
  {
    return _element_data->matrixTagValue(tag);
  }

  /// element solutions
  const FieldVariableValue & sln() const { return _element_data->sln(Moose::Current); }
  const FieldVariableValue & slnOld() const { return _element_data->sln(Moose::Old); }
  const FieldVariableValue & slnOlder() const { return _element_data->sln(Moose::Older); }
  const FieldVariableValue & slnPreviousNL() const { return _element_data->sln(Moose::PreviousNL); }

  /// element gradients
  const FieldVariableGradient & gradSln() const { return _element_data->gradSln(Moose::Current); }
  const FieldVariableGradient & gradSlnOld() const { return _element_data->gradSln(Moose::Old); }
  const FieldVariableGradient & gradSlnOlder() const
  {
    return _element_data->gradSln(Moose::Older);
  }
  const FieldVariableGradient & gradSlnPreviousNL() const
  {
    return _element_data->gradSln(Moose::PreviousNL);
  }

  /// element gradient dots
  const FieldVariableGradient & gradSlnDot() const { return _element_data->gradSlnDot(); }
  const FieldVariableGradient & gradSlnDotDot() const { return _element_data->gradSlnDotDot(); }

  /// element seconds
  const FieldVariableSecond & secondSln() const { return _element_data->secondSln(Moose::Current); }
  const FieldVariableSecond & secondSlnOld() const { return _element_data->secondSln(Moose::Old); }
  const FieldVariableSecond & secondSlnOlder() const
  {
    return _element_data->secondSln(Moose::Older);
  }
  const FieldVariableSecond & secondSlnPreviousNL() const
  {
    return _element_data->secondSln(Moose::PreviousNL);
  }

  /// element curls
  const FieldVariableCurl & curlSln() const { return _element_data->curlSln(Moose::Current); }
  const FieldVariableCurl & curlSlnOld() const { return _element_data->curlSln(Moose::Old); }
  const FieldVariableCurl & curlSlnOlder() const { return _element_data->curlSln(Moose::Older); }

  /// AD
  template <ComputeStage compute_stage>
  const typename VariableValueType<OutputType, compute_stage>::type & adSln() const
  {
    return _element_data->template adSln<compute_stage>();
  }
  template <ComputeStage compute_stage>
  const typename VariableGradientType<OutputType, compute_stage>::type & adGradSln() const
  {
    return _element_data->template adGradSln<compute_stage>();
  }
  template <ComputeStage compute_stage>
  const typename VariableSecondType<OutputType, compute_stage>::type & adSecondSln() const
  {
    return _element_data->template adSecondSln<compute_stage>();
  }
  template <ComputeStage compute_stage>
  const typename VariableValueType<OutputType, compute_stage>::type & adUDot() const
  {
    return _element_data->template adUDot<compute_stage>();
  }

  /// neighbor AD
  template <ComputeStage compute_stage>
  const typename VariableValueType<OutputType, compute_stage>::type & adSlnNeighbor() const
  {
    return _neighbor_data->template adSln<compute_stage>();
  }
  template <ComputeStage compute_stage>
  const typename VariableGradientType<OutputType, compute_stage>::type & adGradSlnNeighbor() const
  {
    return _neighbor_data->template adGradSln<compute_stage>();
  }
  template <ComputeStage compute_stage>
  const typename VariableSecondType<OutputType, compute_stage>::type & adSecondSlnNeighbor() const
  {
    return _neighbor_data->template adSecondSln<compute_stage>();
  }
  template <ComputeStage compute_stage>
  const typename VariableValueType<OutputType, compute_stage>::type & adUDotNeighbor() const
  {
    return _neighbor_data->template adUDot<compute_stage>();
  }

  /// element dots
  const FieldVariableValue & uDot() const { return _element_data->uDot(); }
  const FieldVariableValue & uDotDot() const { return _element_data->uDotDot(); }
  const FieldVariableValue & uDotOld() const { return _element_data->uDotOld(); }
  const FieldVariableValue & uDotDotOld() const { return _element_data->uDotDotOld(); }
  const VariableValue & duDotDu() const { return _element_data->duDotDu(); }
  const VariableValue & duDotDotDu() const { return _element_data->duDotDotDu(); }

  /// neighbor solutions
  const FieldVariableValue & slnNeighbor() const { return _neighbor_data->sln(Moose::Current); }
  const FieldVariableValue & slnOldNeighbor() const { return _neighbor_data->sln(Moose::Old); }
  const FieldVariableValue & slnOlderNeighbor() const { return _neighbor_data->sln(Moose::Older); }
  const FieldVariableValue & slnPreviousNLNeighbor() const
  {
    return _neighbor_data->sln(Moose::PreviousNL);
  }

  /// neighbor solution gradients
  const FieldVariableGradient & gradSlnNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::Current);
  }
  const FieldVariableGradient & gradSlnOldNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::Old);
  }
  const FieldVariableGradient & gradSlnOlderNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::Older);
  }
  const FieldVariableGradient & gradSlnPreviousNLNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::PreviousNL);
  }

  /// neighbor grad dots
  const FieldVariableGradient & gradSlnNeighborDot() const { return _neighbor_data->gradSlnDot(); }
  const FieldVariableGradient & gradSlnNeighborDotDot() const
  {
    return _neighbor_data->gradSlnDotDot();
  }

  /// neighbor solution seconds
  const FieldVariableSecond & secondSlnNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::Current);
  }
  const FieldVariableSecond & secondSlnOldNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::Old);
  }
  const FieldVariableSecond & secondSlnOlderNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::Older);
  }
  const FieldVariableSecond & secondSlnPreviousNLNeighbor() const
  {
    return _neighbor_data->secondSln(Moose::PreviousNL);
  }

  /// neighbor solution curls
  const FieldVariableCurl & curlSlnNeighbor() const
  {
    return _neighbor_data->curlSln(Moose::Current);
  }
  const FieldVariableCurl & curlSlnOldNeighbor() const
  {
    return _neighbor_data->curlSln(Moose::Old);
  }
  const FieldVariableCurl & curlSlnOlderNeighbor() const
  {
    return _neighbor_data->curlSln(Moose::Older);
  }

  /// neighbor dots
  const FieldVariableValue & uDotNeighbor() const { return _neighbor_data->uDot(); }
  const FieldVariableValue & uDotDotNeighbor() const { return _neighbor_data->uDotDot(); }
  const FieldVariableValue & uDotOldNeighbor() const { return _neighbor_data->uDotOld(); }
  const FieldVariableValue & uDotDotOldNeighbor() const { return _neighbor_data->uDotDotOld(); }
  const VariableValue & duDotDuNeighbor() const { return _neighbor_data->duDotDu(); }
  const VariableValue & duDotDotDuNeighbor() const { return _neighbor_data->duDotDotDu(); }

  /// lower-d element solution
  template <ComputeStage compute_stage>
  const typename VariableValueType<OutputType, compute_stage>::type & adSlnLower() const
  {
    return _lower_data->template adSln<compute_stage>();
  }
  const FieldVariableValue & slnLower() const { return _lower_data->sln(Moose::Current); }

  /// Actually compute variable values from the solution vectors
  virtual void computeElemValues() override;
  virtual void computeElemValuesFace() override;
  virtual void computeNeighborValuesFace() override;
  virtual void computeNeighborValues() override;
  virtual void computeLowerDValues() override;

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

  /**
   * Get the value of this variable at given node
   */
  OutputData getNodalValue(const Node & node);
  /**
   * Get the old value of this variable at given node
   */
  OutputData getNodalValueOld(const Node & node);
  /**
   * Get the t-2 value of this variable at given node
   */
  OutputData getNodalValueOlder(const Node & node);
  /**
   * Get the current value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  OutputData getElementalValue(const Elem * elem, unsigned int idx = 0) const;
  /**
   * Get the old value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  OutputData getElementalValueOld(const Elem * elem, unsigned int idx = 0) const;
  /**
   * Get the older value of this variable on an element
   * @param[in] elem   Element at which to get value
   * @param[in] idx    Local index of this variable's element DoFs
   * @return Variable value
   */
  OutputData getElementalValueOlder(const Elem * elem, unsigned int idx = 0) const;
  /**
   * Set the current local DOF values to the input vector
   */
  void insert(NumericVector<Number> & residual) override;
  /**
   * Add the current local DOF values to the input vector
   */
  void add(NumericVector<Number> & residual) override;
  /**
   * Add passed in local DOF values onto the current solution
   */
  void addSolution(const DenseVector<Number> & v);
  /**
   * Add passed in local neighbor DOF values onto the current solution
   */
  void addSolutionNeighbor(const DenseVector<Number> & v);

  const DoFValue & dofValue();
  const DoFValue & dofValues();
  const DoFValue & dofValuesOld();
  const DoFValue & dofValuesOlder();
  const DoFValue & dofValuesPreviousNL();
  const DoFValue & dofValuesNeighbor();
  const DoFValue & dofValuesOldNeighbor();
  const DoFValue & dofValuesOlderNeighbor();
  const DoFValue & dofValuesPreviousNLNeighbor();
  const DoFValue & dofValuesDot();
  const DoFValue & dofValuesDotNeighbor();
  const DoFValue & dofValuesDotOld();
  const DoFValue & dofValuesDotOldNeighbor();
  const DoFValue & dofValuesDotDot();
  const DoFValue & dofValuesDotDotNeighbor();
  const DoFValue & dofValuesDotDotOld();
  const DoFValue & dofValuesDotDotOldNeighbor();
  const MooseArray<Number> & dofValuesDuDotDu();
  const MooseArray<Number> & dofValuesDuDotDuNeighbor();
  const MooseArray<Number> & dofValuesDuDotDotDu();
  const MooseArray<Number> & dofValuesDuDotDotDuNeighbor();

  /**
   * Return the AD dof values
   */
  template <ComputeStage compute_stage>
  const MooseArray<typename Moose::RealType<compute_stage>::type> & adDofValues();

  /**
   * Compute and store incremental change in solution at QPs based on increment_vec
   */
  void computeIncrementAtQps(const NumericVector<Number> & increment_vec);

  /**
   * Compute and store incremental change at the current node based on increment_vec
   */
  void computeIncrementAtNode(const NumericVector<Number> & increment_vec);

  /**
   * Compute the variable value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable value
   */
  OutputType getValue(const Elem * elem, const std::vector<std::vector<OutputShape>> & phi) const;

  /**
   * Compute the variable gradient value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable gradient value
   */
  typename OutputTools<OutputType>::OutputGradient getGradient(
      const Elem * elem,
      const std::vector<std::vector<typename OutputTools<OutputType>::OutputShapeGradient>> &
          grad_phi) const;

  /**
   * Return phi size
   */
  virtual size_t phiSize() const final { return _element_data->phiSize(); }
  /**
   * Return phiFace size
   */
  virtual size_t phiFaceSize() const final { return _element_data->phiFaceSize(); }
  /**
   * Return phiNeighbor size
   */
  virtual size_t phiNeighborSize() const final { return _neighbor_data->phiSize(); }
  /**
   * Return phiFaceNeighbor size
   */
  virtual size_t phiFaceNeighborSize() const final { return _neighbor_data->phiFaceSize(); }

  size_t phiLowerSize() const final { return _lower_data->phiSize(); }

  /**
   * Methods for retrieving values of variables at the nodes
   */
  const OutputType & nodalValue();
  const OutputType & nodalValueOld();
  const OutputType & nodalValueOlder();
  const OutputType & nodalValuePreviousNL();
  const OutputType & nodalValueDot();
  const OutputType & nodalValueDotDot();
  const OutputType & nodalValueDotOld();
  const OutputType & nodalValueDotDotOld();
  const OutputType & nodalValueDuDotDu();
  const OutputType & nodalValueDuDotDotDu();
  const OutputType & nodalValueNeighbor();
  const OutputType & nodalValueOldNeighbor();
  const OutputType & nodalValueOlderNeighbor();
  const OutputType & nodalValuePreviousNLNeighbor();
  const OutputType & nodalValueDotNeighbor();
  const OutputType & nodalValueDotDotNeighbor();
  const OutputType & nodalValueDotOldNeighbor();
  const OutputType & nodalValueDotDotOldNeighbor();
  const OutputType & nodalValueDuDotDuNeighbor();
  const OutputType & nodalValueDuDotDotDuNeighbor();

  /**
   * Methods for retrieving values of variables at the nodes in a MooseArray for AuxKernelBase
   */
  const MooseArray<OutputType> & nodalValueArray()
  {
    return _element_data->nodalValueArray(Moose::Current);
  }
  const MooseArray<OutputType> & nodalValueOldArray()
  {
    return _element_data->nodalValueArray(Moose::Old);
  }
  const MooseArray<OutputType> & nodalValueOlderArray()
  {
    return _element_data->nodalValueArray(Moose::Older);
  }

  const DoFValue & nodalVectorTagValue(TagID tag);
  const DoFValue & nodalMatrixTagValue(TagID tag);

  template <ComputeStage compute_stage>
  const typename Moose::ValueType<OutputType, compute_stage>::type & adNodalValue();

  virtual void computeNodalValues() override;
  virtual void computeNodalNeighborValues() override;

protected:
  /// Holder for all the data associated with the "main" element
  std::unique_ptr<MooseVariableData<OutputType>> _element_data;

  /// Holder for all the data associated with the neighbor element
  std::unique_ptr<MooseVariableData<OutputType>> _neighbor_data;

  /// Holder for all the data associated with the lower dimeensional element
  std::unique_ptr<MooseVariableData<OutputType>> _lower_data;
};

template <typename OutputType>
template <ComputeStage compute_stage>
inline const MooseArray<typename Moose::RealType<compute_stage>::type> &
MooseVariableFE<OutputType>::adDofValues()
{
  return _element_data->template adDofValues<compute_stage>();
}

template <typename OutputType>
template <ComputeStage compute_stage>
inline const typename Moose::ValueType<OutputType, compute_stage>::type &
MooseVariableFE<OutputType>::adNodalValue()
{
  return _element_data->template adNodalValue<compute_stage>();
}
