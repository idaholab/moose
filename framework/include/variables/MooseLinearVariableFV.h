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

namespace libMesh
{
template <typename>
class NumericVector;
}

/// This class provides variable solution values for other classes/objects to
/// bind to when looping over faces or elements.
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

  // TODO: many of these functions are not relevant to FV variables but are
  // still called at various points from existing moose codepaths.  Ideally we
  // would figure out how to remove calls to these functions and then allow
  // throwing mooseError's from them instead of silently doing nothing (e.g.
  // reinitNodes, reinitAux, prepareLowerD, etc.).

  virtual void prepare() override final {}
  virtual void prepareNeighbor() override final {}
  virtual void prepareAux() override final {}
  virtual void reinitNode() override final {}
  virtual void reinitNodes(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitAux() override final {}
  virtual void reinitAuxNeighbor() override final {}
  virtual void prepareLowerD() override final {}

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

  virtual void computeElemValuesFace() override {}
  virtual void computeNeighborValuesFace() override {}
  virtual void computeNeighborValues() override {}
  virtual void computeLowerDValues() override final {}
  virtual void computeNodalNeighborValues() override final {}
  virtual void computeNodalValues() override final {}
  virtual const std::vector<dof_id_type> & dofIndicesLower() const override final;

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

  void clearDofIndices() override final {}

  virtual void prepareIC() override {}

  Moose::VarFieldType fieldType() const override;
  bool isArray() const override;
  bool isVector() const override;

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

  Moose::FV::InterpMethod faceInterpolationMethod() const { return _face_interp_method; }

  void clearAllDofIndices() final {}

  const DoFValue & nodalVectorTagValue(TagID) const override
  {
    mooseError("nodalVectorTagValue not implemented for finite volume variables.");
  }

  /**
   * Retrieve (or potentially compute) the gradient on the provided element. Overriders of this
   * method *cannot* call \p getBoundaryFaceValue because that method itself may lead to a call to
   * \p adGradSln(const Elem * const) resulting in infinite recursion
   * @param elem The element for which to retrieve the gradient
   * @param state State argument which describes at what time / solution iteration  state we want to
   * evaluate the variable
   * @param correct_skewness Whether to perform skew corrections
   * @return The gradient at the element centroid
   */
  virtual const VectorValue<Real> & adGradSln(const Elem * const elem,
                                              const StateArg & state,
                                              const bool correct_skewness = false) const;

  /**
   * Retrieve (or potentially compute) a cross-diffusion-corrected gradient on the provided face.
   * "Correcting" the face gradient involves weighting the gradient stencil more heavily on the
   * solution values on the face-neighbor cells than a linear interpolation between cell center
   * gradients does
   * @param face The face for which to retrieve the gradient.
   * @param state State argument which describes at what time / solution iteration  state we want to
   * evaluate the variable
   * @param correct_skewness Whether to perform skew corrections
   */
  virtual VectorValue<Real>
  adGradSln(const FaceInfo & fi, const StateArg & state, const bool correct_skewness = false) const;

  /**
   * Retrieve (or potentially compute) the uncorrected gradient on the provided face. This
   * uncorrected gradient is a simple linear interpolation between cell gradients computed at the
   * centers of the two neighboring cells. "Correcting" the face gradient involves weighting the
   * gradient stencil more heavily on the solution values on the face-neighbor cells than the linear
   * interpolation process does. This is commonly known as a cross-diffusion correction. Correction
   * is done in \p adGradSln(const FaceInfo & fi)
   * @param face The face for which to retrieve the gradient
   * @param state State argument which describes at what time / solution iteration  state we want to
   * evaluate the variable
   * @param correct_skewness Whether to perform skew corrections
   */
  virtual VectorValue<Real> uncorrectedAdGradSln(const FaceInfo & fi,
                                                 const StateArg & state,
                                                 const bool correct_skewness = false) const;

  virtual void computeElemValues() override {}
  virtual void computeFaceValues(const FaceInfo & /*fi*/) override {}
  void setDofValues(const DenseVector<OutputData> & /*values*/) override {}
  virtual void insert(NumericVector<Number> & /*residual*/) override {}
  virtual void add(NumericVector<Number> & /*residual*/) override {}

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
  /// Returns the AD dof values.
  const MooseArray<ADReal> & adDofValues() const override
  {
    mooseError("adDofValues is not supported for MooseLinearVariableFV.");
  }

  /// Returns the AD neighbor dof values
  const MooseArray<ADReal> & adDofValuesNeighbor() const override
  {
    mooseError("adDofValuesNeighbor is not supported for MooseLinearVariableFV.");
  }

  /// Note: const monomial is always the case - higher order solns are
  /// reconstructed - so this is simpler func than FE equivalent.
  OutputType getValue(const Elem * elem) const;

  /// Compute the variable gradient value at a point on an element
  /// @param elem The element we are computing on
  /// @param phi Evaluated shape functions at a point
  /// @return The variable gradient value
  typename OutputTools<OutputType>::OutputGradient getGradient(const Elem * elem) const;

  std::pair<bool, const FVDirichletBCBase *> getDirichletBC(const FaceInfo & fi) const;

  std::pair<bool, std::vector<const FVFluxBC *>> getFluxBCs(const FaceInfo & fi) const;

  virtual void residualSetup() override;
  virtual void jacobianSetup() override;
  virtual void timestepSetup() override;
  virtual void meshChanged() override;

  /**
   * Get the solution value for the provided element and seed the derivative for the corresponding
   * dof index
   * @param elem The element to retrieive the solution value for
   * @param state State argument which describes at what time / solution iteration  state we want to
   * evaluate the variable
   */
  Real getElemValue(const Elem * elem, const StateArg & state) const;

  void setActiveTags(const std::set<TagID> & /*vtags*/) override
  {
    mooseError("setActiveTags is not implemented for MooseLinearVariableFV!");
  }

protected:
  /**
   * Determine whether a specified face side is a Dirichlet boundary face. In the base
   * implementation we only inspect the face information object for whether there are Dirichlet
   * conditions. However, derived classes may allow discontinuities between + and - side face
   * values, e.g. one side may have a Dirichlet condition and the other side may perform
   * extrapolation to determine its value
   * @param fi The face information object
   * @param elem An element that can be used to indicate sidedness of the face
   * @param state The state at which to determine whether the face is a Dirichlet face or not
   * @return Whether the potentially sided (as indicated by \p elem) \p fi is a Dirichlet boundary
   * face for this variable
   */
  virtual bool isDirichletBoundaryFace(const FaceInfo & fi,
                                       const Elem * elem,
                                       const Moose::StateArg & state) const;

  /**
   * Retrieves a Dirichlet boundary value for the provided face. Callers of this method should be
   * sure that \p isDirichletBoundaryFace returns true. In the base implementation we only inspect
   * the face information object for the Dirichlet value. However, derived
   * classes may allow discontinuities between + and - side face values, e.g. one side may have a
   * Dirichlet condition and the other side may perform extrapolation to determine its value. This
   * is the reason for the existence of the \p elem parameter, to indicate sidedness
   * @param fi The face information object
   * @param elem An element that can be used to indicate sidedness of the face
   * @param state State argument which describes at what time / solution iteration  state we want to
   * evaluate the variable
   * @return The Dirichlet value on the boundary face associated with \p fi (and potentially \p
   * elem)
   */
  virtual Real getDirichletBoundaryFaceValue(const FaceInfo & fi,
                                             const Elem * elem,
                                             const Moose::StateArg & state) const;

  /**
   * Returns whether this is an extrapolated boundary face. An extrapolated boundary face is
   * boundary face for which is not a corresponding Dirichlet condition, e.g. we need to compute
   * some approximation for the boundary face value using the adjacent cell centroid information. In
   * the base implementation we only inspect the face information object for whether we should
   * perform extrapolation. However, derived classes may allow discontinuities between + and - side
   * face values, e.g. one side may have a Dirichlet condition and the other side may perform
   * extrapolation to determine its value
   * @param fi The face information object
   * @param elem An element that can be used to indicate sidedness of the face
   * @param state The state at which to determine whether the face is extrapolated or not
   * @return Whether the potentially sided (as indicated by \p elem) \p fi is an extrapolated
   * boundary face for this variable
   */
  bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                  const Elem * elem,
                                  const Moose::StateArg & state) const override;

  /**
   * Retrieves an extrapolated boundary value for the provided face. Callers of this method should
   * be sure that \p isExtrapolatedBoundaryFace returns true. In the base implementation we only
   * inspect the face information object for the extrapolated value. However, derived classes may
   * allow discontinuities between + and - side face values, e.g. one side may have a Dirichlet
   * condition and the other side may perform extrapolation to determine its value. This is the
   * reason for the existence of the \p elem parameter, to indicate sidedness
   * @param fi The face information object
   * @param two_term_expansion Whether to use the cell gradient in addition to the cell center value
   * to compute the extrapolated boundary face value. If this is false, then the cell center value
   * will be used
   * @param correct_skewness Whether to perform skew corrections. This is relevant when performing
   * two term expansions as the gradient evaluation may involve evaluating face values on internal
   * skewed faces
   * @param elem_side_to_extrapolate_from An element that can be used to indicate sidedness of the
   * face
   * @param state State argument which describes at what time / solution iteration  state we want to
   * evaluate the variable
   * @return The extrapolated value on the boundary face associated with \p fi (and potentially \p
   * elem_side_to_extrapolate_from)
   */
  virtual Real getExtrapolatedBoundaryFaceValue(const FaceInfo & fi,
                                                bool two_term_expansion,
                                                bool correct_skewness,
                                                const Elem * elem_side_to_extrapolate_from,
                                                const StateArg & state) const;

