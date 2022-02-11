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
#include "libmesh/dense_vector.h"

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

  static InputParameters validParams();

  MooseVariableFV(const InputParameters & parameters);

  virtual bool isFV() const override { return true; }

  // TODO: many of these functions are not relevant to FV variables but are
  // still called at various points from existing moose codepaths.  Ideally we
  // would figure out how to remove calls to these functions and then allow
  // throwing mooseError's from them instead of silently doing nothing (e.g.
  // reinitNodes, reinitAux, prepareLowerD, etc.).

  virtual void prepare() override final
  {
    // mooseError("prepare not supported by MooseVariableFVBase");
  }
  virtual void prepareNeighbor() override final
  {
    // mooseError("prepareNeighbor not supported by MooseVariableFVBase");
  }
  virtual void prepareAux() override final
  {
    // mooseError("prepareAux not supported by MooseVariableFVBase");
  }
  virtual void reinitNode() override final
  {
    // mooseError("reinitNode not supported by MooseVariableFVBase");
  }
  virtual void reinitNodes(const std::vector<dof_id_type> & /*nodes*/) override final
  {
    // mooseError("reinitNodes not supported by MooseVariableFVBase");
  }
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & /*nodes*/) override final
  {
    // mooseError("reinitNodesNeighbor not supported by MooseVariableFVBase");
  }
  virtual void reinitAux() override final
  {
    // mooseError("reinitAux not supported by MooseVariableFVBase");
  }
  virtual void reinitAuxNeighbor() override final
  {
    // mooseError("reinitAuxNeighbor not supported by MooseVariableFVBase");
  }
  virtual void prepareLowerD() override final
  {
    // mooseError("prepareLowerD not supported by MooseVariableFVBase");
  }
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

  virtual void computeElemValuesFace() override final;
  virtual void computeNeighborValuesFace() override final;

  virtual void computeNeighborValues() override final;

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
  virtual const std::vector<dof_id_type> & dofIndicesLower() const override final
  {
    mooseError("dofIndicesLower not supported by MooseVariableFVBase");
  }

  unsigned int numberOfDofs() const override final { return _element_data->numberOfDofs(); }

  virtual unsigned int numberOfDofsNeighbor() override final
  {
    mooseError("numberOfDofsNeighbor not supported by MooseVariableFVBase");
  }

  virtual bool isNodal() const override final { return false; }

  virtual bool isNodalDefined() const override final { return false; }

  virtual void setNodalValue(const OutputType & value, unsigned int idx = 0) override;

  virtual void setDofValue(const OutputData & value, unsigned int index) override;

  void clearDofIndices() override;

  virtual void prepareIC() override;

  const std::set<SubdomainID> & activeSubdomains() const override;
  bool activeOnSubdomain(SubdomainID subdomain) const override;

  Moose::VarFieldType fieldType() const override;
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
   * @return The gradient at the element centroid
   */
  virtual const VectorValue<ADReal> & adGradSln(const Elem * const elem,
                                                const bool correct_skewness = false) const;

  /**
   * Retrieve (or potentially compute) a cross-diffusion-corrected gradient on the provided face.
   * "Correcting" the face gradient involves weighting the gradient stencil more heavily on the
   * solution values on the face-neighbor cells than a linear interpolation between cell center
   * gradients does
   * @param face The face for which to retrieve the gradient.
   */
  const VectorValue<ADReal> & adGradSln(const FaceInfo & fi,
                                        const bool correct_skewness = false) const;

  /**
   * Retrieve (or potentially compute) the uncorrected gradient on the provided face. This
   * uncorrected gradient is a simple linear interpolation between cell gradients computed at the
   * centers of the two neighboring cells. "Correcting" the face gradient involves weighting the
   * gradient stencil more heavily on the solution values on the face-neighbor cells than the linear
   * interpolation process does. This is commonly known as a cross-diffusion correction. Correction
   * is done in \p adGradSln(const FaceInfo & fi)
   * @param face The face for which to retrieve the gradient
   */
  const VectorValue<ADReal> & uncorrectedAdGradSln(const FaceInfo & fi,
                                                   const bool correct_skewness = false) const;

  /**
   * Retrieve the solution value at a boundary face. If we're using a one term Taylor series
   * expansion we'll simply return the ajacent element centroid value. If we're doing two terms,
   * then we will compute the gradient if necessary to help us interpolate from the element centroid
   * value to the face
   */
  const ADReal & getBoundaryFaceValue(const FaceInfo & fi) const;

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
   */
  ADReal getElemValue(const Elem * elem) const;

  /**
   * Get the solution value with derivative seeding on the \p neighbor element. If the neighbor
   * is null or this variable doesn't exist on the neighbor element's subdomain, then we compute a
   * neighbor value based on any Dirichlet boundary conditions associated with the face information,
   * or absent that we assume a zero gradient and simply return the \p elem_value
   * @param neighbor The \p neighbor element that we want to retrieve the solution value for
   * @param fi The face information object
   * @param elem_value The solution value on the "element". This value may be used for computing the
   * neighbor value if the neighbor is null or this variable doesn't exist on the neighbor subdomain
   * @return The neighbor solution value with derivative seeding according to the associated degree
   * of freedom
   */
  ADReal getNeighborValue(const Elem * const neighbor,
                          const FaceInfo & fi,
                          const ADReal & elem_value) const;

  /**
   * Compute or retrieve from cache the solution value on an internal face, using linear
   * interpolation or interpolating from vertex values, depending on the stencil.
   * @param fi The face information object
   * @return the face value on the internal face associated with \p fi
   */
  const ADReal & getInternalFaceValue(const FaceInfo & fi,
                                      const bool correct_skewness = false) const;

  using FunctorArg = typename Moose::ADType<OutputType>::type;
  using typename Moose::FunctorBase<FunctorArg>::ValueType;
  using typename Moose::FunctorBase<FunctorArg>::DotType;
  using typename Moose::FunctorBase<FunctorArg>::GradientType;

  /**
   * This method gets forwarded calls to \p evaluate for \p FaceArg and \p SingleSidedFaceArg
   * spatial argument types and returns an internal face evaluation
   */
  template <typename FaceCallingArg>
  ADReal getInternalFaceValue(const FaceCallingArg & face) const;

