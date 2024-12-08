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
#include "MooseVariableFieldBase.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseVariableData.h"
#include "MooseFunctor.h"
#include "MeshChangedInterface.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dense_vector.h"

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
class MooseVariableField : public MooseVariableFieldBase,
                           public Moose::FunctorBase<typename Moose::ADType<OutputType>::type>,
                           public MeshChangedInterface
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

  MooseVariableField(const InputParameters & parameters);

  virtual Moose::VarFieldType fieldType() const override;
  virtual bool isArray() const override;
  virtual bool isVector() const override;

  static InputParameters validParams();

  virtual void setNodalValue(const OutputType & value, unsigned int idx = 0) = 0;

  ///@{
  /**
   * Degree of freedom value setters
   */
  virtual void setDofValue(const OutputData & value, unsigned int index) = 0;
  ///@}

  /**
   * AD solution getter
   */
  virtual const ADTemplateVariableValue<OutputType> & adSln() const = 0;

  /**
   * AD neighbor solution getter
   */
  virtual const ADTemplateVariableValue<OutputType> & adSlnNeighbor() const = 0;

  /**
   * AD grad solution getter
   */
  virtual const ADTemplateVariableGradient<OutputType> & adGradSln() const = 0;

  /**
   * AD grad of time derivative solution getter
   */
  virtual const ADTemplateVariableGradient<OutputType> & adGradSlnDot() const = 0;

  /**
   * AD grad neighbor solution getter
   */
  virtual const ADTemplateVariableGradient<OutputType> & adGradSlnNeighbor() const = 0;

  /**
   * AD grad of time derivative neighbor solution getter
   */
  virtual const ADTemplateVariableGradient<OutputType> & adGradSlnNeighborDot() const = 0;

  /**
   * AD second solution getter
   */
  virtual const ADTemplateVariableSecond<OutputType> & adSecondSln() const = 0;

  /**
   * AD second neighbor solution getter
   */
  virtual const ADTemplateVariableSecond<OutputType> & adSecondSlnNeighbor() const = 0;

  /**
   * AD time derivative getter
   */
  virtual const ADTemplateVariableValue<OutputType> & adUDot() const = 0;

  /**
   * AD second time derivative getter
   */
  virtual const ADTemplateVariableValue<OutputType> & adUDotDot() const = 0;

  /**
   * AD neighbor time derivative getter
   */
  virtual const ADTemplateVariableValue<OutputType> & adUDotNeighbor() const = 0;

  /**
   * AD neighbor second time derivative getter
   */
  virtual const ADTemplateVariableValue<OutputType> & adUDotDotNeighbor() const = 0;

  /**
   * Return the AD dof values
   */
  virtual const MooseArray<ADReal> & adDofValues() const = 0;

  /**
   * Return the AD neighbor dof values
   */
  virtual const MooseArray<ADReal> & adDofValuesNeighbor() const = 0;

  /**
   * Return the AD time derivatives at dofs
   */
  virtual const MooseArray<ADReal> & adDofValuesDot() const = 0;

  ///@{
  /**
   * Methods for retrieving values of variables at the nodes in a MooseArray for AuxKernelBase
   */
  virtual const MooseArray<OutputType> & nodalValueArray() const = 0;
  virtual const MooseArray<OutputType> & nodalValueOldArray() const = 0;
  virtual const MooseArray<OutputType> & nodalValueOlderArray() const = 0;
  ///@}

  /**
   * @return the current elemental solution
   */
  virtual const FieldVariableValue & sln() const = 0;

  /**
   * @return the old elemental solution, e.g. that of the previous timestep
   */
  virtual const FieldVariableValue & slnOld() const = 0;

  /**
   * @return the current neighbor solution
   */
  virtual const FieldVariableValue & slnNeighbor() const = 0;

  /**
   * @return the old neighbor solution, e.g. that of the previous timestep
   */
  virtual const FieldVariableValue & slnOldNeighbor() const = 0;

  /**
   * @return the older elemental solution, e.g. that of two timesteps ago
   */
  virtual const FieldVariableValue & slnOlder() const = 0;

  /// element gradients
  virtual const FieldVariableGradient & gradSln() const = 0;
  virtual const FieldVariableGradient & gradSlnOld() const = 0;

  /// neighbor solution gradients
  virtual const FieldVariableGradient & gradSlnNeighbor() const = 0;
  virtual const FieldVariableGradient & gradSlnOldNeighbor() const = 0;

  /**
   * Whether or not this variable is computing any second derivatives.
   */
  virtual bool computingSecond() const = 0;

  /**
   * Whether or not this variable is computing any curl quantities
   */
  virtual bool computingCurl() const = 0;

  /**
   * Whether or not this variable is computing any divergence quantities
   */
  virtual bool computingDiv() const = 0;

  /**
   * Return the variable's elemental shape functions
   */
  virtual const FieldVariablePhiValue & phi() const = 0;

  /**
   * Return the gradients of the variable's elemental shape functions
   */
  virtual const FieldVariablePhiGradient & gradPhi() const = 0;

  /**
   * Return the rank-2 tensor of second derivatives of the variable's elemental shape functions
   */
  virtual const FieldVariablePhiSecond & secondPhi() const = 0;

  /**
   * Curl of the shape functions
   */
  virtual const FieldVariablePhiValue & curlPhi() const = 0;

  /**
   * Divergence of the shape functions
   */
  virtual const FieldVariablePhiDivergence & divPhi() const = 0;

  /**
   * Return the variable's shape functions on an element face
   */
  virtual const FieldVariablePhiValue & phiFace() const = 0;

  /**
   * Return the gradients of the variable's shape functions on an element face
   */
  virtual const FieldVariablePhiGradient & gradPhiFace() const = 0;

  /**
   * Return the rank-2 tensor of second derivatives of the variable's shape functions on an element
   * face
   */
  virtual const FieldVariablePhiSecond & secondPhiFace() const = 0;

  /**
   * Return the variable's shape functions on a neighboring element face
   */
  virtual const FieldVariablePhiValue & phiFaceNeighbor() const = 0;

  /**
   * Return the gradients of the variable's shape functions on a neighboring element face
   */
  virtual const FieldVariablePhiGradient & gradPhiFaceNeighbor() const = 0;

  /**
   * Return the rank-2 tensor of second derivatives of the variable's shape functions on a
   * neighboring element face
   */
  virtual const FieldVariablePhiSecond & secondPhiFaceNeighbor() const = 0;

  /**
   * Return the variable's shape functions on a neighboring element
   */
  virtual const FieldVariablePhiValue & phiNeighbor() const = 0;

  /**
   * Return the gradients of the variable's shape functions on a neighboring element
   */
  virtual const FieldVariablePhiGradient & gradPhiNeighbor() const = 0;

  /**
   * Return the rank-2 tensor of second derivatives of the variable's shape functions on a
   * neighboring element
   */
  virtual const FieldVariablePhiSecond & secondPhiNeighbor() const = 0;

  /**
   * Return the variable's shape functions on a lower-dimensional element
   */
  virtual const FieldVariablePhiValue & phiLower() const = 0;

  /**
   * Set local DOF values and evaluate the values on quadrature points
   */
  virtual void setDofValues(const DenseVector<OutputData> & values) = 0;

  /**
   * Set local DOF values for a lower dimensional element and evaluate the values on quadrature
   * points
   */
  virtual void setLowerDofValues(const DenseVector<OutputData> & values) = 0;

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
   * Whether or not this variable is actually using the shape function second derivatives.
   */
  virtual bool usesSecondPhiNeighbor() const = 0;

  ///@{
  /**
   * dof values getters
   */
  virtual const DoFValue & dofValues() const = 0;
  virtual const DoFValue & dofValuesOld() const = 0;
  virtual const DoFValue & dofValuesOlder() const = 0;
  virtual const DoFValue & dofValuesPreviousNL() const = 0;
  virtual const DoFValue & dofValuesNeighbor() const = 0;
  virtual const DoFValue & dofValuesOldNeighbor() const = 0;
  virtual const DoFValue & dofValuesOlderNeighbor() const = 0;
  virtual const DoFValue & dofValuesPreviousNLNeighbor() const = 0;
  virtual const DoFValue & dofValuesDot() const = 0;
  virtual const DoFValue & dofValuesDotNeighbor() const = 0;
  virtual const DoFValue & dofValuesDotOld() const = 0;
  virtual const DoFValue & dofValuesDotOldNeighbor() const = 0;
  virtual const DoFValue & dofValuesDotDot() const = 0;
  virtual const DoFValue & dofValuesDotDotNeighbor() const = 0;
  virtual const DoFValue & dofValuesDotDotOld() const = 0;
  virtual const DoFValue & dofValuesDotDotOldNeighbor() const = 0;
  virtual const MooseArray<libMesh::Number> & dofValuesDuDotDu() const = 0;
  virtual const MooseArray<libMesh::Number> & dofValuesDuDotDuNeighbor() const = 0;
  virtual const MooseArray<libMesh::Number> & dofValuesDuDotDotDu() const = 0;
  virtual const MooseArray<libMesh::Number> & dofValuesDuDotDotDuNeighbor() const = 0;

  /**
   * tag values getters
   */
  virtual const FieldVariableValue & vectorTagValue(TagID tag) const = 0;
  virtual const DoFValue & nodalVectorTagValue(TagID tag) const = 0;
  virtual const DoFValue & vectorTagDofValue(TagID tag) const = 0;
  virtual const DoFValue & nodalMatrixTagValue(TagID tag) const = 0;
  virtual const FieldVariableValue & matrixTagValue(TagID tag) const = 0;

  virtual void residualSetup() override;
  virtual void jacobianSetup() override;
  virtual void timestepSetup() override;

  using MooseVariableFieldBase::hasBlocks;
  /*
   * Returns whether a variable is defined on a block as a functor.
   * This makes the link between functor block restriction and the
   * BlockRestrictable interface.
   * @param id subdomain id we want to know whether the variable is defined on
   * @return whether the variable is defined on this domain
   */
  bool hasBlocks(const SubdomainID id) const override { return BlockRestrictable::hasBlocks(id); }

protected:
  /**
   * Get the solution corresponding to the provided state
   */
  const libMesh::NumericVector<libMesh::Number> & getSolution(const Moose::StateArg & state) const;

  /// the time integrator used for computing time derivatives
  const TimeIntegrator * const _time_integrator;

  /// A dummy ADReal variable
  mutable ADReal _ad_real_dummy = 0;
};

#define usingMooseVariableFieldMembers                                                             \
  usingMooseVariableFieldBaseMembers;                                                              \
  using MooseVariableField<OutputType>::_time_integrator;                                          \
  using MooseVariableField<OutputType>::_ad_real_dummy;                                            \
  using MooseVariableField<OutputType>::getSolution

// Prevent implicit instantiation in other translation units where these classes are used
extern template class MooseVariableField<Real>;
extern template class MooseVariableField<RealVectorValue>;
extern template class MooseVariableField<RealEigenVector>;