private:
  using MooseVariableField<OutputType>::evaluate;
  using MooseVariableField<OutputType>::evaluateGradient;
  using MooseVariableField<OutputType>::evaluateDot;

  ValueType evaluate(const ElemArg & elem, const StateArg &) const override final;
  ValueType evaluate(const FaceArg & face, const StateArg &) const override final;
  ValueType evaluate(const NodeArg & node, const StateArg &) const override final;
  ValueType evaluate(const ElemPointArg & elem_point, const StateArg & state) const override final;
  ValueType evaluate(const ElemQpArg & elem_qp, const StateArg & state) const override final;
  ValueType evaluate(const ElemSideQpArg & elem_side_qp,
                     const StateArg & state) const override final;
  GradientType evaluateGradient(const ElemQpArg & qp_arg, const StateArg &) const override final;
  GradientType evaluateGradient(const ElemArg & elem_arg, const StateArg &) const override final;
  GradientType evaluateGradient(const FaceArg & face, const StateArg &) const override final;
  DotType evaluateDot(const ElemArg & elem, const StateArg &) const override final;

  /**
   * Setup the boundary to Dirichlet BC map
   */
  void determineBoundaryToDirichletBCMap();

  /// Whether the boundary to Dirichlet cache map has been setup yet
  bool _dirichlet_map_setup = false;

