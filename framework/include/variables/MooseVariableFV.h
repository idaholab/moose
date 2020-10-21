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
class FVDirichletBC;
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

  virtual void computeElemValuesFace() override final
  {
    // mooseError("computeElemValuesFace not supported by MooseVariableFVBase");
  }
  virtual void computeNeighborValuesFace() override final
  {
    // mooseError("computeNeighborValuesFace not supported by MooseVariableFVBase");
  }
  virtual void computeNeighborValues() override final
  {
    // mooseError("computeNeighborValues not supported by MooseVariableFVBase");
  }
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
    checkIndexingScalingCompatibility();
    return _element_data->adSln();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSln() const override
  {
    checkIndexingScalingCompatibility();
    return _element_data->adGradSln();
  }

#ifdef MOOSE_GLOBAL_AD_INDEXING
  /**
   * Retrieve (or potentially compute) the gradient on the provided element
   * @param elem The element for which to retrieve the gradient
   */
  const VectorValue<ADReal> & adGradSln(const Elem * const elem) const;

  /**
   * Retrieve (or potentially compute) a cross-diffusion-corrected gradient on the provided face.
   * "Correcting" the face gradient involves weighting the gradient stencil more heavily on the
   * solution values on the face-neighbor cells than a linear interpolation between cell center
   * gradients does
   * @param face The face for which to retrieve the gradient.
   */
  const VectorValue<ADReal> & adGradSln(const FaceInfo & fi) const;

  /**
   * Retrieve (or potentially compute) the uncorrected gradient on the provided face. This
   * uncorrected gradient is a simple linear interpolation between cell gradients computed at the
   * centers of the two neighboring cells. "Correcting" the face gradient involves weighting the
   * gradient stencil more heavily on the solution values on the face-neighbor cells than the linear
   * interpolation process does. This is commonly known as a cross-diffusion correction. Correction
   * is done in \p adGradSln(const FaceInfo & fi)
   * @param face The face for which to retrieve the gradient
   */
  const VectorValue<ADReal> & uncorrectedAdGradSln(const FaceInfo & fi) const;

#endif

  const ADTemplateVariableSecond<OutputType> & adSecondSln() const override
  {
    checkIndexingScalingCompatibility();
    return _element_data->adSecondSln();
  }
  const ADTemplateVariableValue<OutputType> & adUDot() const override
  {
    checkIndexingScalingCompatibility();
    return _element_data->adUDot();
  }

  /// neighbor AD
  const ADTemplateVariableValue<OutputType> & adSlnNeighbor() const override
  {
    checkIndexingScalingCompatibility();
    return _neighbor_data->adSln();
  }
  const ADTemplateVariableGradient<OutputType> & adGradSlnNeighbor() const override
  {
    checkIndexingScalingCompatibility();
    return _neighbor_data->adGradSln();
  }
  const ADTemplateVariableSecond<OutputType> & adSecondSlnNeighbor() const override
  {
    checkIndexingScalingCompatibility();
    return _neighbor_data->adSecondSln();
  }
  const ADTemplateVariableValue<OutputType> & adUDotNeighbor() const override
  {
    checkIndexingScalingCompatibility();
    return _neighbor_data->adUDot();
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

  std::pair<bool, const FVDirichletBC *> getDirichletBC(const FaceInfo & fi) const;

  std::pair<bool, std::vector<const FVFluxBC *>> getFluxBCs(const FaceInfo & fi) const;

  void residualSetup() override;
  void jacobianSetup() override;

#ifdef MOOSE_GLOBAL_AD_INDEXING
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

private:
  /**
   * get the finite volume solution interpolated to the face associated with \p fi.  If the
   * neighbor is null or this variable doesn't exist on the neighbor element's subdomain, then we
   * compute a face value based on any Dirichlet boundary conditions associated with the face
   * information, or absent that we assume a zero gradient and simply return the \p elem_value
   * @param neighbor The \p neighbor element which will help us compute the face interpolation
   * @param fi The face information object
   * @param elem_value The solution value on the "element". This value will be returned as the face
   * value if there is no associated neighbor value and there is no Dirichlet boundary condition on
   * the face associated with \p fi. If there is an associated neighbor, then \p elem_value will be
   * used as part of a linear interpolation
   * @return The interpolated face value
   */
  ADReal
  getFaceValue(const Elem * const neighbor, const FaceInfo & fi, const ADReal & elem_value) const;

  /**
   * Get the finite volume solution interpolated to \p vertex. This interpolation is done doing a
   * distance-weighted average of neighboring cell center values
   * @param vertex The mesh vertex we want to interpolate the finite volume solution to
   * @return The interpolated vertex value with derivative information from the degrees of freedom
   * associated with the neighboring cell centers
   */
  const ADReal & getVertexValue(const Node & vertex) const;

public:
  /**
   * Get custom coefficients on a per-element basis. These should correspond to \p a
   * coefficients in the notation of Moukallad's "Finite Volume Method in Computational Fluid
   * Dynamics"
   */
  const ADReal &
  adCoeff(const Elem * elem, void * context, ADReal (*fn)(const Elem * const, void *)) const;
#endif

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
  /**
   * Check and see whether the AD indexing scheme is compatible with the scaling factors. Currently
   * non-unity scaling is not supported when doing global dof indexing. This is because we add
   * Jacobian entries based only on a global index, and we do not a priori know what variable, and
   * hence scaling factor, that global index is tied to. Eventually we will implement some
   * global-index-to-var lookup that we use after we've cached all of our Jacobian entries
   */
  void checkIndexingScalingCompatibility() const;

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

#ifdef MOOSE_GLOBAL_AD_INDEXING
  /// Whether we've already performed a scaling factor check for this variable
  mutable bool _scaling_params_checked = false;

  /// A cache for storing gradients on elements
  mutable std::unordered_map<const Elem *, VectorValue<ADReal>> _elem_to_grad;

  /// A cache for storing uncorrected gradients on faces
  mutable std::unordered_map<const FaceInfo *, VectorValue<ADReal>> _face_to_unc_grad;

  /// A cache for storing gradients on faces
  mutable std::unordered_map<const FaceInfo *, VectorValue<ADReal>> _face_to_grad;

  /// A cache for storing FV \p a coefficients on elements
  mutable std::unordered_map<const Elem *, ADReal> _elem_to_coeff;

  /// A cache that maps from mesh vertices to interpolated finite volume solutions at those vertices
  mutable std::unordered_map<const Node *, ADReal> _vertex_to_value;

  /// Whether to use an extended stencil for interpolating the solution to face centers. If this is
  /// true then the face center value is computed as a weighted average of connected vertices. If it
  /// is false, then the face center value is simply a linear interpolation betweeh the two
  /// neighboring cell center values
  const bool _use_extended_stencil;
#endif
};

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFV<OutputType>::adDofValues() const
{
  return _element_data->adDofValues();
}

template <typename OutputType>
inline void
MooseVariableFV<OutputType>::checkIndexingScalingCompatibility() const
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (!_scaling_params_checked)
  {
    for (const auto scaling_factor : _scaling_factor)
      if (scaling_factor != 1.)
        this->paramError("scaling", "Scaling with global AD indexing is not yet implemented.");

    _scaling_params_checked = true;
  }
#endif
}
