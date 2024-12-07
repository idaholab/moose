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
#include "MooseVariableDataLinearFV.h"

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

  /**
   * If the variable has a dirichlet boundary condition at face described by \p fi .
   */
  virtual bool isDirichletBoundaryFace(const FaceInfo & fi) const;

  /**
   * Switch to request cell gradient computations.
   */
  void computeCellGradients() { _needs_cell_gradients = true; }

  /**
   * Check if cell gradient computations were requested for this variable.
   */
  virtual bool needsGradientVectorStorage() const override { return _needs_cell_gradients; }

  virtual bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                          const Elem * elem,
                                          const Moose::StateArg & state) const override;

  /**
   * Get the variable gradient at a cell center.
   * @param elem_info The ElemInfo of the cell where we need the gradient
   */
  const VectorValue<Real> gradSln(const ElemInfo & elem_info) const;

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
   * @param elem_info The element to retrieve the solution value for
   * @param state State argument which describes at what time / solution iteration state we want to
   * evaluate the variable
   */
  Real getElemValue(const ElemInfo & elem_info, const StateArg & state) const;

  /**
   * Get the boundary condition object which corresponds to the given boundary ID
   * @param bd_id The boundary ID whose condition should be fetched
   */
  LinearFVBoundaryCondition * getBoundaryCondition(const BoundaryID bd_id) const;

  const std::unordered_map<BoundaryID, LinearFVBoundaryCondition *> & getBoundaryConditionMap()
  {
    return _boundary_id_to_bc;
  }

  virtual void prepareIC() override {}

  virtual bool isNodal() const override final { return false; }

  virtual bool hasDoFsOnNodes() const override final { return false; }

  virtual bool isNodalDefined() const override final { return false; }

  virtual bool supportsFaceArg() const override final { return true; }
  virtual bool supportsElemSideQpArg() const override final { return false; }

  virtual const Elem * const & currentElem() const override;

  virtual bool computingSecond() const override final { return false; }
  virtual bool computingCurl() const override final { return false; }
  virtual bool computingDiv() const override final { return false; }
  virtual bool usesSecondPhiNeighbor() const override final { return false; }

protected:
  /// Throw an error when somebody requests time-related data from this variable
  [[noreturn]] void timeIntegratorError() const;

  /// Throw and error when somebody requests lower-dimensional data from this variable
  [[noreturn]] void lowerDError() const;

  /// Throw an error when somebody wants to use this variable as a nodal variable
  [[noreturn]] void nodalError() const;

  /// Throw an error when somebody wants to use this variable with automatic differentiation
  [[noreturn]] void adError() const;

  /**
   * Setup the boundary to Dirichlet BC map
   */
  void cacheBoundaryBCMap();

  usingMooseVariableBaseMembers;

  /// Boolean to check if this variable needs gradient computations.
  bool _needs_cell_gradients;

  /// Temporary storage for the cell gradient to avoid unnecessary allocations.
  mutable RealVectorValue _cell_gradient;

  /// Pointer to the cell gradients which are stored on the linear system
  const std::vector<std::unique_ptr<libMesh::NumericVector<libMesh::Number>>> & _grad_container;

  /// Holder for all the data associated with the "main" element. The data in this is
  /// mainly used by finite element-based loops such as the postprocessor and auxkernel
  /// loops
  std::unique_ptr<MooseVariableDataLinearFV<OutputType>> _element_data;

  /// Holder for all the data associated with the "neighbor" element. The data in this is
  /// mainly used by finite element-based loops such as the postprocessor and auxkernel
  /// loops
  std::unique_ptr<MooseVariableDataLinearFV<OutputType>> _neighbor_data;

  /// Map for easily accessing the boundary conditions based on the boundary IDs.
  /// We assume that each boundary has one boundary condition only.
  std::unordered_map<BoundaryID, LinearFVBoundaryCondition *> _boundary_id_to_bc;

  /// Cache the number of the system this variable belongs to
  const unsigned int _sys_num;

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

  /// The current (ghosted) solution. Note that this needs to be stored as a reference to a pointer
  /// because the solution might not exist at the time that this variable is constructed, so we
  /// cannot safely dereference at that time
  const libMesh::NumericVector<libMesh::Number> * const & _solution;

  /// Shape functions, only used when we are postprocessing or using this variable
  /// in an auxiliary system
  const FieldVariablePhiValue & _phi;
  const FieldVariablePhiGradient & _grad_phi;
  const FieldVariablePhiValue & _phi_face;
  const FieldVariablePhiGradient & _grad_phi_face;
  const FieldVariablePhiValue & _phi_face_neighbor;
  const FieldVariablePhiGradient & _grad_phi_face_neighbor;
  const FieldVariablePhiValue & _phi_neighbor;
  const FieldVariablePhiGradient & _grad_phi_neighbor;