protected:
  /**
   * @return whether \p fi is an internal face for this variable
   */
  bool isInternalFace(const FaceInfo & fi) const;

  /**
   * @return whether \p fi is a Dirichlet boundary face for this variable
   */
  virtual bool isDirichletBoundaryFace(const FaceInfo & fi) const;

  /**
   * @return the Dirichlet value on the boundary face associated with \p fi
   */
  virtual const ADReal & getDirichletBoundaryFaceValue(const FaceInfo & fi) const;

  /**
   * Returns whether this is an extrapolated boundary face. An extrapolated boundary face is
   * boundary face for which is not a corresponding Dirichlet condition, e.g. we need to compute
   * some approximation for the boundary face value using the adjacent cell centroid information
   */
  bool isExtrapolatedBoundaryFace(const FaceInfo & fi) const override;

private:
  using MooseVariableField<OutputType>::evaluate;
  using MooseVariableField<OutputType>::evaluateGradient;
  using MooseVariableField<OutputType>::evaluateDot;
  using ElemArg = Moose::ElemArg;
  using ElemFromFaceArg = Moose::ElemFromFaceArg;
  using FaceArg = Moose::FaceArg;
  using SingleSidedFaceArg = Moose::SingleSidedFaceArg;

  ValueType evaluate(const ElemArg & elem, unsigned int) const override final;
  ValueType evaluate(const ElemFromFaceArg & elem_from_face, unsigned int) const override final;
  ValueType evaluate(const FaceArg & face, unsigned int) const override final;
  ValueType evaluate(const SingleSidedFaceArg & face, unsigned int) const override final;
  GradientType evaluateGradient(const ElemArg & elem_arg, unsigned int) const override final;
  GradientType evaluateGradient(const ElemFromFaceArg & elem_from_face,
                                unsigned int) const override final;
  GradientType evaluateGradient(const FaceArg & face, unsigned int) const override final;
  GradientType evaluateGradient(const SingleSidedFaceArg & face, unsigned int) const override final;
  DotType evaluateDot(const ElemArg & elem, unsigned int) const override final;
  DotType evaluateDot(const FaceArg & face, unsigned int) const override final;
  DotType evaluateDot(const SingleSidedFaceArg & face, unsigned int) const override final;

  /**
   * @return the extrapolated value on the boundary face associated with \p fi
   */
  const ADReal & getExtrapolatedBoundaryFaceValue(const FaceInfo & fi) const;

  /**
   * Get the finite volume solution interpolated to \p vertex. This interpolation is done doing a
   * distance-weighted average of neighboring cell center values
   * @param vertex The mesh vertex we want to interpolate the finite volume solution to
   * @return The interpolated vertex value with derivative information from the degrees of freedom
   * associated with the neighboring cell centers
   */
  const ADReal & getVertexValue(const Node & vertex) const;

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

