//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseLinearVariableFV.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"
#include "DisplacedSystem.h"
#include "SystemBase.h"
#include "LinearSystem.h"
#include "AuxiliarySystem.h"
#include "FEProblemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "MathFVUtils.h"
#include "FVUtils.h"
#include "FVFluxBC.h"
#include "FVDirichletBCBase.h"
#include "GreenGaussGradient.h"
#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"
#include "GradientLimiterType.h"
#include "FVGradientMethod.h"

#include "libmesh/numeric_vector.h"

#include <climits>
#include <typeinfo>

using namespace Moose;

registerMooseObject("MooseApp", MooseLinearVariableFVReal);

namespace
{
const FVGradientMethod &
getLinearFVGradientMethod(SystemBase & sys, const GradientMethodName & method_name)
{
  auto & fe_problem = sys.feProblem();
  if (method_name == "green-gauss" && !fe_problem.hasFVGradientMethod(method_name))
    fe_problem.addFVGradientMethod("FVGreenGaussGradient", method_name);

  if (!fe_problem.hasFVGradientMethod(method_name))
    mooseError("Unable to find FVGradientMethod with name '", method_name, "'");

  return fe_problem.getFVGradientMethod(method_name);
}

}

template <typename OutputType>
InputParameters
MooseLinearVariableFV<OutputType>::validParams()
{
  InputParameters params = MooseVariableField<OutputType>::validParams();
  params.set<bool>("fv") = true;
  params.set<MooseEnum>("family") = "MONOMIAL";
  params.set<MooseEnum>("order") = "CONSTANT";
  params.addParam<GradientMethodName>(
      "gradient_method",
      "green-gauss",
      "Default gradient computation method to register when a consumer requests gradients from "
      "this variable. This may be 'green-gauss' or the name of an object in [FVGradientMethods].");
  return params;
}

