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
#include "MooseVariableField.h"
#include "MooseVariableData.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/enum_fe_family.h"

class TimeIntegrator;
template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<RealVectorValue> VectorMooseVariable;
typedef MooseVariableFE<RealEigenVector> ArrayMooseVariable;

/**
 * Class for stuff related to variables
 *
 * Each variable can compute nodal or elemental (at QPs) values.
 *
 * OutputType          OutputShape           OutputData
 * ----------------------------------------------------
 * Real                Real                  Real
 * RealVectorValue     RealVectorValue       Real
 * RealEigenVector     Real                  RealEigenVector
 *
 */
template <typename OutputType>
class MooseVariableFE : public MooseVariableField<OutputType>
{
public:
  using OutputGradient = typename MooseVariableField<OutputType>::OutputGradient;
  using OutputSecond = typename MooseVariableField<OutputType>::OutputSecond;
  using OutputDivergence = typename MooseVariableField<OutputType>::OutputDivergence;

  using FieldVariableValue = typename MooseVariableField<OutputType>::FieldVariableValue;
  using FieldVariableGradient = typename MooseVariableField<OutputType>::FieldVariableGradient;
  using FieldVariableSecond = typename MooseVariableField<OutputType>::FieldVariableSecond;
  using FieldVariableCurl = typename MooseVariableField<OutputType>::FieldVariableCurl;
  using FieldVariableDivergence = typename MooseVariableField<OutputType>::FieldVariableDivergence;

  using OutputShape = typename MooseVariableField<OutputType>::OutputShape;
  using OutputShapeGradient = typename MooseVariableField<OutputType>::OutputShapeGradient;
  using OutputShapeSecond = typename MooseVariableField<OutputType>::OutputShapeSecond;
  using OutputShapeDivergence = typename MooseVariableField<OutputType>::OutputShapeDivergence;

  using FieldVariablePhiValue = typename MooseVariableField<OutputType>::FieldVariablePhiValue;
  using FieldVariablePhiGradient =
      typename MooseVariableField<OutputType>::FieldVariablePhiGradient;
  using FieldVariablePhiSecond = typename MooseVariableField<OutputType>::FieldVariablePhiSecond;
  using FieldVariablePhiCurl = typename MooseVariableField<OutputType>::FieldVariablePhiCurl;
  using FieldVariablePhiDivergence =
      typename MooseVariableField<OutputType>::FieldVariablePhiDivergence;

  using FieldVariableTestValue = typename MooseVariableField<OutputType>::FieldVariableTestValue;
  using FieldVariableTestGradient =
      typename MooseVariableField<OutputType>::FieldVariableTestGradient;
  using FieldVariableTestSecond = typename MooseVariableField<OutputType>::FieldVariableTestSecond;
  using FieldVariableTestCurl = typename MooseVariableField<OutputType>::FieldVariableTestCurl;
  using FieldVariableTestDivergence =
      typename MooseVariableField<OutputType>::FieldVariableTestDivergence;

  using OutputData = typename MooseVariableField<OutputType>::OutputData;
  using DoFValue = typename MooseVariableField<OutputType>::DoFValue;

  using FunctorArg = typename Moose::ADType<OutputType>::type;
  using typename Moose::FunctorBase<FunctorArg>::ValueType;
  using typename Moose::FunctorBase<FunctorArg>::GradientType;
  using typename Moose::FunctorBase<FunctorArg>::DotType;

  MooseVariableFE(const InputParameters & parameters);

  static InputParameters validParams();

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
   * Whether or not this variable is computing any second derivatives.
   */
  bool usesSecondPhi() const;

  /**
   * Whether or not this variable is actually using the shape function second derivative on a
   * neighbor.
   */
  bool usesSecondPhiNeighbor() const override final;

  /**
   * Whether or not this variable is computing any second derivatives.
   */
  bool computingSecond() const override final { return usesSecondPhi(); }

  /**
   * Whether or not this variable is computing the curl
   */
  bool computingCurl() const override final;

  /**
   * Whether or not this variable is computing the divergence
   */
  bool computingDiv() const override final;

