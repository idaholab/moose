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
#include "MooseVariableField.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "MooseVariableDataFV.h"

#include "libmesh/numeric_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/dense_vector.h"
#include "libmesh/enum_fe_family.h"

template <typename>
class MooseLinearVariableFV;

typedef MooseLinearVariableFV<Real> MooseLinearVariableFVReal;
class FVDirichletBCBase;
class FVFluxBC;
class LinearFVBoundaryCondition;

namespace libMesh
{
template <typename>
class NumericVector;
}

/// This class provides variable solution interface for linear
/// finite volume problems.
/// This class is designed to store gradient information when enabled.
template <typename OutputType>
class MooseLinearVariableFV : public MooseVariableField<OutputType>
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

  using OutputData = typename MooseVariableField<OutputType>::OutputData;
  using DoFValue = typename MooseVariableField<OutputType>::DoFValue;

  using FieldVariablePhiValue = typename MooseVariableField<OutputType>::FieldVariablePhiValue;
  using FieldVariablePhiGradient =
      typename MooseVariableField<OutputType>::FieldVariablePhiGradient;
  using FieldVariablePhiSecond = typename MooseVariableField<OutputType>::FieldVariablePhiSecond;
  using FieldVariablePhiDivergence =
      typename MooseVariableField<OutputType>::FieldVariablePhiDivergence;
  using ElemQpArg = Moose::ElemQpArg;
  using ElemSideQpArg = Moose::ElemSideQpArg;
  using ElemArg = Moose::ElemArg;
  using FaceArg = Moose::FaceArg;
  using StateArg = Moose::StateArg;
  using NodeArg = Moose::NodeArg;
  using ElemPointArg = Moose::ElemPointArg;
  using typename MooseVariableField<OutputType>::ValueType;
  using typename MooseVariableField<OutputType>::DotType;
  using typename MooseVariableField<OutputType>::GradientType;

  static InputParameters validParams();
  MooseLinearVariableFV(const InputParameters & parameters);

  virtual bool isFV() const override { return true; }

  virtual Moose::VarFieldType fieldType() const override;
  virtual bool isArray() const override;
  virtual bool isVector() const override;

  /**
   * Switch to request cell gradient computations.
   */
  void computeCellGradients() { _needs_cell_gradients = true; }

  /**
   * Check if cell gradient computations were requested for this variable.
   */
  bool needsCellGradients() const { return _needs_cell_gradients; }

  /**
   * Reference to the gradient container.
   */
  std::vector<std::unique_ptr<NumericVector<Number>>> & gradientContainer() { return _grad_cache; }

  virtual bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                          const Elem * elem,
                                          const Moose::StateArg & state) const override;

  /**
   * Get the variable gradient at a cell center.
   * @param elem_info The ElemInfo of the cell where we need the gradient
   */
  const VectorValue<Real> & gradSln(const ElemInfo & elem_info) const;

  /**
   * Compute interpolated gradient on the provided face.
   * @param face The face for which to retrieve the gradient.
   * @param state State argument which describes at what time / solution iteration state we want to
   * evaluate the variable
   */
  VectorValue<Real> gradSln(const FaceInfo & fi, const StateArg & state) const;

  virtual void initialSetup() override;

  /**
   * Get the solution value for the provided element and seed the derivative for the corresponding
   * dof index
   * @param elem_info The element to retrieive the solution value for
   * @param state State argument which describes at what time / solution iteration state we want to
   * evaluate the variable
   */
  Real getElemValue(const ElemInfo & elem_info, const StateArg & state) const;

  /**
   * Get the boundary condition object which corresponds to the given boundary ID
   * @param bd_id The boundary ID whose condition should be fetched
   */
  LinearFVBoundaryCondition * getBoundaryCondition(const BoundaryID bd_id) const;

protected:
  usingMooseVariableBaseMembers;

  /// Boolean to check if this variable needs gradient computations.
  bool _needs_cell_gradients;

  /// A cache for storing gradients on elements
  std::vector<std::unique_ptr<NumericVector<Number>>> _grad_cache;

  /// Temporary storage for the cell gradient to avoid unnecessary allocations.
  mutable RealVectorValue _cell_gradient;

  friend void Moose::initDofIndices<>(MooseLinearVariableFV<OutputType> &, const Elem &);