template <typename OutputType>
MooseLinearVariableFV<OutputType>::MooseLinearVariableFV(const InputParameters & parameters)
  : MooseVariableField<OutputType>(parameters),
    _needs_cell_gradients(false),
    _linear_system(dynamic_cast<LinearSystem *>(&this->_sys)),
    _auxiliary_system(dynamic_cast<AuxiliarySystem *>(&this->_sys)),
    _gradient_field(nullptr),
    _default_gradient_method_name(this->template getParam<GradientMethodName>("gradient_method")),
    _sys_num(this->_sys.number()),
    _solution(this->_sys.currentSolution()),
    // The following members are needed to be able to interface with the postprocessor and
    // auxiliary systems
    _phi(this->_assembly.template fePhi<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _grad_phi(this->_assembly.template feGradPhi<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _phi_face(this->_assembly.template fePhiFace<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _grad_phi_face(this->_assembly.template feGradPhiFace<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _phi_face_neighbor(
        this->_assembly.template fePhiFaceNeighbor<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _grad_phi_face_neighbor(
        this->_assembly.template feGradPhiFaceNeighbor<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _phi_neighbor(this->_assembly.template fePhiNeighbor<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _grad_phi_neighbor(
        this->_assembly.template feGradPhiNeighbor<OutputShape>(FEType(CONSTANT, MONOMIAL)))
{
  if (!_linear_system && !_auxiliary_system)
    this->paramError("solver_sys",
                     "The assigned system is not a linear or an auxiliary system! Linear variables "
                     "can only be assigned to linear or auxiliary systems!");
  _element_data = std::make_unique<MooseVariableDataLinearFV<OutputType>>(
      *this, _sys, _tid, Moose::ElementType::Element, this->_assembly.elem());
  _neighbor_data = std::make_unique<MooseVariableDataLinearFV<OutputType>>(
      *this, _sys, _tid, Moose::ElementType::Neighbor, this->_assembly.neighbor());

  if (libMesh::n_threads() > 1)
    mooseError("MooseLinearVariableFV does not support threading at the moment!");
}

template <typename OutputType>
const LinearFVGradientField &
MooseLinearVariableFV<OutputType>::computeCellGradients()
{
  const auto & method = getLinearFVGradientMethod(this->_sys, _default_gradient_method_name);
  const auto & field = computeCellGradients(method);
  _gradient_field = &field;
  return field;
}

template <typename OutputType>
const LinearFVGradientField &
MooseLinearVariableFV<OutputType>::computeCellGradients(const FVGradientMethod & method)
{
  return registerCellGradientMethod(method);
}

template <typename OutputType>
const LinearFVGradientField &
MooseLinearVariableFV<OutputType>::computeCellGradients(
    const Moose::FV::GradientLimiterType limiter_type)
{
  if (limiter_type == Moose::FV::GradientLimiterType::None)
    return computeCellGradients();

  const auto & method = getLinearFVGradientMethod(this->_sys, _default_gradient_method_name);
  if (method.limiterType() != limiter_type)
    mooseError("Variable '",
               this->name(),
               "' requested limited gradients with a limiter that differs from its "
               "gradient_method. Set the limiter on the [FVGradientMethods] object instead.");

  return computeCellGradients();
}

template <typename OutputType>
const LinearFVGradientField &
MooseLinearVariableFV<OutputType>::registerCellGradientMethod(const FVGradientMethod & method)
{
  _needs_cell_gradients = true;

  const auto it = _gradient_field_cache.find(&method);
  if (it != _gradient_field_cache.end())
    return *it->second;

  auto & gradient_field = _linear_system
                              ? _linear_system->registerFVGradient(this->_var_num, method)
                              : _auxiliary_system->registerFVGradient(this->_var_num, method);
  _gradient_field_cache.emplace(&method, &gradient_field);
  return gradient_field;
}

template <typename OutputType>
bool
MooseLinearVariableFV<OutputType>::isExtrapolatedBoundaryFace(
    const FaceInfo & /*fi*/, const Elem * const /*elem*/, const Moose::StateArg & /*state*/) const
{
  /// This is not used by this variable at this point.
  return false;
}

template <typename OutputType>
Real
MooseLinearVariableFV<OutputType>::getElemValue(const ElemInfo & elem_info,
                                                const StateArg & state) const
{
  mooseAssert(
      this->hasBlocks(elem_info.subdomain_id()),
      "The variable should be defined on the element's subdomain! This typically occurs when the "
      "user wants to evaluate the elements right next to the boundary of two variables (block "
      "boundary). The subdomain which is queried: " +
          Moose::stringify(this->activeSubdomains()) + " the subdomain of the element " +
          std::to_string(elem_info.subdomain_id()));

  // It's not safe to use solutionState(0) because it returns the libMesh System solution member
  // which is wrong during things like finite difference Jacobian evaluation, e.g. when PETSc
  // perturbs the solution vector we feed these perturbations into the current_local_solution
  // while the libMesh solution is frozen in the non-perturbed state
  const auto & global_soln = (state.state == 0)
                                 ? *this->_sys.currentSolution()
                                 : this->_sys.solutionState(state.state, state.iteration_type);

  return global_soln(elem_info.dofIndices()[this->_sys_num][this->_var_num]);
}

template <typename OutputType>
VectorValue<Real>
MooseLinearVariableFV<OutputType>::gradSln(const ElemInfo & elem_info, const StateArg & state) const
{
  mooseAssert(_gradient_field, "Gradient requested without calling computeCellGradients().");
  if (state.state != 0)
    gradientStateError(state);

  return _gradient_field->gradient(elem_info);
}

template <typename OutputType>
Real
MooseLinearVariableFV<OutputType>::gradSlnComponent(const ElemInfo & elem_info,
                                                    const unsigned int component) const
{
  mooseAssert(_gradient_field,
              "Gradient component requested without calling computeCellGradients().");
  return _gradient_field->component(elem_info, component);
}

template <typename OutputType>
VectorValue<Real>
MooseLinearVariableFV<OutputType>::gradSln(const FaceInfo & fi, const StateArg & state) const
{
  mooseAssert(_gradient_field, "Gradient requested without calling computeCellGradients().");
  if (state.state != 0)
    gradientStateError(state);

  return _gradient_field->gradient(fi);
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::initialSetup()
{
  MooseVariableField<OutputType>::initialSetup();
  cacheBoundaryBCMap();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::timestepSetup()
{
  MooseVariableField<OutputType>::timestepSetup();
  cacheBoundaryBCMap();
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const FaceArg & face, const StateArg & state) const
{
  const FaceInfo * const fi = face.fi;

  mooseAssert(fi, "The face information must be non-null");

  const auto face_type = fi->faceType(std::make_pair(this->_var_num, this->_sys_num));

  if (face_type == FaceInfo::VarFaceNeighbors::BOTH)
    return Moose::FV::interpolate(*this, face, state);
  else if (auto * bc_pointer = this->getBoundaryCondition(*fi->boundaryIDs().begin()))
  {
    mooseAssert(fi->boundaryIDs().size() == 1, "We should only have one boundary on every face.");
    bc_pointer->setupFaceData(fi, face_type);
    return bc_pointer->computeBoundaryValue();
  }
  // If no boundary condition is defined but we are evaluating on a boundary, just return the
  // element value
  else if (face_type == FaceInfo::VarFaceNeighbors::ELEM)
  {
    const auto & elem_info = this->_mesh.elemInfo(fi->elemPtr()->id());
    return getElemValue(elem_info, state);
  }
  else if (face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    const auto & elem_info = this->_mesh.elemInfo(fi->neighborPtr()->id());
    return getElemValue(elem_info, state);
  }
  else
    mooseError("We should never get here!");
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::ValueType
MooseLinearVariableFV<OutputType>::evaluate(const NodeArg & /*node_arg*/,
                                            const StateArg & /*state*/) const
{
  mooseError("Not implemented yet");
}

template <typename OutputType>
typename MooseLinearVariableFV<OutputType>::DotType
MooseLinearVariableFV<OutputType>::evaluateDot(const ElemArg &, const StateArg &) const
{
  timeIntegratorError();
}

template <>
ADReal
MooseLinearVariableFV<Real>::evaluateDot(const ElemArg & /*elem_arg*/,
                                         const StateArg & /*state*/) const
{
  timeIntegratorError();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::cacheBoundaryBCMap()
{
  _boundary_id_to_bc.clear();
  std::vector<LinearFVBoundaryCondition *> bcs;

  // I believe because query() returns by value but condition returns by reference that binding to a
  // const lvalue reference results in the query() getting destructed and us holding onto a dangling
  // reference. I think that condition returned by value we would be able to bind to a const lvalue
  // reference here. But as it is we'll bind to a regular lvalue
  auto base_query = this->_subproblem.getMooseApp()
                        .theWarehouse()
                        .query()
                        .template condition<AttribSystem>("LinearFVBoundaryCondition")
                        .template condition<AttribThread>(_tid)
                        .template condition<AttribVar>(_var_num)
                        .template condition<AttribSysNum>(this->_sys.number());

  for (const auto bnd_id : this->_mesh.getBoundaryIDs())
  {
    auto base_query_copy = base_query;
    base_query_copy.template condition<AttribBoundaries>(std::set<BoundaryID>({bnd_id}))
        .queryInto(bcs);
    mooseAssert(bcs.size() <= 1, "cannot have multiple BCs on the same boundary");
    if (!bcs.empty())
      _boundary_id_to_bc.emplace(bnd_id, bcs[0]);
  }
}

template <typename OutputType>
LinearFVBoundaryCondition *
MooseLinearVariableFV<OutputType>::getBoundaryCondition(const BoundaryID bd_id) const
{
  const auto iter = _boundary_id_to_bc.find(bd_id);
  if (iter == _boundary_id_to_bc.end())
    return nullptr;
  else
    return iter->second;
}

template <typename OutputType>
const Elem * const &
MooseLinearVariableFV<OutputType>::currentElem() const
{
  return this->_assembly.elem();
}

template <typename OutputType>
bool
MooseLinearVariableFV<OutputType>::isDirichletBoundaryFace(const FaceInfo & fi) const
{
  for (const auto bnd_id : fi.boundaryIDs())
    if (auto it = _boundary_id_to_bc.find(bnd_id); it != _boundary_id_to_bc.end())
      if (dynamic_cast<LinearFVAdvectionDiffusionFunctorDirichletBC *>(it->second))
        return true;

  return false;
}

// ****************************************************************************
// The functions below are used for interfacing the auxiliary and
// postprocessor/userobject systems. Most of the postprocessors/
// auxkernels require quadrature-based evaluations and we provide that
// interface with the functions below.
// ****************************************************************************

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::getDofIndices(const Elem * elem,
                                                 std::vector<dof_id_type> & dof_indices) const
{
  dof_indices.clear();
  const auto & elem_info = this->_mesh.elemInfo(elem->id());
  dof_indices.push_back(elem_info.dofIndices()[this->_sys_num][this->number()]);
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::setDofValue(const DofValue & value, unsigned int index)
{
  _element_data->setDofValue(value, index);
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::setDofValues(const DenseVector<DofValue> & values)
{
  _element_data->setDofValues(values);
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::clearDofIndices()
{
  _element_data->clearDofIndices();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValues() const
{
  return _element_data->dofValues();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesNeighbor() const
{
  return _neighbor_data->dofValues();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesOld() const
{
  return _element_data->dofValuesOld();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesOlder() const
{
  return _element_data->dofValuesOlder();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesPreviousNL() const
{
  return _element_data->dofValuesPreviousNL();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesOldNeighbor() const
{
  return _neighbor_data->dofValuesOld();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesOlderNeighbor() const
{
  return _neighbor_data->dofValuesOlder();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesPreviousNLNeighbor() const
{
  return _neighbor_data->dofValuesPreviousNL();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDot() const
{
  timeIntegratorError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDotDot() const
{
  timeIntegratorError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDotOld() const
{
  timeIntegratorError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDotDotOld() const
{
  timeIntegratorError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDotNeighbor() const
{
  timeIntegratorError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDotDotNeighbor() const
{
  timeIntegratorError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDotOldNeighbor() const
{
  timeIntegratorError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::dofValuesDotDotOldNeighbor() const
{
  timeIntegratorError();
}

template <typename OutputType>
const MooseArray<Number> &
MooseLinearVariableFV<OutputType>::dofValuesDuDotDu() const
{
  timeIntegratorError();
}

template <typename OutputType>
const MooseArray<Number> &
MooseLinearVariableFV<OutputType>::dofValuesDuDotDotDu() const
{
  timeIntegratorError();
}

template <typename OutputType>
const MooseArray<Number> &
MooseLinearVariableFV<OutputType>::dofValuesDuDotDuNeighbor() const
{
  timeIntegratorError();
}

template <typename OutputType>
const MooseArray<Number> &
MooseLinearVariableFV<OutputType>::dofValuesDuDotDotDuNeighbor() const
{
  timeIntegratorError();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::computeElemValues()
{
  _element_data->setGeometry(Moose::Volume);
  _element_data->computeValues();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::computeElemValuesFace()
{
  _element_data->setGeometry(Moose::Face);
  _element_data->computeValues();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::computeNeighborValuesFace()
{
  _neighbor_data->setGeometry(Moose::Face);
  _neighbor_data->computeValues();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::computeNeighborValues()
{
  _neighbor_data->setGeometry(Moose::Volume);
  _neighbor_data->computeValues();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::computeLowerDValues()
{
  lowerDError();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::computeNodalNeighborValues()
{
  nodalError();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::computeNodalValues()
{
  nodalError();
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseLinearVariableFV<OutputType>::dofIndices() const
{
  return _element_data->dofIndices();
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseLinearVariableFV<OutputType>::dofIndicesNeighbor() const
{
  return _neighbor_data->dofIndices();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::setLowerDofValues(const DenseVector<DofValue> &)
{
  lowerDError();
}

template <typename OutputType>
unsigned int
MooseLinearVariableFV<OutputType>::oldestSolutionStateRequested() const
{
  unsigned int state = 0;
  state = std::max(state, _element_data->oldestSolutionStateRequested());
  state = std::max(state, _neighbor_data->oldestSolutionStateRequested());
  return state;
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::clearAllDofIndices()
{
  _element_data->clearDofIndices();
  _neighbor_data->clearDofIndices();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::setNodalValue(const OutputType & /*value*/, unsigned int /*idx*/)
{
  nodalError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::nodalVectorTagValue(TagID) const
{
  nodalError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::nodalMatrixTagValue(TagID) const
{
  nodalError();
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseLinearVariableFV<OutputType>::dofIndicesLower() const
{
  lowerDError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiValue &
MooseLinearVariableFV<OutputType>::phiLower() const
{
  lowerDError();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::insert(NumericVector<Number> & vector)
{
  _element_data->insert(vector);
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::insertLower(NumericVector<Number> & /*residual*/)
{
  mooseError("We don't support value insertion to residuals in MooseLinearVariableFV!");
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::add(NumericVector<Number> & /*residual*/)
{
  mooseError("We don't support value addition to residuals in MooseLinearVariableFV!");
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::setActiveTags(const std::set<TagID> & vtags)
{
  _element_data->setActiveTags(vtags);
  _neighbor_data->setActiveTags(vtags);
}

template <typename OutputType>
const MooseArray<OutputType> &
MooseLinearVariableFV<OutputType>::nodalValueArray() const
{
  nodalError();
}

template <typename OutputType>
const MooseArray<OutputType> &
MooseLinearVariableFV<OutputType>::nodalValueOldArray() const
{
  nodalError();
}

template <typename OutputType>
const MooseArray<OutputType> &
MooseLinearVariableFV<OutputType>::nodalValueOlderArray() const
{
  nodalError();
}

template <typename OutputType>
std::size_t
MooseLinearVariableFV<OutputType>::phiLowerSize() const
{
  lowerDError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableValue &
MooseLinearVariableFV<OutputType>::vectorTagValue(TagID tag) const
{
  return _element_data->vectorTagValue(tag);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::DofValues &
MooseLinearVariableFV<OutputType>::vectorTagDofValue(TagID tag) const
{
  return _element_data->vectorTagDofValue(tag);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableValue &
MooseLinearVariableFV<OutputType>::matrixTagValue(TagID tag) const
{
  return _element_data->matrixTagValue(tag);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiSecond &
MooseLinearVariableFV<OutputType>::secondPhi() const
{
  mooseError("We don't currently implement second derivatives for FV");
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiValue &
MooseLinearVariableFV<OutputType>::curlPhi() const
{
  mooseError("We don't currently implement curl for FV");
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiDivergence &
MooseLinearVariableFV<OutputType>::divPhi() const
{
  mooseError("We don't currently implement divergence for FV");
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiSecond &
MooseLinearVariableFV<OutputType>::secondPhiFace() const
{
  mooseError("We don't currently implement second derivatives for FV");
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiSecond &
MooseLinearVariableFV<OutputType>::secondPhiFaceNeighbor() const
{
  mooseError("We don't currently implement second derivatives for FV");
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariablePhiSecond &
MooseLinearVariableFV<OutputType>::secondPhiNeighbor() const
{
  mooseError("We don't currently implement second derivatives for FV");
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableValue &
MooseLinearVariableFV<OutputType>::sln() const
{
  return _element_data->sln(Moose::Current);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableValue &
MooseLinearVariableFV<OutputType>::slnOld() const
{
  return _element_data->sln(Moose::Old);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableValue &
MooseLinearVariableFV<OutputType>::slnOlder() const
{
  return _element_data->sln(Moose::Older);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableGradient &
MooseLinearVariableFV<OutputType>::gradSln() const
{
  return _element_data->gradSln(Moose::Current);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableGradient &
MooseLinearVariableFV<OutputType>::gradSlnOld() const
{
  return _element_data->gradSln(Moose::Old);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableValue &
MooseLinearVariableFV<OutputType>::slnNeighbor() const
{
  return _neighbor_data->sln(Moose::Current);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableValue &
MooseLinearVariableFV<OutputType>::slnOldNeighbor() const
{
  return _neighbor_data->sln(Moose::Old);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableGradient &
MooseLinearVariableFV<OutputType>::gradSlnNeighbor() const
{
  return _neighbor_data->gradSln(Moose::Current);
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::FieldVariableGradient &
MooseLinearVariableFV<OutputType>::gradSlnOldNeighbor() const
{
  return _neighbor_data->gradSln(Moose::Old);
}

template <typename OutputType>
const ADTemplateVariableSecond<OutputType> &
MooseLinearVariableFV<OutputType>::adSecondSln() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableValue<OutputType> &
MooseLinearVariableFV<OutputType>::adUDot() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableValue<OutputType> &
MooseLinearVariableFV<OutputType>::adUDotDot() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableGradient<OutputType> &
MooseLinearVariableFV<OutputType>::adGradSlnDot() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableValue<OutputType> &
MooseLinearVariableFV<OutputType>::adSlnNeighbor() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableGradient<OutputType> &
MooseLinearVariableFV<OutputType>::adGradSlnNeighbor() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableSecond<OutputType> &
MooseLinearVariableFV<OutputType>::adSecondSlnNeighbor() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableValue<OutputType> &
MooseLinearVariableFV<OutputType>::adUDotNeighbor() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableValue<OutputType> &
MooseLinearVariableFV<OutputType>::adUDotDotNeighbor() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableGradient<OutputType> &
MooseLinearVariableFV<OutputType>::adGradSlnNeighborDot() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableValue<OutputType> &
MooseLinearVariableFV<OutputType>::adSln() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableGradient<OutputType> &
MooseLinearVariableFV<OutputType>::adGradSln() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableCurl<OutputType> &
MooseLinearVariableFV<OutputType>::adCurlSln() const
{
  adError();
}

template <typename OutputType>
const ADTemplateVariableCurl<OutputType> &
MooseLinearVariableFV<OutputType>::adCurlSlnNeighbor() const
{
  adError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::ADDofValues &
MooseLinearVariableFV<OutputType>::adDofValues() const
{
  adError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::ADDofValues &
MooseLinearVariableFV<OutputType>::adDofValuesNeighbor() const
{
  adError();
}

template <typename OutputType>
const typename MooseLinearVariableFV<OutputType>::ADDofValues &
MooseLinearVariableFV<OutputType>::adDofValuesDot() const
{
  adError();
}

template <typename OutputType>
const dof_id_type &
MooseLinearVariableFV<OutputType>::nodalDofIndex() const
{
  nodalError();
}

template <typename OutputType>
const dof_id_type &
MooseLinearVariableFV<OutputType>::nodalDofIndexNeighbor() const
{
  nodalError();
}

template <typename OutputType>
void
MooseLinearVariableFV<OutputType>::sizeMatrixTagData()
{
  _element_data->sizeMatrixTagData();
}

template class MooseLinearVariableFV<Real>;