  bool isNodal() const override { return _element_data->isNodal(); }
  bool hasDoFsOnNodes() const override { return _element_data->hasDoFsOnNodes(); }
  libMesh::FEContinuity getContinuity() const override { return _element_data->getContinuity(); };
  const Node * const & node() const { return _element_data->node(); }
  const dof_id_type & nodalDofIndex() const override { return _element_data->nodalDofIndex(); }
  virtual bool isNodalDefined() const override;

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

  virtual void getDofIndices(const Elem * elem,
                             std::vector<dof_id_type> & dof_indices) const override;
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

  void clearAllDofIndices() final;

  unsigned int numberOfDofsNeighbor() override { return _neighbor_data->dofIndices().size(); }

  const FieldVariablePhiValue & phi() const override { return _element_data->phi(); }
  const FieldVariablePhiGradient & gradPhi() const override final
  {
    return _element_data->gradPhi();
  }
  const MappedArrayVariablePhiGradient & arrayGradPhi() const
  {
    return _element_data->arrayGradPhi();
  }
  const FieldVariablePhiSecond & secondPhi() const override final;
  const FieldVariablePhiCurl & curlPhi() const override final;
  const FieldVariablePhiDivergence & divPhi() const override final;

  const FieldVariablePhiValue & phiFace() const override final { return _element_data->phiFace(); }
  const FieldVariablePhiGradient & gradPhiFace() const override final
  {
    return _element_data->gradPhiFace();
  }
  const MappedArrayVariablePhiGradient & arrayGradPhiFace() const
  {
    return _element_data->arrayGradPhiFace();
  }
  const FieldVariablePhiSecond & secondPhiFace() const override final;
  const FieldVariablePhiCurl & curlPhiFace() const;
  const FieldVariablePhiDivergence & divPhiFace() const;

  const FieldVariablePhiValue & phiNeighbor() const override final { return _neighbor_data->phi(); }
  const FieldVariablePhiGradient & gradPhiNeighbor() const override final
  {
    return _neighbor_data->gradPhi();
  }
  const MappedArrayVariablePhiGradient & arrayGradPhiNeighbor() const
  {
    return _neighbor_data->arrayGradPhi();
  }
  const FieldVariablePhiSecond & secondPhiNeighbor() const override final;
  const FieldVariablePhiCurl & curlPhiNeighbor() const;
  const FieldVariablePhiDivergence & divPhiNeighbor() const;

  const FieldVariablePhiValue & phiFaceNeighbor() const override final
  {
    return _neighbor_data->phiFace();
  }
  const FieldVariablePhiGradient & gradPhiFaceNeighbor() const override final
  {
    return _neighbor_data->gradPhiFace();
  }
  const MappedArrayVariablePhiGradient & arrayGradPhiFaceNeighbor() const
  {
    return _neighbor_data->arrayGradPhiFace();
  }
  const FieldVariablePhiSecond & secondPhiFaceNeighbor() const override final;
  const FieldVariablePhiCurl & curlPhiFaceNeighbor() const;
  const FieldVariablePhiDivergence & divPhiFaceNeighbor() const;

  virtual const FieldVariablePhiValue & phiLower() const override { return _lower_data->phi(); }
  const FieldVariablePhiGradient & gradPhiLower() const { return _lower_data->gradPhi(); }

  const ADTemplateVariableTestGradient<OutputShape> & adGradPhi() const
  {
    return _element_data->adGradPhi();
  }

  const ADTemplateVariableTestGradient<OutputShape> & adGradPhiFace() const
  {
    return _element_data->adGradPhiFace();
  }

  const ADTemplateVariableTestGradient<OutputShape> & adGradPhiFaceNeighbor() const
  {
    return _neighbor_data->adGradPhiFace();
  }

  // damping
  const FieldVariableValue & increment() const { return _element_data->increment(); }

  const FieldVariableValue & vectorTagValue(TagID tag) const override
  {
    return _element_data->vectorTagValue(tag);
  }
  const FieldVariableGradient & vectorTagGradient(TagID tag) const
  {
    return _element_data->vectorTagGradient(tag);
  }
  const DoFValue & vectorTagDofValue(TagID tag) const override
  {
    return _element_data->vectorTagDofValue(tag);
  }
  const FieldVariableValue & matrixTagValue(TagID tag) const override
  {
    return _element_data->matrixTagValue(tag);
  }