private:
  using MooseVariableField<OutputType>::evaluate;
  using MooseVariableField<OutputType>::evaluateGradient;
  using MooseVariableField<OutputType>::evaluateDot;

  virtual ValueType evaluate(const ElemArg & elem, const StateArg &) const override final;
  virtual ValueType evaluate(const FaceArg & face, const StateArg &) const override final;
  virtual ValueType evaluate(const NodeArg & node, const StateArg &) const override final;
  virtual ValueType evaluate(const ElemPointArg & elem_point,
                             const StateArg & state) const override final;
  virtual ValueType evaluate(const ElemQpArg & elem_qp,
                             const StateArg & state) const override final;
  virtual ValueType evaluate(const ElemSideQpArg & elem_side_qp,
                             const StateArg & state) const override final;
  virtual GradientType evaluateGradient(const ElemQpArg & qp_arg,
                                        const StateArg &) const override final;
  virtual GradientType evaluateGradient(const ElemArg & elem_arg,
                                        const StateArg &) const override final;
  virtual GradientType evaluateGradient(const FaceArg & face,
                                        const StateArg &) const override final;
  virtual DotType evaluateDot(const ElemArg & elem, const StateArg &) const override final;

  unsigned int oldestSolutionStateRequested() const override final { return 0; }

  /**
   * Setup the boundary to Dirichlet BC map
   */
  void cacheBoundaryBCMap();

  /// Map for easily accessing the boundary conditions based on the boundary IDs.
  /// We assume that each boundary has one boundary condition only.
  std::unordered_map<BoundaryID, LinearFVBoundaryCondition *> _boundary_id_to_bc;