protected:
  /// A cache for storing gradients on elements
  mutable std::unordered_map<const Elem *, VectorValue<ADReal>> _elem_to_grad;

  /// A cache for storing uncorrected gradients on faces
  mutable std::unordered_map<const FaceInfo *, VectorValue<ADReal>> _face_to_unc_grad;

  /// A cache that maps from faces to face values
  mutable std::unordered_map<const FaceInfo *, ADReal> _face_to_value;

  /// Whether to use a two term expansion for computing boundary face values
  bool _two_term_boundary_expansion;

  /// A member to hold the cell gradient when not caching, used to return a reference (due to
  /// expensive ADReal copy)
  mutable VectorValue<ADReal> _temp_cell_gradient;

  /// A member to hold the uncorrected face gradient when not caching, used to return a reference
  mutable VectorValue<ADReal> _temp_face_unc_gradient;

  /// A member to hold the face gradient when not caching, used to return a reference
  mutable VectorValue<ADReal> _temp_face_gradient;

  /// A member to hold the face value when not caching, used to return a reference
  mutable ADReal _temp_face_value;

  /// Whether to cache the gradients the first time they are computed on a cell face. Caching avoids
  /// redundant calculation, but can increase the memory cost substantially
  const bool _cache_face_gradients;

  /// Whether to cache face values or re-compute them every time
  const bool _cache_face_values;

  /// Whether to cache cell gradients
  const bool _cache_cell_gradients;

  /// Decides if a vertex-based, average or skewed corrected average is used for the
  /// face interpolation. Other options are not taken into account here,
  /// but at higher, kernel-based levels.
  Moose::FV::InterpMethod _face_interp_method;