  /// element solutions
  const FieldVariableValue & sln() const override { return _element_data->sln(Moose::Current); }
  const FieldVariableValue & slnOld() const override { return _element_data->sln(Moose::Old); }
  const FieldVariableValue & slnOlder() const override { return _element_data->sln(Moose::Older); }
  const FieldVariableValue & slnPreviousNL() const { return _element_data->sln(Moose::PreviousNL); }

  /// element gradients
  const FieldVariableGradient & gradSln() const override
  {
    return _element_data->gradSln(Moose::Current);
  }
  const FieldVariableGradient & gradSlnOld() const override
  {
    return _element_data->gradSln(Moose::Old);
  }
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

  /// element divergence
  const FieldVariableDivergence & divSln() const { return _element_data->divSln(Moose::Current); }
  const FieldVariableDivergence & divSlnOld() const { return _element_data->divSln(Moose::Old); }
  const FieldVariableDivergence & divSlnOlder() const
  {
    return _element_data->divSln(Moose::Older);
  }

  /// AD
  const ADTemplateVariableValue<OutputType> & adSln() const override
  {
    return _element_data->adSln();
  }

  const ADTemplateVariableGradient<OutputType> & adGradSln() const override
  {
    return _element_data->adGradSln();
  }
  const ADTemplateVariableSecond<OutputType> & adSecondSln() const override
  {
    return _element_data->adSecondSln();
  }
  const ADTemplateVariableValue<OutputType> & adUDot() const override
  {
    return _element_data->adUDot();
  }
  const ADTemplateVariableValue<OutputType> & adUDotDot() const override
  {
    return _element_data->adUDotDot();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnDot() const override
  {
    return _element_data->adGradSlnDot();
  }

  /// neighbor AD
  const ADTemplateVariableValue<OutputType> & adSlnNeighbor() const override
  {
    return _neighbor_data->adSln();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnNeighbor() const override
  {
    return _neighbor_data->adGradSln();
  }
  const ADTemplateVariableSecond<OutputType> & adSecondSlnNeighbor() const override
  {
    return _neighbor_data->adSecondSln();
  }
  const ADTemplateVariableValue<OutputType> & adUDotNeighbor() const override
  {
    return _neighbor_data->adUDot();
  }
  const ADTemplateVariableValue<OutputType> & adUDotDotNeighbor() const override
  {
    return _neighbor_data->adUDotDot();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnNeighborDot() const override
  {
    return _neighbor_data->adGradSlnDot();
  }

  /// element dots
  const FieldVariableValue & uDot() const { return _element_data->uDot(); }
  const FieldVariableValue & uDotDot() const { return _element_data->uDotDot(); }
  const FieldVariableValue & uDotOld() const { return _element_data->uDotOld(); }
  const FieldVariableValue & uDotDotOld() const { return _element_data->uDotDotOld(); }
  const VariableValue & duDotDu() const { return _element_data->duDotDu(); }
  const VariableValue & duDotDotDu() const { return _element_data->duDotDotDu(); }

  /// neighbor solutions
  const FieldVariableValue & slnNeighbor() const override
  {
    return _neighbor_data->sln(Moose::Current);
  }
  const FieldVariableValue & slnOldNeighbor() const override
  {
    return _neighbor_data->sln(Moose::Old);
  }
  const FieldVariableValue & slnOlderNeighbor() const { return _neighbor_data->sln(Moose::Older); }
  const FieldVariableValue & slnPreviousNLNeighbor() const
  {
    return _neighbor_data->sln(Moose::PreviousNL);
  }

  /// neighbor solution gradients
  const FieldVariableGradient & gradSlnNeighbor() const override
  {
    return _neighbor_data->gradSln(Moose::Current);
  }
  const FieldVariableGradient & gradSlnOldNeighbor() const override
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

  /// neighbor solution divergence
  const FieldVariableDivergence & divSlnNeighbor() const
  {
    return _neighbor_data->divSln(Moose::Current);
  }
  const FieldVariableDivergence & divSlnOldNeighbor() const
  {
    return _neighbor_data->divSln(Moose::Old);
  }
  const FieldVariableDivergence & divSlnOlderNeighbor() const
  {
    return _neighbor_data->divSln(Moose::Older);
  }

  /// neighbor dots
  const FieldVariableValue & uDotNeighbor() const { return _neighbor_data->uDot(); }
  const FieldVariableValue & uDotDotNeighbor() const { return _neighbor_data->uDotDot(); }
  const FieldVariableValue & uDotOldNeighbor() const { return _neighbor_data->uDotOld(); }
  const FieldVariableValue & uDotDotOldNeighbor() const { return _neighbor_data->uDotDotOld(); }
  const VariableValue & duDotDuNeighbor() const { return _neighbor_data->duDotDu(); }
  const VariableValue & duDotDotDuNeighbor() const { return _neighbor_data->duDotDotDu(); }

  /// lower-d element solution
  const ADTemplateVariableValue<OutputType> & adSlnLower() const { return _lower_data->adSln(); }
  const FieldVariableValue & slnLower() const { return _lower_data->sln(Moose::Current); }
  const FieldVariableValue & slnLowerOld() const { return _lower_data->sln(Moose::Old); }

  /// Actually compute variable values from the solution vectors
  virtual void computeElemValues() override;
  virtual void computeElemValuesFace() override;
  virtual void computeNeighborValuesFace() override;
  virtual void computeNeighborValues() override;
  virtual void computeLowerDValues() override;

  virtual void setNodalValue(const OutputType & value, unsigned int idx = 0) override;

  virtual void setDofValue(const OutputData & value, unsigned int index) override;

  /**
   * Set local DOF values and evaluate the values on quadrature points
   */
  virtual void setDofValues(const DenseVector<OutputData> & values) override;
  virtual void setLowerDofValues(const DenseVector<OutputData> & values) override;

  /**
   * Write a nodal value to the passed-in solution vector
   */
  void insertNodalValue(libMesh::NumericVector<libMesh::Number> & residual, const OutputData & v);

  /**
   * Get the value of this variable at given node
   */
  OutputData getNodalValue(const Node & node) const;
  /**
   * Get the old value of this variable at given node
   */
  OutputData getNodalValueOld(const Node & node) const;
  /**
   * Get the t-2 value of this variable at given node
   */
  OutputData getNodalValueOlder(const Node & node) const;
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
  virtual void insert(libMesh::NumericVector<libMesh::Number> & vector) override;
  virtual void insertLower(libMesh::NumericVector<libMesh::Number> & vector) override;

  /**
   * Add the current local DOF values to the input vector
   */
  virtual void add(libMesh::NumericVector<libMesh::Number> & vector) override;

  /**
   * Add passed in local DOF values onto the current solution
   */
  void addSolution(const DenseVector<libMesh::Number> & v);

  /**
   * Add passed in local neighbor DOF values onto the current solution
   */
  void addSolutionNeighbor(const DenseVector<libMesh::Number> & v);

  const DoFValue & dofValue() const;
  const DoFValue & dofValues() const override;
  const DoFValue & dofValuesOld() const override;
  const DoFValue & dofValuesOlder() const override;
  const DoFValue & dofValuesPreviousNL() const override;
  const DoFValue & dofValuesNeighbor() const override;
  const DoFValue & dofValuesOldNeighbor() const override;
  const DoFValue & dofValuesOlderNeighbor() const override;
  const DoFValue & dofValuesPreviousNLNeighbor() const override;
  const DoFValue & dofValuesDot() const override;
  const DoFValue & dofValuesDotNeighbor() const override;
  const DoFValue & dofValuesDotNeighborResidual() const;
  const DoFValue & dofValuesDotOld() const override;
  const DoFValue & dofValuesDotOldNeighbor() const override;
  const DoFValue & dofValuesDotDot() const override;
  const DoFValue & dofValuesDotDotNeighbor() const override;
  const DoFValue & dofValuesDotDotNeighborResidual() const;
  const DoFValue & dofValuesDotDotOld() const override;
  const DoFValue & dofValuesDotDotOldNeighbor() const override;
  const MooseArray<libMesh::Number> & dofValuesDuDotDu() const override;
  const MooseArray<libMesh::Number> & dofValuesDuDotDuNeighbor() const override;
  const MooseArray<libMesh::Number> & dofValuesDuDotDotDu() const override;
  const MooseArray<libMesh::Number> & dofValuesDuDotDotDuNeighbor() const override;

  const MooseArray<ADReal> & adDofValues() const override;
  const MooseArray<ADReal> & adDofValuesNeighbor() const override;
  const MooseArray<ADReal> & adDofValuesDot() const override;

  /**
   * Compute and store incremental change in solution at QPs based on increment_vec
   */
  void computeIncrementAtQps(const libMesh::NumericVector<libMesh::Number> & increment_vec);

  /**
   * Compute and store incremental change at the current node based on increment_vec
   */
  void computeIncrementAtNode(const libMesh::NumericVector<libMesh::Number> & increment_vec);

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
  virtual std::size_t phiSize() const final { return _element_data->phiSize(); }
  /**
   * Return phiFace size
   */
  virtual std::size_t phiFaceSize() const final { return _element_data->phiFaceSize(); }
  /**
   * Return phiNeighbor size
   */
  virtual std::size_t phiNeighborSize() const final { return _neighbor_data->phiSize(); }
  /**
   * Return phiFaceNeighbor size
   */
  virtual std::size_t phiFaceNeighborSize() const final { return _neighbor_data->phiFaceSize(); }

  std::size_t phiLowerSize() const final { return _lower_data->phiSize(); }

  /**
   * Methods for retrieving values of variables at the nodes
   */
  const OutputType & nodalValue() const;
  const OutputType & nodalValueOld() const;
  const OutputType & nodalValueOlder() const;
  const OutputType & nodalValuePreviousNL() const;
  const OutputType & nodalValueDot() const;
  const OutputType & nodalValueDotDot() const;
  const OutputType & nodalValueDotOld() const;
  const OutputType & nodalValueDotDotOld() const;
  const OutputType & nodalValueDuDotDu() const;
  const OutputType & nodalValueDuDotDotDu() const;
  const OutputType & nodalValueNeighbor() const;
  const OutputType & nodalValueOldNeighbor() const;
  const OutputType & nodalValueOlderNeighbor() const;
  const OutputType & nodalValuePreviousNLNeighbor() const;
  const OutputType & nodalValueDotNeighbor() const;
  const OutputType & nodalValueDotNeighborResidual() const;
  const OutputType & nodalValueDotDotNeighbor() const;
  const OutputType & nodalValueDotDotNeighborResidual() const;
  const OutputType & nodalValueDotOldNeighbor() const;
  const OutputType & nodalValueDotDotOldNeighbor() const;
  const OutputType & nodalValueDuDotDuNeighbor() const;
  const OutputType & nodalValueDuDotDotDuNeighbor() const;

  const MooseArray<OutputType> & nodalValueArray() const override
  {
    return _element_data->nodalValueArray(Moose::Current);
  }
  const MooseArray<OutputType> & nodalValueOldArray() const override
  {
    return _element_data->nodalValueArray(Moose::Old);
  }
  const MooseArray<OutputType> & nodalValueOlderArray() const override
  {
    return _element_data->nodalValueArray(Moose::Older);
  }

  const DoFValue & nodalVectorTagValue(TagID tag) const override;
  const DoFValue & nodalMatrixTagValue(TagID tag) const override;

  const typename Moose::ADType<OutputType>::type & adNodalValue() const;

  virtual void computeNodalValues() override;
  virtual void computeNodalNeighborValues() override;

  unsigned int oldestSolutionStateRequested() const override final;

  void setActiveTags(const std::set<TagID> & vtags) override;

  virtual void meshChanged() override;
  virtual void residualSetup() override;
  virtual void jacobianSetup() override;

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return true; }

protected:
  usingMooseVariableFieldMembers;

  /// Holder for all the data associated with the "main" element
  std::unique_ptr<MooseVariableData<OutputType>> _element_data;

  /// Holder for all the data associated with the neighbor element
  std::unique_ptr<MooseVariableData<OutputType>> _neighbor_data;

  /// Holder for all the data associated with the lower dimeensional element
  std::unique_ptr<MooseVariableData<OutputType>> _lower_data;

  using MooseVariableField<OutputType>::evaluate;
  using MooseVariableField<OutputType>::evaluateGradient;
  using MooseVariableField<OutputType>::evaluateDot;
  using MooseVariableField<OutputType>::evaluateGradDot;
  using ElemArg = Moose::ElemArg;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using FaceArg = Moose::FaceArg;
  using StateArg = Moose::StateArg;
  using NodeArg = Moose::NodeArg;
  using ElemPointArg = Moose::ElemPointArg;

  /**
   * A common method that both evaluate(FaceArg) and evaluateDot(FaceArg) can call. A value
   * evaluation vs dot evaluation is delineated via the passed-in \p cache_data, e.g. if the
   * passed-in cache data is the sln data member then this will return a value evaluation and if the
   * cache data is the dot data member then this will return a dot evaluation
   */
  ValueType
  faceEvaluate(const FaceArg &, const StateArg &, const std::vector<ValueType> & cache_data) const;

  ValueType evaluate(const ElemQpArg & elem_qp, const StateArg & state) const override final;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp,
                     const StateArg & state) const override final;
  ValueType evaluate(const ElemArg &, const StateArg &) const override final;
  ValueType evaluate(const ElemPointArg &, const StateArg &) const override final;
  ValueType evaluate(const NodeArg & node_arg, const StateArg & state) const override final;
  ValueType evaluate(const FaceArg &, const StateArg &) const override final;

  GradientType evaluateGradient(const ElemQpArg & elem_qp, const StateArg & state) const override;
  GradientType evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                const StateArg & state) const override final;
  GradientType evaluateGradient(const ElemArg &, const StateArg &) const override final;

