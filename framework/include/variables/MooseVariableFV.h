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
  virtual size_t phiSize() const override final
  {
    mooseError("phiSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiFaceSize() const override final
  {
    mooseError("phiFaceSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiNeighborSize() const override final
  {
    mooseError("phiNeighborSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiFaceNeighborSize() const override final
  {
    mooseError("phiFaceNeighborSize not supported by MooseVariableFVBase");
  }
  virtual size_t phiLowerSize() const override final
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
  void setDofValues(const DenseVector<OutputData> & values);

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

protected:
  usingMooseVariableBaseMembers;

  /// Holder for all the data associated with the "main" element
  std::unique_ptr<MooseVariableDataFV<OutputType>> _element_data;

  /// Holder for all the data associated with the neighbor element
  std::unique_ptr<MooseVariableDataFV<OutputType>> _neighbor_data;
};

template <typename OutputType>
inline const MooseArray<ADReal> &
MooseVariableFV<OutputType>::adDofValues() const
{
  return _element_data->adDofValues();
}