public:
  /*---------------------------------------------------------------------------
   * The overridden functions below are necessary to ensure that this variable can still be used
   * with most interfaces but should not be used due to the fact that they are FEM and Newton method
   * assembly specific
   * ---------------------------------------------------------------------------*/

  virtual const std::vector<dof_id_type> & dofIndicesLower() const override final
  {
    mooseError("dofIndicesLower not supported by MooseLinearVariableFVBase");
  }

  virtual const FieldVariablePhiValue & phiLower() const override
  {
    mooseError("phiLower not supported by MooseLinearVariableFVBase");
  }

  virtual void residualSetup() override
  {
    mooseError("residualSetup not supported by MooseLinearVariableFVBase");
  }

  virtual void jacobianSetup() override
  {
    mooseError("jacobianSetup not supported by MooseLinearVariableFVBase");
  }

  void clearDofIndices() override final {}

  virtual void prepareIC() override {}

  unsigned int numberOfDofs() const override final
  {
    mooseError("numberOfDofsNeighbor not supported by MooseLinearVariableFVBase");
  }

  virtual unsigned int numberOfDofsNeighbor() override final
  {
    mooseError("numberOfDofs not supported by MooseLinearVariableFVBase");
  }

  virtual bool isNodal() const override final { return false; }

  bool hasDoFsOnNodes() const override final { return false; }

  FEContinuity getContinuity() const override final
  {
    mooseError("getContinuity not supported by MooseLinearVariableFVBase");
  };

  virtual bool isNodalDefined() const override final { return false; }

  virtual void setNodalValue(const OutputType & /*value*/, unsigned int /*idx = 0*/) override
  {
    mooseError("currentElem not supported by MooseLinearVariableFVBase");
  }

  virtual void setDofValue(const OutputData & /*value*/, unsigned int /*index*/) override
  {
    mooseError("currentElem not supported by MooseLinearVariableFVBase");
  }

  void clearAllDofIndices() final {}

  const DoFValue & nodalVectorTagValue(TagID) const override
  {
    mooseError("nodalVectorTagValue not implemented for finite volume variables.");
  }

  virtual const Elem * const & currentElem() const override
  {
    mooseError("currentElem not supported by MooseLinearVariableFVBase");
  }

  virtual void getDofIndices(const Elem * /*elem*/,
                             std::vector<dof_id_type> & /*dof_indices*/) const override
  {
    mooseError("getDofIndices not supported by MooseLinearVariableFVBase");
  }

  virtual const std::vector<dof_id_type> & dofIndices() const final
  {
    mooseError("dofIndices not supported by MooseLinearVariableFVBase");
  }
  virtual const std::vector<dof_id_type> & dofIndicesNeighbor() const final
  {
    mooseError("dofIndicesNeighbor not supported by MooseLinearVariableFVBase");
  }

  virtual void prepare() override final {}
  virtual void prepareNeighbor() override final {}
  virtual void prepareAux() override final {}
  virtual void reinitNode() override final {}
  virtual void reinitNodes(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitAux() override final {}
  virtual void reinitAuxNeighbor() override final {}
  virtual void prepareLowerD() override final {}

  virtual void computeElemValuesFace() override {}
  virtual void computeNeighborValuesFace() override {}
  virtual void computeNeighborValues() override {}
  virtual void computeLowerDValues() override final {}
  virtual void computeNodalNeighborValues() override final {}
  virtual void computeNodalValues() override final {}

  virtual void computeElemValues() override {}
  virtual void computeFaceValues(const FaceInfo & /*fi*/) override {}
  void setDofValues(const DenseVector<OutputData> & /*values*/) override {}
  virtual void setLowerDofValues(const DenseVector<OutputData> & /*values*/) override {}
  virtual void insert(NumericVector<Number> & /*residual*/) override {}
  virtual void insertLower(NumericVector<Number> & /*residual*/) override {}
  virtual void add(NumericVector<Number> & /*residual*/) override {}
  void setActiveTags(const std::set<TagID> & /*vtags*/) override
  {
    mooseError("setActiveTags is not implemented for MooseLinearVariableFV!");
  }
  const MooseArray<OutputType> & nodalValueArray() const override
  {
    mooseError("Finite volume variables do not have defined values at nodes.");
  }
  const MooseArray<OutputType> & nodalValueOldArray() const override
  {
    mooseError("Finite volume variables do not have defined values at nodes.");
  }
  const MooseArray<OutputType> & nodalValueOlderArray() const override
  {
    mooseError("Finite volume variables do not have defined values at nodes.");
  }
  bool computingSecond() const override final { return false; }
  bool computingCurl() const override final { return false; }
  bool computingDiv() const override final { return false; }
  bool usesSecondPhiNeighbor() const override final { return false; }
  const FieldVariablePhiValue & phi() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiGradient & gradPhi() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiSecond & secondPhi() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiValue & curlPhi() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiDivergence & divPhi() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiValue & phiFace() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiGradient & gradPhiFace() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiSecond & secondPhiFace() const override final
  {
    mooseError("We don't currently implement second derivatives for FV");
  }
  const FieldVariablePhiValue & phiFaceNeighbor() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiGradient & gradPhiFaceNeighbor() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiSecond & secondPhiFaceNeighbor() const override final
  {
    mooseError("We don't currently implement second derivatives for FV");
  }
  const FieldVariablePhiValue & phiNeighbor() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiGradient & gradPhiNeighbor() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariablePhiSecond & secondPhiNeighbor() const override final
  {
    mooseError("We don't currently have shape functions for linear FV variables");
  }
  const FieldVariableValue & vectorTagValue(TagID /*tag*/) const override
  {
    mooseError("vectorTagValue is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & vectorTagDofValue(TagID /*tag*/) const override
  {
    mooseError("vectorTagDofValue is not implemented for MooseLinearVariableFV!");
  }
  virtual const DoFValue & nodalMatrixTagValue(TagID /*tag*/) const override
  {
    mooseError("nodalMatrixTagValue is not implemented for MooseLinearVariableFV!");
  }
  virtual const FieldVariableValue & matrixTagValue(TagID /*tag*/) const override
  {
    mooseError("matrixTagValue is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableValue & sln() const override
  {
    mooseError("sln is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableValue & slnOld() const override
  {
    mooseError("slnOld is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableValue & slnOlder() const override
  {
    mooseError("slnOlder is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableGradient & gradSln() const override
  {
    mooseError("gradSln is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableGradient & gradSlnOld() const override
  {
    mooseError("gradSlnOld is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableValue & slnNeighbor() const override
  {
    mooseError("slnNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableValue & slnOldNeighbor() const override
  {
    mooseError("slnOldNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableGradient & gradSlnNeighbor() const override
  {
    mooseError("gradSlnNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const FieldVariableGradient & gradSlnOldNeighbor() const override
  {
    mooseError("gradSlnOldNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableSecond<OutputType> & adSecondSln() const override
  {
    mooseError("adSecondSln is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableValue<OutputType> & adUDot() const override
  {
    mooseError("adUDot is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableValue<OutputType> & adUDotDot() const override
  {
    mooseError("adUDotDot is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnDot() const override
  {
    mooseError("adGradSlnDot is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableValue<OutputType> & adSlnNeighbor() const override
  {
    mooseError("adSlnNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnNeighbor() const override
  {
    mooseError("adGradSlnNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableSecond<OutputType> & adSecondSlnNeighbor() const override
  {
    mooseError("adSecondSlnNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableValue<OutputType> & adUDotNeighbor() const override
  {
    mooseError("adUDotNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableValue<OutputType> & adUDotDotNeighbor() const override
  {
    mooseError("adUDotDotNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnNeighborDot() const override
  {
    mooseError("adGradSlnNeighborDot is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableValue<OutputType> & adSln() const override
  {
    mooseError("adSln is not implemented for MooseLinearVariableFV!");
  }
  const ADTemplateVariableGradient<OutputType> & adGradSln() const override
  {
    mooseError("adGradSln is not implemented for MooseLinearVariableFV!");
  }

  const DoFValue & dofValues() const override
  {
    mooseError("add is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesOld() const override
  {
    mooseError("dofValues is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesOlder() const override
  {
    mooseError("dofValuesOlder is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesPreviousNL() const override
  {
    mooseError("dofValuesPreviousNL is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesNeighbor() const override
  {
    mooseError("dofValuesNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesOldNeighbor() const override
  {
    mooseError("dofValuesOldNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesOlderNeighbor() const override
  {
    mooseError("dofValuesOlderNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesPreviousNLNeighbor() const override
  {
    mooseError("dofValuesPreviousNLNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDot() const override
  {
    mooseError("dofValuesDot is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDotNeighbor() const override
  {
    mooseError("dofValuesDotNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDotOld() const override
  {
    mooseError("dofValuesDotOld is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDotOldNeighbor() const override
  {
    mooseError("dofValuesDotOldNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDotDot() const override
  {
    mooseError("dofValuesDotDot is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDotDotNeighbor() const override
  {
    mooseError("dofValuesDotDotNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDotDotOld() const override
  {
    mooseError("dofValuesDotDotOld is not implemented for MooseLinearVariableFV!");
  }
  const DoFValue & dofValuesDotDotOldNeighbor() const override
  {
    mooseError("dofValuesDotDotOldNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const MooseArray<Number> & dofValuesDuDotDu() const override
  {
    mooseError("dofValuesDuDotDu is not implemented for MooseLinearVariableFV!");
  }
  const MooseArray<Number> & dofValuesDuDotDuNeighbor() const override
  {
    mooseError("dofValuesDuDotDuNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const MooseArray<Number> & dofValuesDuDotDotDu() const override
  {
    mooseError("dofValuesDuDotDotDu is not implemented for MooseLinearVariableFV!");
  }
  const MooseArray<Number> & dofValuesDuDotDotDuNeighbor() const override
  {
    mooseError("dofValuesDuDotDotDuNeighbor is not implemented for MooseLinearVariableFV!");
  }
  const MooseArray<ADReal> & adDofValues() const override
  {
    mooseError("adDofValues is not supported for MooseLinearVariableFV.");
  }
  const MooseArray<ADReal> & adDofValuesNeighbor() const override
  {
    mooseError("adDofValuesNeighbor is not supported for MooseLinearVariableFV.");
  }
  virtual const dof_id_type & nodalDofIndex() const override final
  {
    mooseError("nodalDofIndex not supported by MooseLinearVariableFV");
  }
  virtual const dof_id_type & nodalDofIndexNeighbor() const override final
  {
    mooseError("nodalDofIndexNeighbor not supported by MooseLinearVariableFV");
  }
  virtual std::size_t phiSize() const override final
  {
    mooseError("phiSize not supported by MooseLinearVariableFVBase");
  }
  virtual std::size_t phiFaceSize() const override final
  {
    mooseError("phiFaceSize not supported by MooseLinearVariableFVBase");
  }
  virtual std::size_t phiNeighborSize() const override final
  {
    mooseError("phiNeighborSize not supported by MooseLinearVariableFVBase");
  }
  virtual std::size_t phiFaceNeighborSize() const override final
  {
    mooseError("phiFaceNeighborSize not supported by MooseLinearVariableFVBase");
  }
  virtual std::size_t phiLowerSize() const override final
  {
    mooseError("phiLowerSize not supported by MooseLinearVariableFVBase");
  }
};

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemArg & elem_arg, const StateArg & state) const
{
  const auto & elem_info = this->_mesh.elemInfo(elem_arg.elem->id());
  return getElemValue(elem_info, state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemPointArg & elem_point,
                                            const StateArg & state) const
{
  const auto & elem_info = this->_mesh.elemInfo(elem_point.elem->id());
  return getElemValue(elem_info, state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemQpArg & elem_qp, const StateArg & state) const
{
  const auto & elem_info = this->_mesh.elemInfo(elem_qp.elem->id());
  return getElemValue(elem_info, state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemSideQpArg & elem_side_qp,
                                            const StateArg & state) const
{
  return (*this)(ElemPointArg{elem_side_qp.elem, elem_side_qp.point, false}, state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::GradientType
MooseLinearVariableFV<OutputType>::evaluateGradient(const ElemQpArg & qp_arg,
                                                    const StateArg & /*state*/) const
{
  const auto & elem_info = this->_mesh.elemInfo(qp_arg.elem->id());
  return gradSln(elem_info);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::GradientType
MooseLinearVariableFV<OutputType>::evaluateGradient(const ElemArg & elem_arg,
                                                    const StateArg & /*state*/) const
{
  const auto & elem_info = this->_mesh.elemInfo(elem_arg.elem->id());
  return gradSln(elem_info);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::GradientType
MooseLinearVariableFV<OutputType>::evaluateGradient(const FaceArg & face,
                                                    const StateArg & state) const
{
  mooseAssert(face.fi, "We must have a non-null face information");
  return gradSln(*face.fi, state);
}

template <>
ADReal MooseLinearVariableFV<Real>::evaluateDot(const ElemArg & elem, const StateArg & state) const;