private:
  /**
   * A helper function for evaluating this variable with face arguments. This is leveraged by both
   * \p FaceArg and \p SingleSidedFaceArg evaluate overloads
   */
  template <typename FaceCallingArg>
  ValueType evaluateFaceHelper(const FaceCallingArg & face) const;

  /**
   * A helper function for evaluating this variable's time derivative with face arguments. This is
   * leveraged by both \p FaceArg and \p SingleSidedFaceArg evaluateDot overloads
   */
  template <typename FaceCallingArg>
  DotType evaluateFaceDotHelper(const FaceCallingArg & face) const;

  /// A cache for storing gradients on faces
  mutable std::unordered_map<const FaceInfo *, VectorValue<ADReal>> _face_to_grad;

  /// A cache that maps from mesh vertices to interpolated finite volume solutions at those vertices
  mutable std::unordered_map<const Node *, ADReal> _vertex_to_value;
};

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFV<OutputType>::adDofValues() const
{
  return _element_data->adDofValues();
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::ValueType
MooseVariableFV<OutputType>::evaluate(const ElemArg & elem_arg, unsigned int) const
{
  return getElemValue(elem_arg.elem);
}

template <typename OutputType>
template <typename FaceCallingArg>
ADReal
MooseVariableFV<OutputType>::getInternalFaceValue(const FaceCallingArg & face) const
{
  const FaceInfo * const fi = face.fi;
  mooseAssert(fi, "The face information must be non-null");
  mooseAssert(face.limiter_type == Moose::FV::LimiterType::CentralDifference,
              "This method currently only supports central differencing.");

  return getInternalFaceValue(*fi, face.apply_gradient_to_skewness);
}

template <typename OutputType>
template <typename FaceCallingArg>
typename MooseVariableFV<OutputType>::ValueType
MooseVariableFV<OutputType>::evaluateFaceHelper(const FaceCallingArg & face) const
{
  const FaceInfo * const fi = face.fi;
  mooseAssert(fi, "The face information must be non-null");
  if (isExtrapolatedBoundaryFace(*fi))
    return getExtrapolatedBoundaryFaceValue(*fi);
  else if (isInternalFace(*fi))
    return getInternalFaceValue(face);
  else
  {
    mooseAssert(isDirichletBoundaryFace(*fi), "We've run out of face types");
    return getDirichletBoundaryFaceValue(*fi);
  }
}

template <typename OutputType>
template <typename FaceCallingArg>
typename MooseVariableFV<OutputType>::DotType
MooseVariableFV<OutputType>::evaluateFaceDotHelper(const FaceCallingArg & face) const
{
  const FaceInfo * const fi = face.fi;
  mooseAssert(fi, "The face information must be non-null");
  if (isInternalFace(*fi))
  {
    auto limiter = Moose::FV::Limiter<ADReal>::build(face.limiter_type);
    mooseAssert(limiter->constant(),
                "Cannot do interpolation of time derivatives with non-constant limiting functions "
                "because we have not implementation computation of gradients of time derivatives.");
    const bool elem_is_upwind = face.elem_is_upwind;

    const auto elem_dot = this->dot(face.makeElem());
    mooseAssert(fi->neighborPtr(), "We're supposed to be on an internal face.");
    const auto neighbor_dot = this->dot(face.makeNeighbor());
    const auto & upwind_dot = elem_is_upwind ? elem_dot : neighbor_dot;
    const auto & downwind_dot = elem_is_upwind ? neighbor_dot : elem_dot;

    return Moose::FV::interpolate(
        *limiter, upwind_dot, downwind_dot, (ADRealVectorValue *)nullptr, *fi, elem_is_upwind);
  }
  else
  {
    if (this->hasBlocks(fi->elem().subdomain_id()))
      // Use element centroid evaluation as face evaluation
      return this->dot(face.makeElem());
    mooseAssert(fi->neighborPtr() && this->hasBlocks(fi->neighbor().subdomain_id()),
                "We should not be evaluating this variable when the variable doesn't exist on "
                "either side of the face.");
    return this->dot(face.makeNeighbor());
  }
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::GradientType
MooseVariableFV<OutputType>::evaluateGradient(const ElemArg & elem_arg, unsigned int) const
{
  return adGradSln(elem_arg.elem, elem_arg.apply_gradient_to_skewness);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::GradientType
MooseVariableFV<OutputType>::evaluateGradient(const FaceArg & face, unsigned int) const
{
  mooseAssert(face.fi, "We must have a non-null face information");
  return adGradSln(*face.fi, face.apply_gradient_to_skewness);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::GradientType
MooseVariableFV<OutputType>::evaluateGradient(const SingleSidedFaceArg & face, unsigned int) const
{
  const auto * const fi = face.fi;
  mooseAssert(fi, "We must have a non-null face information");
  return adGradSln(*fi, face.apply_gradient_to_skewness);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::DotType
MooseVariableFV<OutputType>::evaluateDot(const FaceArg & face, unsigned int) const
{
  return evaluateFaceDotHelper(face);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::DotType
MooseVariableFV<OutputType>::evaluateDot(const SingleSidedFaceArg & face, unsigned int) const
{
  return evaluateFaceDotHelper(face);
}

template <>
ADReal MooseVariableFV<Real>::evaluateDot(const ElemArg & elem, unsigned int state) const;
