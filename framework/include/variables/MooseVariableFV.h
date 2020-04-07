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
#include "MooseVariableFVBase.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseVariableDataFV.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dense_vector.h"

template <typename>
class MooseVariableFV;

typedef MooseVariableFV<Real> MooseVariableFVReal;

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
class MooseVariableFV : public MooseVariableFVBase
{
public:
  using OutputGradient = typename MooseVariableData<OutputType>::OutputGradient;
  using OutputSecond = typename MooseVariableData<OutputType>::OutputSecond;
  using OutputDivergence = typename MooseVariableData<OutputType>::OutputDivergence;

  using FieldVariableValue = typename MooseVariableData<OutputType>::FieldVariableValue;
  using FieldVariableGradient = typename MooseVariableData<OutputType>::FieldVariableGradient;
  using FieldVariableSecond = typename MooseVariableData<OutputType>::FieldVariableSecond;
  using FieldVariableCurl = typename MooseVariableData<OutputType>::FieldVariableCurl;
  using FieldVariableDivergence = typename MooseVariableData<OutputType>::FieldVariableDivergence;

  using OutputShape = typename MooseVariableData<OutputType>::OutputShape;
  using OutputShapeGradient = typename MooseVariableData<OutputType>::OutputShapeGradient;
  using OutputShapeSecond = typename MooseVariableData<OutputType>::OutputShapeSecond;
  using OutputShapeDivergence = typename MooseVariableData<OutputType>::OutputShapeDivergence;

  using OutputData = typename MooseVariableData<OutputType>::OutputData;
  using DoFValue = typename MooseVariableData<OutputType>::DoFValue;

  static InputParameters validParams();

  MooseVariableFV(const InputParameters & parameters);

  void clearDofIndices() override;

  virtual void prepareIC() override;

  const std::set<SubdomainID> & activeSubdomains() const override;
  bool activeOnSubdomain(SubdomainID subdomain) const override;

  Moose::VarFieldType fieldType() const override;
  bool isVector() const override;

  virtual const Elem * const & currentElem() const override { return _element_data->currentElem(); }
  virtual const Elem * const & currentNeighbor() const override
  {
    return _neighbor_data->currentElem();
  }

  virtual void getDofIndices(const Elem * elem,
                             std::vector<dof_id_type> & dof_indices) const override
  {
    return _element_data->getDofIndices(elem, dof_indices);
  }

  virtual const std::vector<dof_id_type> & dofIndices() const final
  {
    return _element_data->dofIndices();
  }
  virtual const std::vector<dof_id_type> & dofIndicesNeighbor() const final
  {
    return _neighbor_data->dofIndices();
  }

  const FieldVariableValue & vectorTagValue(TagID tag)
  {
    return _element_data->vectorTagValue(tag);
  }
  const FieldVariableValue & matrixTagValue(TagID tag)
  {
    return _element_data->matrixTagValue(tag);
  }
  const FieldVariableValue & vectorTagValueNeighbor(TagID tag)
  {
    return _neighbor_data->vectorTagValue(tag);
  }
  const FieldVariableValue & matrixTagValueNeighbor(TagID tag)
  {
    return _neighbor_data->matrixTagValue(tag);
  }

  ///////////////// TODO: START of soln funcs to rewrite ////////////////////
  //

  const FieldVariableValue & uDot() const { return _element_data->uDot(); }
  const FieldVariableValue & sln() const { return _element_data->sln(Moose::Current); }
  const FieldVariableGradient & gradSln() const { return _element_data->gradSln(Moose::Current); }
  const FieldVariableValue & uDotNeighbor() const { return _neighbor_data->uDot(); }
  const FieldVariableValue & slnNeighbor() const { return _neighbor_data->sln(Moose::Current); }
  const FieldVariableGradient & gradSlnNeighbor() const
  {
    return _neighbor_data->gradSln(Moose::Current);
  }

  const VariableValue & duDotDu() const { return _element_data->duDotDu(); }
  const VariableValue & duDotDotDu() const { return _element_data->duDotDotDu(); }
  const VariableValue & duDotDuNeighbor() const { return _neighbor_data->duDotDu(); }
  const VariableValue & duDotDotDuNeighbor() const { return _neighbor_data->duDotDotDu(); }

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
  const typename VariableValueType<OutputType, compute_stage>::type & adUDotNeighbor() const
  {
    return _neighbor_data->template adUDot<compute_stage>();
  }

  ///////////////// TODO: END of soln funcs to rewrite ////////////////////

  /// Actually compute variable values from the solution vectors
  virtual void computeElemValues() override;
  virtual void computeFaceValues(const FaceInfo & fi) override;

  /**
   * Set local DOF values and evaluate the values on quadrature points
   */
  void setDofValues(const DenseVector<OutputData> & values);

  void setDofValue(const OutputData & value, unsigned int index);

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
  virtual void insert(NumericVector<Number> & residual) override;
  /**
   * Add the current local DOF values to the input vector
   */
  virtual void add(NumericVector<Number> & residual) override;
  /**
   * Add passed in local DOF values onto the current solution
   */
  void addSolution(const DenseVector<Number> & v);
  /**
   * Add passed in local neighbor DOF values onto the current solution
   */
  void addSolutionNeighbor(const DenseVector<Number> & v);

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
   * Note: const monomial is always the case - higher order solns are
   * reconstructed - so this is simpler func than FE equivalent.
   */
  OutputType getValue(const Elem * elem) const;

  /**
   * Compute the variable gradient value at a point on an element
   * @param elem The element we are computing on
   * @param phi Evaluated shape functions at a point
   * @return The variable gradient value
   */
  typename OutputTools<OutputType>::OutputGradient getGradient(const Elem * elem) const;

protected:
  /// Holder for all the data associated with the "main" element
  std::unique_ptr<MooseVariableDataFV<OutputType>> _element_data;

  /// Holder for all the data associated with the neighbor element
  std::unique_ptr<MooseVariableDataFV<OutputType>> _neighbor_data;
};

template <typename OutputType>
template <ComputeStage compute_stage>
inline const MooseArray<typename Moose::RealType<compute_stage>::type> &
MooseVariableFV<OutputType>::adDofValues()
{
  return _element_data->template adDofValues<compute_stage>();
}