  DotType evaluateDot(const ElemQpArg & elem_qp, const StateArg & state) const override final;
  DotType evaluateDot(const ElemSideQpArg & elem_side_qp,
                      const StateArg & state) const override final;
  DotType evaluateDot(const ElemArg &, const StateArg &) const override final;
  DotType evaluateDot(const FaceArg &, const StateArg &) const override final;

  GradientType evaluateGradDot(const ElemArg &, const StateArg &) const override final;

private:
  /**
   * Compute the solution, gradient, time derivative, and gradient of the time derivative with
   * provided shape functions
   */
  template <typename Shapes, typename Solution, typename GradShapes, typename GradSolution>
  void computeSolution(const Elem * elem,
                       unsigned int n_qp,
                       const StateArg & state,
                       const Shapes & phi,
                       Solution & local_soln,
                       const GradShapes & grad_phi,
                       GradSolution & grad_local_soln,
                       Solution & dot_local_soln,
                       GradSolution & grad_dot_local_soln) const;

  /**
   * Evaluate solution and gradient for the \p elem_qp argument
   */
  void
  evaluateOnElement(const ElemQpArg & elem_qp, const StateArg & state, bool cache_eligible) const;

  /**
   * Evaluate solution and gradient for the \p elem_side_qp argument
   */
  void evaluateOnElementSide(const ElemSideQpArg & elem_side_qp,
                             const StateArg & state,
                             bool cache_eligible) const;