public:
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

  virtual const FieldVariablePhiValue & phiLower() const override;

  unsigned int oldestSolutionStateRequested() const override final
  {
    mooseError("oldestSolutionStateRequested is not implemented for MooseLinearVariableFV!");
  }

  /**
   * Retrieve the solution value at a boundary face. If we're using a one term Taylor series
   * expansion we'll simply return the ajacent element centroid value. If we're doing two terms,
   * then we will compute the gradient if necessary to help us interpolate from the element centroid
   * value to the face
   */
  Real getBoundaryFaceValue(const FaceInfo & fi,
                            const StateArg & state,
                            bool correct_skewness = false) const;

protected:
  /**
   * clear finite volume caches
   */
  void clearCaches();

  usingMooseVariableBaseMembers;

private:
  /// The current (ghosted) solution. Note that this needs to be stored as a reference to a pointer
  /// because the solution might not exist at the time that this variable is constructed, so we
  /// cannot safely dereference at that time
  const NumericVector<Number> * const & _solution;

  /// A member used to help determine when we can return cached data as opposed to computing new
  /// data
  mutable const Elem * _prev_elem;

  /// Map from boundary ID to Dirichlet boundary conditions. Added to speed up Dirichlet BC lookups
  /// in \p getDirichletBC
  std::unordered_map<BoundaryID, const FVDirichletBCBase *> _boundary_id_to_dirichlet_bc;