public:
  // *********************************************************************************
  // *********************************************************************************
  // The following functions are separated here because they are not essential for the
  // solver but are necessary to interface with the auxiliary and postprocessor
  // systems.
  // *********************************************************************************
  // *********************************************************************************

  virtual void setDofValue(const OutputData & /*value*/, unsigned int /*index*/) override;

  virtual void getDofIndices(const Elem * elem,
                             std::vector<dof_id_type> & dof_indices) const override;

  virtual void setDofValues(const DenseVector<OutputData> & values) override;

  virtual void clearDofIndices() override;

  virtual unsigned int numberOfDofs() const override final { return 1; }
  virtual unsigned int numberOfDofsNeighbor() override final { return 1; }

  virtual unsigned int oldestSolutionStateRequested() const override final;

  virtual void clearAllDofIndices() override final;

  [[noreturn]] virtual const std::vector<dof_id_type> & dofIndicesLower() const override final;
  [[noreturn]] virtual const FieldVariablePhiValue & phiLower() const override;

  // Overriding these to make sure nothing happens during residual/jacobian setup.
  // The only time this can actually happen is when residual setup is called on the auxiliary
  // system.
  virtual void residualSetup() override {}
  virtual void jacobianSetup() override {}

  virtual libMesh::FEContinuity getContinuity() const override
  {
    return _element_data->getContinuity();
  };

  virtual void setNodalValue(const OutputType & value, unsigned int idx = 0) override;

  [[noreturn]] virtual const DoFValue & nodalVectorTagValue(TagID) const override;

  virtual const std::vector<dof_id_type> & dofIndices() const final;
  virtual const std::vector<dof_id_type> & dofIndicesNeighbor() const final;

  virtual void prepare() override final {}
  virtual void prepareNeighbor() override final {}
  virtual void prepareAux() override final {}
  virtual void reinitNode() override final {}
  virtual void reinitNodes(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitNodesNeighbor(const std::vector<dof_id_type> & /*nodes*/) override final {}
  virtual void reinitAux() override final {}
  virtual void reinitAuxNeighbor() override final {}
  virtual void prepareLowerD() override final {}

  virtual void computeElemValuesFace() override;
  virtual void computeNeighborValuesFace() override;
  virtual void computeNeighborValues() override;
  virtual void computeLowerDValues() override final;

  virtual void computeNodalNeighborValues() override final;
  virtual void computeNodalValues() override final;

  virtual void computeElemValues() override;
  virtual void computeFaceValues(const FaceInfo & /*fi*/) override {}

  virtual void setLowerDofValues(const DenseVector<OutputData> & values) override;

  virtual void insert(libMesh::NumericVector<libMesh::Number> & vector) override;
  virtual void insertLower(libMesh::NumericVector<libMesh::Number> & vector) override;
  virtual void add(libMesh::NumericVector<libMesh::Number> & vector) override;

  virtual void setActiveTags(const std::set<TagID> & vtags) override;

  [[noreturn]] virtual const MooseArray<OutputType> & nodalValueArray() const override;
  [[noreturn]] virtual const MooseArray<OutputType> & nodalValueOldArray() const override;
  [[noreturn]] virtual const MooseArray<OutputType> & nodalValueOlderArray() const override;

  virtual const FieldVariablePhiValue & phi() const override final { return _phi; }
  virtual const FieldVariablePhiGradient & gradPhi() const override final { return _grad_phi; }
  [[noreturn]] virtual const FieldVariablePhiSecond & secondPhi() const override final;
  [[noreturn]] const FieldVariablePhiValue & curlPhi() const override final;
  [[noreturn]] const FieldVariablePhiDivergence & divPhi() const override final;

  virtual const FieldVariablePhiValue & phiFace() const override final { return _phi_face; }
  virtual const FieldVariablePhiGradient & gradPhiFace() const override final
  {
    return _grad_phi_face;
  }
  [[noreturn]] virtual const FieldVariablePhiSecond & secondPhiFace() const override final;

  virtual const FieldVariablePhiValue & phiFaceNeighbor() const override final
  {
    return _phi_face_neighbor;
  }
  virtual const FieldVariablePhiGradient & gradPhiFaceNeighbor() const override final
  {
    return _grad_phi_face_neighbor;
  }
  [[noreturn]] virtual const FieldVariablePhiSecond & secondPhiFaceNeighbor() const override final;

  virtual const FieldVariablePhiValue & phiNeighbor() const override final { return _phi_neighbor; }
  virtual const FieldVariablePhiGradient & gradPhiNeighbor() const override final
  {
    return _grad_phi_neighbor;
  }
  [[noreturn]] virtual const FieldVariablePhiSecond & secondPhiNeighbor() const override final;

  virtual const FieldVariableValue & vectorTagValue(TagID tag) const override;
  virtual const DoFValue & vectorTagDofValue(TagID tag) const override;
  [[noreturn]] virtual const DoFValue & nodalMatrixTagValue(TagID tag) const override;
  virtual const FieldVariableValue & matrixTagValue(TagID tag) const override;

  virtual const FieldVariableValue & sln() const override;
  virtual const FieldVariableValue & slnOld() const override;
  virtual const FieldVariableValue & slnOlder() const override;
  virtual const FieldVariableGradient & gradSln() const override;
  virtual const FieldVariableGradient & gradSlnOld() const override;
  virtual const FieldVariableValue & slnNeighbor() const override;
  virtual const FieldVariableValue & slnOldNeighbor() const override;
  virtual const FieldVariableGradient & gradSlnNeighbor() const override;
  virtual const FieldVariableGradient & gradSlnOldNeighbor() const override;

  [[noreturn]] virtual const ADTemplateVariableSecond<OutputType> & adSecondSln() const override;
  [[noreturn]] virtual const ADTemplateVariableValue<OutputType> & adUDot() const override;
  [[noreturn]] virtual const ADTemplateVariableValue<OutputType> & adUDotDot() const override;
  [[noreturn]] virtual const ADTemplateVariableGradient<OutputType> & adGradSlnDot() const override;
  [[noreturn]] virtual const ADTemplateVariableValue<OutputType> & adSlnNeighbor() const override;
  [[noreturn]] virtual const ADTemplateVariableGradient<OutputType> &
  adGradSlnNeighbor() const override;
  [[noreturn]] virtual const ADTemplateVariableSecond<OutputType> &
  adSecondSlnNeighbor() const override;
  [[noreturn]] virtual const ADTemplateVariableValue<OutputType> & adUDotNeighbor() const override;
  [[noreturn]] virtual const ADTemplateVariableValue<OutputType> &
  adUDotDotNeighbor() const override;
  [[noreturn]] virtual const ADTemplateVariableGradient<OutputType> &
  adGradSlnNeighborDot() const override;
  [[noreturn]] virtual const ADTemplateVariableValue<OutputType> & adSln() const override;
  [[noreturn]] virtual const ADTemplateVariableGradient<OutputType> & adGradSln() const override;

  virtual const DoFValue & dofValues() const override;
  virtual const DoFValue & dofValuesOld() const override;

  virtual const DoFValue & dofValuesOlder() const override;
  virtual const DoFValue & dofValuesPreviousNL() const override;
  virtual const DoFValue & dofValuesNeighbor() const override;
  virtual const DoFValue & dofValuesOldNeighbor() const override;
  virtual const DoFValue & dofValuesOlderNeighbor() const override;
  virtual const DoFValue & dofValuesPreviousNLNeighbor() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDot() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDotNeighbor() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDotOld() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDotOldNeighbor() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDotDot() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDotDotNeighbor() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDotDotOld() const override;
  [[noreturn]] virtual const DoFValue & dofValuesDotDotOldNeighbor() const override;
  [[noreturn]] virtual const MooseArray<libMesh::Number> & dofValuesDuDotDu() const override;
  [[noreturn]] virtual const MooseArray<libMesh::Number> &
  dofValuesDuDotDuNeighbor() const override;
  [[noreturn]] virtual const MooseArray<libMesh::Number> & dofValuesDuDotDotDu() const override;
  [[noreturn]] virtual const MooseArray<libMesh::Number> &
  dofValuesDuDotDotDuNeighbor() const override;

  [[noreturn]] virtual const MooseArray<ADReal> & adDofValues() const override;
  [[noreturn]] virtual const MooseArray<ADReal> & adDofValuesNeighbor() const override;
  [[noreturn]] virtual const MooseArray<ADReal> & adDofValuesDot() const override;
  [[noreturn]] virtual const dof_id_type & nodalDofIndex() const override final;
  [[noreturn]] virtual const dof_id_type & nodalDofIndexNeighbor() const override final;

  virtual std::size_t phiSize() const override final { return _phi.size(); }
  virtual std::size_t phiFaceSize() const override final { return _phi_face.size(); }
  virtual std::size_t phiNeighborSize() const override final { return _phi_neighbor.size(); }
  virtual std::size_t phiFaceNeighborSize() const override final
  {
    return _phi_face_neighbor.size();
  }
  [[noreturn]] virtual std::size_t phiLowerSize() const override final;
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

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::timeIntegratorError() const
{
  mooseError("MooseLinearVariableFV does not support time integration at the moment! The variable "
             "which is causing the issue: ",
             this->name());
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::lowerDError() const
{
  mooseError("Lower dimensional element support not implemented for finite volume variables!The "
             "variable which is causing the issue: ",
             this->name());
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::nodalError() const
{
  mooseError("FV variables don't support nodal variable treatment! The variable which is causing "
             "the issue: ",
             this->name());
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::adError() const
{
  mooseError("Linear FV variable does not support automatic differentiation, the variable which is "
             "attempting it is: ",
             this->name());
}

// Declare all the specializations, as the template specialization declarations below must know
template <>
ADReal MooseLinearVariableFV<Real>::evaluateDot(const ElemArg & elem, const StateArg & state) const;

// Prevent implicit instantiation in other translation units where these classes are used
extern template class MooseLinearVariableFV<Real>;
