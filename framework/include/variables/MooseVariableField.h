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

  MooseVariableField(const InputParameters & parameters);

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
  virtual const MooseArray<Number> & dofValuesDuDotDu() const = 0;
  virtual const MooseArray<Number> & dofValuesDuDotDuNeighbor() const = 0;
  virtual const MooseArray<Number> & dofValuesDuDotDotDu() const = 0;
  virtual const MooseArray<Number> & dofValuesDuDotDotDuNeighbor() const = 0;

  /**
   * tag values getters
   */
  virtual const FieldVariableValue & vectorTagValue(TagID tag) const = 0;
  virtual const DoFValue & nodalVectorTagValue(TagID tag) const = 0;
  virtual const DoFValue & vectorTagDofValue(TagID tag) const = 0;

  virtual void meshChanged() override;
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
  using FunctorArg = typename Moose::ADType<OutputType>::type;
  using Moose::FunctorBase<FunctorArg>::evaluate;
  using Moose::FunctorBase<FunctorArg>::evaluateGradient;
  using Moose::FunctorBase<FunctorArg>::evaluateDot;
  using typename Moose::FunctorBase<FunctorArg>::ValueType;
  using typename Moose::FunctorBase<FunctorArg>::DotType;
  using typename Moose::FunctorBase<FunctorArg>::GradientType;

  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using ElemPointArg = Moose::ElemPointArg;
  using StateArg = Moose::StateArg;

  ValueType evaluate(const ElemQpArg & elem_qp, const StateArg & state) const override final;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp,
                     const StateArg & state) const override final;
  ValueType evaluate(const ElemPointArg & elem_point, const StateArg & state) const override final;

  GradientType evaluateGradient(const ElemQpArg & elem_qp, const StateArg & state) const override;
  GradientType evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                const StateArg & state) const override final;

  DotType evaluateDot(const ElemQpArg & elem_qp, const StateArg & state) const override final;
  DotType evaluateDot(const ElemSideQpArg & elem_side_qp,
                      const StateArg & state) const override final;

  /// the time integrator used for computing time derivatives
  const TimeIntegrator * const _time_integrator;

  /// A dummy ADReal variable
  mutable ADReal _ad_real_dummy = 0;

private:
  /**
   * Compute the solution, gradient, and time derivative with provided shape functions
   */
  template <typename Shapes, typename Solution, typename GradShapes, typename GradSolution>
  void computeSolution(const Elem * elem,
                       const QBase *,
                       const StateArg & state,
                       const Shapes & phi,
                       Solution & local_soln,
                       const GradShapes & grad_phi,
                       GradSolution & grad_local_soln,
                       Solution & dot_local_soln) const;

  /**
   * Evaluate solution and gradient for the \p elem_qp argument
   */
  void evaluateOnElement(const ElemQpArg & elem_qp, const StateArg & state) const;

  /**
   * Evaluate solution and gradient for the \p elem_side_qp argument
   */
  void evaluateOnElementSide(const ElemSideQpArg & elem_side_qp, const StateArg & state) const;

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
};