protected:
  /// A cache for storing gradients on elements
  mutable std::unordered_map<const Elem *, VectorValue<Real>> _elem_to_grad;

  /// Whether to use a two term expansion for computing boundary face values
  bool _two_term_boundary_expansion;

  /// A member to hold the cell gradient when not caching, used to return a reference (due to
  /// expensive ADReal copy)
  mutable VectorValue<Real> _temp_cell_gradient;

  /// Whether to cache cell gradients
  const bool _cache_cell_gradients;

  /// Decides if an average or skewed corrected average is used for the
  /// face interpolation. Other options are not taken into account here,
  /// but at higher, kernel-based levels.
  Moose::FV::InterpMethod _face_interp_method;

  friend void Moose::initDofIndices<>(MooseLinearVariableFV<OutputType> &, const Elem &);
};

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemArg & elem_arg, const StateArg & state) const
{
  return getElemValue(elem_arg.elem, state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemPointArg & elem_point,
                                            const StateArg & state) const
{
  return (*this)(elem_point.makeElem(), state) +
         (elem_point.point - elem_point.elem->vertex_average()) *
             this->gradient(elem_point.makeElem(), state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemQpArg & elem_qp, const StateArg & state) const
{
  return (*this)(ElemPointArg{elem_qp.elem,
                              elem_qp.point,
                              _face_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage},
                 state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const ElemSideQpArg & elem_side_qp,
                                            const StateArg & state) const
{
  return (*this)(ElemPointArg{elem_side_qp.elem,
                              elem_side_qp.point,
                              _face_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage},
                 state);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::GradientType
MooseLinearVariableFV<OutputType>::evaluateGradient(const ElemQpArg & qp_arg,
                                                    const StateArg & state) const
{
  return adGradSln(qp_arg.elem, state, false);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::GradientType
MooseLinearVariableFV<OutputType>::evaluateGradient(const ElemArg & elem_arg,
                                                    const StateArg & state) const
{
  return adGradSln(elem_arg.elem, state, elem_arg.correct_skewness);
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::GradientType
MooseLinearVariableFV<OutputType>::evaluateGradient(const FaceArg & face,
                                                    const StateArg & state) const
{
  mooseAssert(face.fi, "We must have a non-null face information");
  return adGradSln(*face.fi, state, face.correct_skewness);
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseLinearVariableFV<OutputType>::dofIndicesLower() const
{
  static const std::vector<dof_id_type> empty;
  return empty;
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::meshChanged()
{
  _prev_elem = nullptr;
  _dirichlet_map_setup = false;
  MooseVariableField<OutputType>::meshChanged();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::timestepSetup()
{
  _dirichlet_map_setup = false;
  MooseVariableField<OutputType>::timestepSetup();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiValue &
MooseLinearVariableFV<OutputType>::phiLower() const
{
  mooseError("We don't currently have shape functions for linear FV variables");
}

template <>
ADReal MooseLinearVariableFV<Real>::evaluateDot(const ElemArg & elem, const StateArg & state) const;
