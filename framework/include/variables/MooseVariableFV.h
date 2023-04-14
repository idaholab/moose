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
class MooseVariableFV;

typedef MooseVariableFV<Real> MooseVariableFVReal;
class FVDirichletBCBase;
class FVFluxBC;

namespace libMesh
{
template <typename>
class NumericVector;
}

/// This class provides variable solution values for other classes/objects to
/// bind to when looping over faces or elements.  It provides both
/// elem and neighbor values when accessed in flux/face object calculations.
/// The templating is to enable support for both vector and scalar variables.
/// Gradient reconstruction (when enabled) occurs transparently within this
/// class and kernels coupling to these values will naturally "see" according
/// to the selected reconstruction methods.
///
/// OutputType          OutputShape           OutputData
/// ----------------------------------------------------
/// Real                Real                  Real
/// RealVectorValue     RealVectorValue       Real
/// RealEigenVector      Real                  RealEigenVector
template <typename OutputType>
class MooseVariableFV : public MooseVariableField<OutputType>
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
  using ElemArg = Moose::ElemArg;
  using FaceArg = Moose::FaceArg;
  using StateArg = Moose::StateArg;

  static InputParameters validParams();

  MooseVariableFV(const InputParameters & parameters);

  virtual bool isFV() const override { return true; }

  // TODO: many of these functions are not relevant to FV variables but are
  // still called at various points from existing moose codepaths.  Ideally we
  // would figure out how to remove calls to these functions and then allow
  // throwing mooseError's from them instead of silently doing nothing (e.g.
  // reinitNodes, reinitAux, prepareLowerD, etc.).

  virtual void prepare() override final {}
  virtual void prepareNeighbor() override final {}
  virtual void prepareAux() override final;
  virtual void reinitNode() override final {}
  virtual void reinitNodes(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitAux() override final {}
  virtual void reinitAuxNeighbor() override final {}
  virtual void prepareLowerD() override final {}

  virtual const dof_id_type & nodalDofIndex() const override final
  {
    mooseError("nodalDofIndex not supported by MooseVariableFVBase");
  }
  virtual const dof_id_type & nodalDofIndexNeighbor() const override final
  {
    mooseError("nodalDofIndexNeighbor not supported by MooseVariableFVBase");
  }
  virtual std::size_t phiSize() const override final { return _phi.size(); }
  virtual std::size_t phiFaceSize() const override final { return _phi_face.size(); }
  virtual std::size_t phiNeighborSize() const override final { return _phi_neighbor.size(); }
  virtual std::size_t phiFaceNeighborSize() const override final
  {
    return _phi_face_neighbor.size();
  }
  virtual std::size_t phiLowerSize() const override final
  {
    mooseError("phiLowerSize not supported by MooseVariableFVBase");
  }

  virtual void computeElemValuesFace() override;
  virtual void computeNeighborValuesFace() override;
  virtual void computeNeighborValues() override;
  virtual void computeLowerDValues() override final
  {
    // mooseError("computeLowerDValues not supported by MooseVariableFVBase");
  }
  virtual void computeNodalNeighborValues() override final
  {
    // mooseError("computeNodalNeighborValues not supported by MooseVariableFVBase");
  }
  virtual void computeNodalValues() override final
  {
    // mooseError("computeNodalValues not supported by MooseVariableFVBase");
  }
  virtual const std::vector<dof_id_type> & dofIndicesLower() const override final;

  unsigned int numberOfDofs() const override final { return _element_data->numberOfDofs(); }

  virtual unsigned int numberOfDofsNeighbor() override final
  {
    mooseError("numberOfDofsNeighbor not supported by MooseVariableFVBase");
  }

  virtual bool isNodal() const override final { return false; }

  bool hasDoFsOnNodes() const override final { return false; }

  FEContinuity getContinuity() const override final { return _element_data->getContinuity(); };

  virtual bool isNodalDefined() const override final { return false; }

  virtual void setNodalValue(const OutputType & value, unsigned int idx = 0) override;

  virtual void setDofValue(const OutputData & value, unsigned int index) override;

  void clearDofIndices() override;

  virtual void prepareIC() override;

  Moose::VarFieldType fieldType() const override;
  bool isArray() const override;
  bool isVector() const override;

  virtual const Elem * const & currentElem() const override { return _element_data->currentElem(); }

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

  Moose::FV::InterpMethod faceInterpolationMethod() const { return _face_interp_method; }

  void clearAllDofIndices() final;

  const DoFValue & nodalVectorTagValue(TagID) const override
  {
    mooseError("nodalVectorTagValue not implemented for finite volume variables.");
  }

  const FieldVariableValue & vectorTagValue(TagID tag) const override
  {
    return _element_data->vectorTagValue(tag);
  }
  const DoFValue & vectorTagDofValue(TagID tag) const override
  {
    return _element_data->vectorTagDofValue(tag);
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

  const FieldVariableValue & uDot() const { return _element_data->uDot(); }
  const FieldVariableValue & sln() const override { return _element_data->sln(Moose::Current); }
  const FieldVariableValue & slnOld() const override { return _element_data->sln(Moose::Old); }
  const FieldVariableValue & slnOlder() const override { return _element_data->sln(Moose::Older); }
  const FieldVariableGradient & gradSln() const override
  {
    return _element_data->gradSln(Moose::Current);
  }
  const FieldVariableGradient & gradSlnOld() const override
  {
    return _element_data->gradSln(Moose::Old);
  }
  const FieldVariableValue & uDotNeighbor() const { return _neighbor_data->uDot(); }
  const FieldVariableValue & slnNeighbor() const override
  {
    return _neighbor_data->sln(Moose::Current);
  }
  const FieldVariableValue & slnOldNeighbor() const override
  {
    return _neighbor_data->sln(Moose::Old);
  }
  const FieldVariableGradient & gradSlnNeighbor() const override
  {
    return _neighbor_data->gradSln(Moose::Current);
  }
  const FieldVariableGradient & gradSlnOldNeighbor() const override
  {
    return _neighbor_data->gradSln(Moose::Old);
  }

  const VariableValue & duDotDu() const { return _element_data->duDotDu(); }
  const VariableValue & duDotDotDu() const { return _element_data->duDotDotDu(); }
  const VariableValue & duDotDuNeighbor() const { return _neighbor_data->duDotDu(); }
  const VariableValue & duDotDotDuNeighbor() const { return _neighbor_data->duDotDotDu(); }

  /// AD
  const ADTemplateVariableValue<OutputType> & adSln() const override
  {
    return _element_data->adSln();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSln() const override
  {
    return _element_data->adGradSln();
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
  virtual const VectorValue<ADReal> & adGradSln(const Elem * const elem,
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
  virtual VectorValue<ADReal>
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
  virtual VectorValue<ADReal> uncorrectedAdGradSln(const FaceInfo & fi,
                                                   const StateArg & state,
                                                   const bool correct_skewness = false) const;

  /**
   * Retrieve the solution value at a boundary face. If we're using a one term Taylor series
   * expansion we'll simply return the ajacent element centroid value. If we're doing two terms,
   * then we will compute the gradient if necessary to help us interpolate from the element centroid
   * value to the face
   */
  ADReal getBoundaryFaceValue(const FaceInfo & fi,
                              const StateArg & state,
                              bool correct_skewness = false) const;

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

  /// Initializes/computes variable values from the solution vectors for the
  /// current element being operated on in assembly. This
  /// must be called every time the current element (as defined by the assembly
  /// data structure changes in order for objects that bind to sln, adSln,
  /// uDot, etc. to be updated/correct.
  virtual void computeElemValues() override;
  /// Initializes/computes variable values from the solution vectors for the
  /// face represented by fi.  This includes initializing data for elements on
  /// both sides of the face (elem and neighbor) and handles the case where
  /// there is no element on one side of the face gracefully - i.e. you call
  /// this uniformly for every face regardless of whether you are on a boundary
  /// or not - or whether the variable is only defined on one side of the face
  /// or not.
  virtual void computeFaceValues(const FaceInfo & fi) override;

  /**
   * Set local DOF values and evaluate the values on quadrature points
   */
  void setDofValues(const DenseVector<OutputData> & values) override;

  /// Get the current value of this variable on an element
  /// @param[in] elem   Element at which to get value
  /// @param[in] idx    Local index of this variable's element DoFs
  /// @return Variable value
  OutputData getElementalValue(const Elem * elem, unsigned int idx = 0) const;
  /// Get the old value of this variable on an element
  /// @param[in] elem   Element at which to get value
  /// @param[in] idx    Local index of this variable's element DoFs
  /// @return Variable value
  OutputData getElementalValueOld(const Elem * elem, unsigned int idx = 0) const;
  /// Get the older value of this variable on an element
  /// @param[in] elem   Element at which to get value
  /// @param[in] idx    Local index of this variable's element DoFs
  /// @return Variable value
  OutputData getElementalValueOlder(const Elem * elem, unsigned int idx = 0) const;

  virtual void insert(NumericVector<Number> & residual) override;
  virtual void add(NumericVector<Number> & residual) override;

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
  const DoFValue & dofValuesDotOld() const override;
  const DoFValue & dofValuesDotOldNeighbor() const override;
  const DoFValue & dofValuesDotDot() const override;
  const DoFValue & dofValuesDotDotNeighbor() const override;
  const DoFValue & dofValuesDotDotOld() const override;
  const DoFValue & dofValuesDotDotOldNeighbor() const override;
  const MooseArray<Number> & dofValuesDuDotDu() const override;
  const MooseArray<Number> & dofValuesDuDotDuNeighbor() const override;
  const MooseArray<Number> & dofValuesDuDotDotDu() const override;
  const MooseArray<Number> & dofValuesDuDotDotDuNeighbor() const override;

  /// Returns the AD dof values.
  const MooseArray<ADReal> & adDofValues() const override;

  /// Returns the AD neighbor dof values
  const MooseArray<ADReal> & adDofValuesNeighbor() const override;

  /// Note: const monomial is always the case - higher order solns are
  /// reconstructed - so this is simpler func than FE equivalent.
  OutputType getValue(const Elem * elem) const;

  /// Compute the variable gradient value at a point on an element
  /// @param elem The element we are computing on
  /// @param phi Evaluated shape functions at a point
  /// @return The variable gradient value
  typename OutputTools<OutputType>::OutputGradient getGradient(const Elem * elem) const;

  /// Returns true if a Dirichlet BC exists on the current face.  This only
  /// works if the variable has been initialized on a face with
  /// computeFaceValues.  Its return value is nonsense if initialized on a
  /// single element via computeElemValues.
  bool hasDirichletBC() const
  {
    return _element_data->hasDirichletBC() || _neighbor_data->hasDirichletBC();
  }

  std::pair<bool, const FVDirichletBCBase *> getDirichletBC(const FaceInfo & fi) const;

  std::pair<bool, std::vector<const FVFluxBC *>> getFluxBCs(const FaceInfo & fi) const;

  void residualSetup() override;
  void jacobianSetup() override;

  /**
   * Get the solution value for the provided element and seed the derivative for the corresponding
   * dof index
   * @param elem The element to retrieive the solution value for
   * @param state State argument which describes at what time / solution iteration  state we want to
   * evaluate the variable
   */
  ADReal getElemValue(const Elem * elem, const StateArg & state) const;

  using FunctorArg = typename Moose::ADType<OutputType>::type;
  using typename Moose::FunctorBase<FunctorArg>::ValueType;
  using typename Moose::FunctorBase<FunctorArg>::DotType;
  using typename Moose::FunctorBase<FunctorArg>::GradientType;

  void setActiveTags(const std::set<TagID> & vtags) override;

  void meshChanged() override;
  void initialSetup() override;

  /**
   * Request that quadrature point data be (pre)computed. Quadrature point data is (pre)computed by
   * default for this base class but derived variable classes may choose not to unless this API is
   * called
   */
  virtual void requireQpComputations() {}

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
  virtual ADReal getDirichletBoundaryFaceValue(const FaceInfo & fi,
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
  virtual ADReal getExtrapolatedBoundaryFaceValue(const FaceInfo & fi,
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
  GradientType evaluateGradient(const ElemQpArg & qp_arg, const StateArg &) const override final;
  GradientType evaluateGradient(const ElemArg & elem_arg, const StateArg &) const override final;
  GradientType evaluateGradient(const FaceArg & face, const StateArg &) const override final;
  DotType evaluateDot(const ElemArg & elem, const StateArg &) const override final;

  /**
   * Setup the boundary to Dirichlet BC map
   */
  void determineBoundaryToDirichletBCMap();

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

  const FieldVariablePhiValue & phi() const override final { return _phi; }
  const FieldVariablePhiGradient & gradPhi() const override final { return _grad_phi; }
  const FieldVariablePhiSecond & secondPhi() const override final
  {
    mooseError("We don't currently implement second derivatives for FV");
  }
  const FieldVariablePhiValue & curlPhi() const override final
  {
    mooseError("We don't currently implement curl for FV");
  }

  const FieldVariablePhiValue & phiFace() const override final { return _phi_face; }
  const FieldVariablePhiGradient & gradPhiFace() const override final { return _grad_phi_face; }
  const FieldVariablePhiSecond & secondPhiFace() const override final
  {
    mooseError("We don't currently implement second derivatives for FV");
  }

  const FieldVariablePhiValue & phiFaceNeighbor() const override final
  {
    return _phi_face_neighbor;
  }
  const FieldVariablePhiGradient & gradPhiFaceNeighbor() const override final
  {
    return _grad_phi_face_neighbor;
  }
  const FieldVariablePhiSecond & secondPhiFaceNeighbor() const override final
  {
    mooseError("We don't currently implement second derivatives for FV");
  }

  const FieldVariablePhiValue & phiNeighbor() const override final { return _phi_neighbor; }
  const FieldVariablePhiGradient & gradPhiNeighbor() const override final
  {
    return _grad_phi_neighbor;
  }
  const FieldVariablePhiSecond & secondPhiNeighbor() const override final
  {
    mooseError("We don't currently implement second derivatives for FV");
  }

  virtual const FieldVariablePhiValue & phiLower() const override;

  unsigned int oldestSolutionStateRequested() const override final;

protected:
  /**
   * clear finite volume caches
   */
  void clearCaches();

  usingMooseVariableBaseMembers;

  /// Holder for all the data associated with the "main" element
  std::unique_ptr<MooseVariableDataFV<OutputType>> _element_data;

  /// Holder for all the data associated with the neighbor element
  std::unique_ptr<MooseVariableDataFV<OutputType>> _neighbor_data;

private:
  /// The current (ghosted) solution. Note that this needs to be stored as a reference to a pointer
  /// because the solution might not exist at the time that this variable is constructed, so we
  /// cannot safely dereference at that time
  const NumericVector<Number> * const & _solution;

  /// Shape functions
  const FieldVariablePhiValue & _phi;
  const FieldVariablePhiGradient & _grad_phi;
  const FieldVariablePhiValue & _phi_face;
  const FieldVariablePhiGradient & _grad_phi_face;
  const FieldVariablePhiValue & _phi_face_neighbor;
  const FieldVariablePhiGradient & _grad_phi_face_neighbor;
  const FieldVariablePhiValue & _phi_neighbor;
  const FieldVariablePhiGradient & _grad_phi_neighbor;

  /// A member used to help determine when we can return cached data as opposed to computing new
  /// data
  mutable const Elem * _prev_elem;

  /// Map from boundary ID to Dirichlet boundary conditions. Added to speed up Dirichlet BC lookups
  /// in \p getDirichletBC
  std::unordered_map<BoundaryID, const FVDirichletBCBase *> _boundary_id_to_dirichlet_bc;

protected:
  /// A cache for storing gradients on elements
  mutable std::unordered_map<const Elem *, VectorValue<ADReal>> _elem_to_grad;

  /// Whether to use a two term expansion for computing boundary face values
  bool _two_term_boundary_expansion;

  /// A member to hold the cell gradient when not caching, used to return a reference (due to
  /// expensive ADReal copy)
  mutable VectorValue<ADReal> _temp_cell_gradient;

  /// Whether to cache cell gradients
  const bool _cache_cell_gradients;

  /// Decides if an average or skewed corrected average is used for the
  /// face interpolation. Other options are not taken into account here,
  /// but at higher, kernel-based levels.
  Moose::FV::InterpMethod _face_interp_method;

  friend void Moose::initDofIndices<>(MooseVariableFV<OutputType> &, const Elem &);
};

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFV<OutputType>::adDofValues() const
{
  return _element_data->adDofValues();
}

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFV<OutputType>::adDofValuesNeighbor() const
{
  return _neighbor_data->adDofValues();
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::ValueType
MooseVariableFV<OutputType>::evaluate(const ElemArg & elem_arg, const StateArg & state) const
{
  return getElemValue(elem_arg.elem, state);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::GradientType
MooseVariableFV<OutputType>::evaluateGradient(const ElemQpArg & qp_arg,
                                              const StateArg & state) const
{
  return adGradSln(std::get<0>(qp_arg), state, false);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::GradientType
MooseVariableFV<OutputType>::evaluateGradient(const ElemArg & elem_arg,
                                              const StateArg & state) const
{
  return adGradSln(elem_arg.elem, state, elem_arg.correct_skewness);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::GradientType
MooseVariableFV<OutputType>::evaluateGradient(const FaceArg & face, const StateArg & state) const
{
  mooseAssert(face.fi, "We must have a non-null face information");
  return adGradSln(*face.fi, state, face.correct_skewness);
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::setActiveTags(const std::set<TagID> & vtags)
{
  _element_data->setActiveTags(vtags);
  _neighbor_data->setActiveTags(vtags);
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseVariableFV<OutputType>::dofIndicesLower() const
{
  static const std::vector<dof_id_type> empty;
  return empty;
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::meshChanged()
{
  _prev_elem = nullptr;
  determineBoundaryToDirichletBCMap();
  MooseVariableField<OutputType>::meshChanged();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::FieldVariablePhiValue &
MooseVariableFV<OutputType>::phiLower() const
{
  mooseError("Not defined for finite volume variables");
}

template <>
ADReal MooseVariableFV<Real>::evaluateDot(const ElemArg & elem, const StateArg & state) const;