  /// Keep track of the current elem-qp functor element in order to enable local caching (e.g. if we
  /// call evaluate on the same element, but just with a different quadrature point, we can return
  /// previously computed results indexed at the different qp
  mutable const Elem * _current_elem_qp_functor_elem = nullptr;

  /// The values of the solution for the \p _current_elem_qp_functor_elem
  mutable std::vector<ValueType> _current_elem_qp_functor_sln;

  /// The values of the gradient for the \p _current_elem_qp_functor_elem
  mutable std::vector<GradientType> _current_elem_qp_functor_gradient;

  /// The values of the time derivative for the \p _current_elem_qp_functor_elem
  mutable std::vector<DotType> _current_elem_qp_functor_dot;

  /// The values of the gradient of the time derivative for the \p _current_elem_qp_functor_elem
  mutable std::vector<GradientType> _current_elem_qp_functor_grad_dot;

  /// Keep track of the current elem-side-qp functor element and side in order to enable local
  /// caching (e.g. if we call evaluate with the same element and side, but just with a different
  /// quadrature point, we can return previously computed results indexed at the different qp
  mutable std::pair<const Elem *, unsigned int> _current_elem_side_qp_functor_elem_side{
      nullptr, libMesh::invalid_uint};

  /// The values of the solution for the \p _current_elem_side_qp_functor_elem_side
  mutable std::vector<ValueType> _current_elem_side_qp_functor_sln;

