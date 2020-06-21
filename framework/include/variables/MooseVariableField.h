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
class MooseVariableField : public MooseVariableFieldBase
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

  virtual void setDofValue(const OutputData & value, unsigned int index) = 0;

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
   * AD grad neighbor solution getter
   */
  virtual const ADTemplateVariableGradient<OutputType> & adGradSlnNeighbor() const = 0;

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
   * AD neighbor time derivative getter
   */
  virtual const ADTemplateVariableValue<OutputType> & adUDotNeighbor() const = 0;

  /**
   * Return the AD dof values
   */
  virtual const MooseArray<ADReal> & adDofValues() const = 0;
};