  /// The values of the gradient for the \p _current_elem_side_qp_functor_elem_side
  mutable std::vector<GradientType> _current_elem_side_qp_functor_gradient;

  /// The values of the time derivative for the \p _current_elem_side_qp_functor_elem_side
  mutable std::vector<DotType> _current_elem_side_qp_functor_dot;

  /// The values of the gradient of the time derivative for the \p
  /// _current_elem_side_qp_functor_elem_side
  mutable std::vector<GradientType> _current_elem_side_qp_functor_grad_dot;
};

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFE<OutputType>::adDofValues() const
{
  return _element_data->adDofValues();
}

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFE<OutputType>::adDofValuesNeighbor() const
{
  return _neighbor_data->adDofValues();
}

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFE<OutputType>::adDofValuesDot() const
{
  return _element_data->adDofValuesDot();
}

template <typename OutputType>
inline const typename Moose::ADType<OutputType>::type &
MooseVariableFE<OutputType>::adNodalValue() const
{
  return _element_data->adNodalValue();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setActiveTags(const std::set<TagID> & vtags)
{
  _element_data->setActiveTags(vtags);
  _neighbor_data->setActiveTags(vtags);
  _lower_data->setActiveTags(vtags);
}

// Declare all the specializations, as the template specialization declarations below must know
template <>
InputParameters MooseVariableFE<Real>::validParams();
template <>
InputParameters MooseVariableFE<RealVectorValue>::validParams();
template <>
InputParameters MooseVariableFE<RealEigenVector>::validParams();
template <>
RealEigenVector
MooseVariableFE<RealEigenVector>::getValue(const Elem * elem,
                                           const std::vector<std::vector<Real>> & phi) const;
template <>
RealVectorArrayValue MooseVariableFE<RealEigenVector>::getGradient(
    const Elem * elem, const std::vector<std::vector<RealVectorValue>> & grad_phi) const;
template <>
void MooseVariableFE<RealEigenVector>::evaluateOnElement(const ElemQpArg &,
                                                         const StateArg &,
                                                         bool) const;
template <>
void MooseVariableFE<RealEigenVector>::evaluateOnElementSide(const ElemSideQpArg &,
                                                             const StateArg &,
                                                             bool) const;
template <>
typename MooseVariableFE<RealEigenVector>::ValueType
MooseVariableFE<RealEigenVector>::evaluate(const ElemQpArg &, const StateArg &) const;
template <>
typename MooseVariableFE<RealEigenVector>::ValueType
MooseVariableFE<RealEigenVector>::evaluate(const ElemSideQpArg &, const StateArg &) const;
template <>
typename MooseVariableFE<RealEigenVector>::GradientType
MooseVariableFE<RealEigenVector>::evaluateGradient(const ElemQpArg &, const StateArg &) const;
template <>
typename MooseVariableFE<RealEigenVector>::GradientType
MooseVariableFE<RealEigenVector>::evaluateGradient(const ElemSideQpArg &, const StateArg &) const;
template <>
typename MooseVariableFE<RealEigenVector>::DotType
MooseVariableFE<RealEigenVector>::evaluateDot(const ElemQpArg &, const StateArg &) const;
template <>
typename MooseVariableFE<RealEigenVector>::DotType
MooseVariableFE<RealEigenVector>::evaluateDot(const ElemSideQpArg &, const StateArg &) const;

// Prevent implicit instantiation in other translation units where these classes are used
extern template class MooseVariableFE<Real>;
extern template class MooseVariableFE<RealVectorValue>;
extern template class MooseVariableFE<RealEigenVector>;
