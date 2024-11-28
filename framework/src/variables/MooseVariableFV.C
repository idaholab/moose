//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFV.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"
#include "DisplacedSystem.h"
#include "SystemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "MathFVUtils.h"
#include "FVUtils.h"
#include "FVFluxBC.h"
#include "FVDirichletBCBase.h"
#include "GreenGaussGradient.h"

#include "libmesh/numeric_vector.h"

#include <climits>
#include <typeinfo>

using namespace Moose;

registerMooseObject("MooseApp", MooseVariableFVReal);

template <typename OutputType>
InputParameters
MooseVariableFV<OutputType>::validParams()
{
  InputParameters params = MooseVariableField<OutputType>::validParams();
  params.set<bool>("fv") = true;
  params.set<MooseEnum>("family") = "MONOMIAL";
  params.set<MooseEnum>("order") = "CONSTANT";
  params.template addParam<bool>(
      "two_term_boundary_expansion",
      true,
      "Whether to use a two-term Taylor expansion to calculate boundary face values. "
      "If the two-term expansion is used, then the boundary face value depends on the "
      "adjoining cell center gradient, which itself depends on the boundary face value. "
      "Consequently an implicit solve is used to simultaneously solve for the adjoining cell "
      "center gradient and boundary face value(s).");
  MooseEnum face_interp_method("average skewness-corrected", "average");
  params.template addParam<MooseEnum>("face_interp_method",
                                      face_interp_method,
                                      "Switch that can select between face interpolation methods.");
  params.template addParam<bool>(
      "cache_cell_gradients", true, "Whether to cache cell gradients or re-compute them.");
  // Just evaluating finite volume variables at an arbitrary location in a cell requires a layer of
  // ghosting since we will use two term expansions
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  return params;
}

template <typename OutputType>
MooseVariableFV<OutputType>::MooseVariableFV(const InputParameters & parameters)
  : MooseVariableField<OutputType>(parameters),
    _solution(this->_sys.currentSolution()),
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
        this->_assembly.template feGradPhiNeighbor<OutputShape>(FEType(CONSTANT, MONOMIAL))),
    _prev_elem(nullptr),
    _two_term_boundary_expansion(this->isParamValid("two_term_boundary_expansion")
                                     ? this->template getParam<bool>("two_term_boundary_expansion")
                                     : true),
    _cache_cell_gradients(this->isParamValid("cache_cell_gradients")
                              ? this->template getParam<bool>("cache_cell_gradients")
                              : true)
{
  _element_data = std::make_unique<MooseVariableDataFV<OutputType>>(
      *this, _sys, _tid, Moose::ElementType::Element, this->_assembly.elem());
  _neighbor_data = std::make_unique<MooseVariableDataFV<OutputType>>(
      *this, _sys, _tid, Moose::ElementType::Neighbor, this->_assembly.neighbor());

  if (this->isParamValid("face_interp_method"))
  {
    const auto & interp_method = this->template getParam<MooseEnum>("face_interp_method");
    if (interp_method == "average")
      _face_interp_method = Moose::FV::InterpMethod::Average;
    else if (interp_method == "skewness-corrected")
      _face_interp_method = Moose::FV::InterpMethod::SkewCorrectedAverage;
  }
  else
    _face_interp_method = Moose::FV::InterpMethod::Average;
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::clearDofIndices()
{
  _element_data->clearDofIndices();
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::OutputData
MooseVariableFV<OutputType>::getElementalValue(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Current, idx);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::OutputData
MooseVariableFV<OutputType>::getElementalValueOld(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Old, idx);
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::OutputData
MooseVariableFV<OutputType>::getElementalValueOlder(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Older, idx);
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::insert(NumericVector<Number> & residual)
{
  _element_data->insert(residual);
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::insertLower(NumericVector<Number> &)
{
  lowerDError();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::add(NumericVector<Number> & residual)
{
  _element_data->add(residual);
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValues() const
{
  return _element_data->dofValues();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesOld() const
{
  return _element_data->dofValuesOld();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesOlder() const
{
  return _element_data->dofValuesOlder();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesPreviousNL() const
{
  return _element_data->dofValuesPreviousNL();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesNeighbor() const
{
  return _neighbor_data->dofValues();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesOldNeighbor() const
{
  return _neighbor_data->dofValuesOld();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesOlderNeighbor() const
{
  return _neighbor_data->dofValuesOlder();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesPreviousNLNeighbor() const
{
  return _neighbor_data->dofValuesPreviousNL();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDot() const
{
  return _element_data->dofValuesDot();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDotDot() const
{
  return _element_data->dofValuesDotDot();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDotOld() const
{
  return _element_data->dofValuesDotOld();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDotDotOld() const
{
  return _element_data->dofValuesDotDotOld();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDotNeighbor() const
{
  return _neighbor_data->dofValuesDot();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDotDotNeighbor() const
{
  return _neighbor_data->dofValuesDotDot();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDotOldNeighbor() const
{
  return _neighbor_data->dofValuesDotOld();
}

template <typename OutputType>
const typename MooseVariableFV<OutputType>::DoFValue &
MooseVariableFV<OutputType>::dofValuesDotDotOldNeighbor() const
{
  return _neighbor_data->dofValuesDotDotOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFV<OutputType>::dofValuesDuDotDu() const
{
  return _element_data->dofValuesDuDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFV<OutputType>::dofValuesDuDotDotDu() const
{
  return _element_data->dofValuesDuDotDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFV<OutputType>::dofValuesDuDotDuNeighbor() const
{
  return _neighbor_data->dofValuesDuDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFV<OutputType>::dofValuesDuDotDotDuNeighbor() const
{
  return _neighbor_data->dofValuesDuDotDotDu();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::prepareIC()
{
  _element_data->prepareIC();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::computeElemValues()
{
  _element_data->setGeometry(Moose::Volume);
  _element_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::computeElemValuesFace()
{
  _element_data->setGeometry(Moose::Face);
  _element_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::computeNeighborValuesFace()
{
  _neighbor_data->setGeometry(Moose::Face);
  _neighbor_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::computeNeighborValues()
{
  _neighbor_data->setGeometry(Moose::Volume);
  _neighbor_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::computeFaceValues(const FaceInfo & fi)
{
  _element_data->setGeometry(Moose::Face);
  _neighbor_data->setGeometry(Moose::Face);

  const auto facetype = fi.faceType(std::make_pair(this->number(), this->sys().number()));
  if (facetype == FaceInfo::VarFaceNeighbors::NEITHER)
    return;
  else if (facetype == FaceInfo::VarFaceNeighbors::BOTH)
  {
    _element_data->computeValuesFace(fi);
    _neighbor_data->computeValuesFace(fi);
  }
  else if (facetype == FaceInfo::VarFaceNeighbors::ELEM)
    _element_data->computeValuesFace(fi);
  else if (facetype == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _neighbor_data->computeValuesFace(fi);
  else
    mooseError("robert wrote broken MooseVariableFV code");
}

template <typename OutputType>
OutputType
MooseVariableFV<OutputType>::getValue(const Elem * elem) const
{
  Moose::initDofIndices(const_cast<MooseVariableFV<OutputType> &>(*this), *elem);
  mooseAssert(this->_dof_indices.size() == 1, "Wrong size for dof indices");
  OutputType value = (*this->_sys.currentSolution())(this->_dof_indices[0]);
  return value;
}

template <typename OutputType>
typename OutputTools<OutputType>::OutputGradient
MooseVariableFV<OutputType>::getGradient(const Elem * /*elem*/) const
{
  return {};
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::setNodalValue(const OutputType & /*value*/, unsigned int /*idx*/)
{
  mooseError("FV variables do not support setNodalValue");
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::setDofValue(const OutputData & value, unsigned int index)
{
  _element_data->setDofValue(value, index);
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::setDofValues(const DenseVector<OutputData> & values)
{
  _element_data->setDofValues(values);
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::setLowerDofValues(const DenseVector<OutputData> &)
{
  lowerDError();
}

template <typename OutputType>
std::pair<bool, const FVDirichletBCBase *>
MooseVariableFV<OutputType>::getDirichletBC(const FaceInfo & fi) const
{
  if (!_dirichlet_map_setup)
    const_cast<MooseVariableFV<OutputType> *>(this)->determineBoundaryToDirichletBCMap();

  for (const auto bnd_id : fi.boundaryIDs())
    if (auto it = _boundary_id_to_dirichlet_bc.find(bnd_id);
        it != _boundary_id_to_dirichlet_bc.end())
      return {true, it->second};

  return {false, nullptr};
}

template <typename OutputType>
std::pair<bool, std::vector<const FVFluxBC *>>
MooseVariableFV<OutputType>::getFluxBCs(const FaceInfo & fi) const
{
  std::vector<const FVFluxBC *> bcs;

  this->_subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribSystem>("FVFluxBC")
      .template condition<AttribThread>(_tid)
      .template condition<AttribBoundaries>(fi.boundaryIDs())
      .template condition<AttribVar>(_var_num)
      .template condition<AttribSysNum>(this->_sys.number())
      .queryInto(bcs);

  bool has_flux_bc = bcs.size() > 0;

  if (has_flux_bc)
    return std::make_pair(true, bcs);
  else
    return std::make_pair(false, std::vector<const FVFluxBC *>());
}

template <typename OutputType>
ADReal
MooseVariableFV<OutputType>::getElemValue(const Elem * const elem, const StateArg & state) const
{
  mooseAssert(elem,
              "The elem shall exist! This typically occurs when the "
              "user wants to evaluate non-existing elements (nullptr) at physical boundaries.");
  mooseAssert(
      this->hasBlocks(elem->subdomain_id()),
      "The variable should be defined on the element's subdomain! This typically occurs when the "
      "user wants to evaluate the elements right next to the boundary of two variables (block "
      "boundary). The subdomain which is queried: " +
          Moose::stringify(this->activeSubdomains()) + " the subdomain of the element " +
          std::to_string(elem->subdomain_id()));

  Moose::initDofIndices(const_cast<MooseVariableFV<OutputType> &>(*this), *elem);

  mooseAssert(
      this->_dof_indices.size() == 1,
      "There should only be one dof-index for a constant monomial variable on any given element");

  const dof_id_type index = this->_dof_indices[0];

  // It's not safe to use solutionState(0) because it returns the libMesh System solution member
  // which is wrong during things like finite difference Jacobian evaluation, e.g. when PETSc
  // perturbs the solution vector we feed these perturbations into the current_local_solution
  // while the libMesh solution is frozen in the non-perturbed state
  const auto & global_soln =
      (state.state == 0)
          ? *this->_sys.currentSolution()
          : std::as_const(this->_sys).solutionState(state.state, state.iteration_type);

  ADReal value = global_soln(index);

  if (ADReal::do_derivatives && state.state == 0 &&
      this->_sys.number() == this->_subproblem.currentNlSysNum())
    Moose::derivInsert(value.derivatives(), index, 1.);

  return value;
}

template <typename OutputType>
bool
MooseVariableFV<OutputType>::isDirichletBoundaryFace(const FaceInfo & fi,
                                                     const Elem *,
                                                     const Moose::StateArg &) const
{
  const auto & pr = getDirichletBC(fi);

  // First member of this pair indicates whether we have a DirichletBC
  return pr.first;
}

template <typename OutputType>
ADReal
MooseVariableFV<OutputType>::getDirichletBoundaryFaceValue(const FaceInfo & fi,
                                                           const Elem * const libmesh_dbg_var(elem),
                                                           const Moose::StateArg & state) const
{
  mooseAssert(isDirichletBoundaryFace(fi, elem, state),
              "This function should only be called on Dirichlet boundary faces.");

  const auto & diri_pr = getDirichletBC(fi);

  mooseAssert(diri_pr.first,
              "This functor should only be called if we are on a Dirichlet boundary face.");

  const FVDirichletBCBase & bc = *diri_pr.second;

  return ADReal(bc.boundaryValue(fi, state));
}

template <typename OutputType>
bool
MooseVariableFV<OutputType>::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                                        const Elem * const elem,
                                                        const Moose::StateArg & state) const
{
  if (isDirichletBoundaryFace(fi, elem, state))
    return false;
  else
    return !this->isInternalFace(fi);
}

template <typename OutputType>
ADReal
MooseVariableFV<OutputType>::getExtrapolatedBoundaryFaceValue(const FaceInfo & fi,
                                                              const bool two_term_expansion,
                                                              const bool correct_skewness,
                                                              const Elem * elem_to_extrapolate_from,
                                                              const StateArg & state) const
{
  mooseAssert(
      isExtrapolatedBoundaryFace(fi, elem_to_extrapolate_from, state) || !two_term_expansion,
      "We allow Dirichlet boundary conditions to call this method. However, the only way to "
      "ensure we don't have infinite recursion, with Green Gauss gradients calling back to the "
      "Dirichlet boundary condition calling back to this method, is to do a one term expansion");

  ADReal boundary_value;
  bool elem_to_extrapolate_from_is_fi_elem;
  std::tie(elem_to_extrapolate_from, elem_to_extrapolate_from_is_fi_elem) =
      [this, &fi, elem_to_extrapolate_from]() -> std::pair<const Elem *, bool>
  {
    if (elem_to_extrapolate_from)
      // Somebody already specified the element to extropolate from
      return {elem_to_extrapolate_from, elem_to_extrapolate_from == fi.elemPtr()};
    else
    {
      const auto [elem_guaranteed_to_have_dofs,
                  other_elem,
                  elem_guaranteed_to_have_dofs_is_fi_elem] =
          Moose::FV::determineElemOneAndTwo(fi, *this);
      // We only care about the element guaranteed to have degrees of freedom and current C++
      // doesn't allow us to not assign one of the returned items like python does
      libmesh_ignore(other_elem);
      // We will extrapolate from the element guaranteed to have degrees of freedom
      return {elem_guaranteed_to_have_dofs, elem_guaranteed_to_have_dofs_is_fi_elem};
    }
  }();

  if (two_term_expansion)
  {
    const Point vector_to_face = elem_to_extrapolate_from_is_fi_elem
                                     ? (fi.faceCentroid() - fi.elemCentroid())
                                     : (fi.faceCentroid() - fi.neighborCentroid());
    boundary_value = adGradSln(elem_to_extrapolate_from, state, correct_skewness) * vector_to_face +
                     getElemValue(elem_to_extrapolate_from, state);
  }
  else
    boundary_value = getElemValue(elem_to_extrapolate_from, state);

  return boundary_value;
}

template <typename OutputType>
ADReal
MooseVariableFV<OutputType>::getBoundaryFaceValue(const FaceInfo & fi,
                                                  const StateArg & state,
                                                  const bool correct_skewness) const
{
  mooseAssert(!this->isInternalFace(fi),
              "A boundary face value has been requested on an internal face.");

  if (isDirichletBoundaryFace(fi, nullptr, state))
    return getDirichletBoundaryFaceValue(fi, nullptr, state);
  else if (isExtrapolatedBoundaryFace(fi, nullptr, state))
    return getExtrapolatedBoundaryFaceValue(
        fi, _two_term_boundary_expansion, correct_skewness, nullptr, state);

  mooseError("Unknown boundary face type!");
}

template <typename OutputType>
const VectorValue<ADReal> &
MooseVariableFV<OutputType>::adGradSln(const Elem * const elem,
                                       const StateArg & state,
                                       const bool correct_skewness) const
{
  // We ensure that no caching takes place when we compute skewness-corrected
  // quantities.
  if (_cache_cell_gradients && !correct_skewness && state.state == 0)
  {
    auto it = _elem_to_grad.find(elem);

    if (it != _elem_to_grad.end())
      return it->second;
  }

  auto grad = FV::greenGaussGradient(
      ElemArg({elem, correct_skewness}), state, *this, _two_term_boundary_expansion, this->_mesh);

  if (_cache_cell_gradients && !correct_skewness && state.state == 0)
  {
    auto pr = _elem_to_grad.emplace(elem, std::move(grad));
    mooseAssert(pr.second, "Insertion should have just happened.");
    return pr.first->second;
  }
  else
  {
    _temp_cell_gradient = std::move(grad);
    return _temp_cell_gradient;
  }
}

template <typename OutputType>
VectorValue<ADReal>
MooseVariableFV<OutputType>::uncorrectedAdGradSln(const FaceInfo & fi,
                                                  const StateArg & state,
                                                  const bool correct_skewness) const
{
  const bool var_defined_on_elem = this->hasBlocks(fi.elem().subdomain_id());
  const Elem * const elem_one = var_defined_on_elem ? &fi.elem() : fi.neighborPtr();
  const Elem * const elem_two = var_defined_on_elem ? fi.neighborPtr() : &fi.elem();

  const VectorValue<ADReal> elem_one_grad = adGradSln(elem_one, state, correct_skewness);

  // If we have a neighbor then we interpolate between the two to the face. If we do not, then we
  // apply a zero Hessian assumption and use the element centroid gradient as the uncorrected face
  // gradient
  if (elem_two && this->hasBlocks(elem_two->subdomain_id()))
  {
    const VectorValue<ADReal> & elem_two_grad = adGradSln(elem_two, state, correct_skewness);

    // Uncorrected gradient value
    return Moose::FV::linearInterpolation(elem_one_grad, elem_two_grad, fi, var_defined_on_elem);
  }
  else
    return elem_one_grad;
}

template <typename OutputType>
VectorValue<ADReal>
MooseVariableFV<OutputType>::adGradSln(const FaceInfo & fi,
                                       const StateArg & state,
                                       const bool correct_skewness) const
{
  const bool var_defined_on_elem = this->hasBlocks(fi.elem().subdomain_id());
  const Elem * const elem = &fi.elem();
  const Elem * const neighbor = fi.neighborPtr();

  const bool is_internal_face = this->isInternalFace(fi);

  const ADReal side_one_value = (!is_internal_face && !var_defined_on_elem)
                                    ? getBoundaryFaceValue(fi, state, correct_skewness)
                                    : getElemValue(elem, state);
  const ADReal side_two_value = (var_defined_on_elem && !is_internal_face)
                                    ? getBoundaryFaceValue(fi, state, correct_skewness)
                                    : getElemValue(neighbor, state);

  const auto delta =
      this->isInternalFace(fi)
          ? fi.dCNMag()
          : (fi.faceCentroid() - (var_defined_on_elem ? fi.elemCentroid() : fi.neighborCentroid()))
                .norm();

  // This is the component of the gradient which is parallel to the line connecting
  // the cell centers. Therefore, we can use our second order, central difference
  // scheme to approximate it.
  auto face_grad = ((side_two_value - side_one_value) / delta) * fi.eCN();

  // We only need non-orthogonal correctors in 2+ dimensions
  if (this->_mesh.dimension() > 1)
  {
    // We are using an orthogonal approach for the non-orthogonal correction, for more information
    // see Hrvoje Jasak's PhD Thesis (Imperial College, 1996)
    const auto & interpolated_gradient = uncorrectedAdGradSln(fi, state, correct_skewness);
    face_grad += interpolated_gradient - (interpolated_gradient * fi.eCN()) * fi.eCN();
  }

  return face_grad;
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::residualSetup()
{
  clearCaches();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::jacobianSetup()
{
  clearCaches();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::clearCaches()
{
  _elem_to_grad.clear();
}

template <typename OutputType>
unsigned int
MooseVariableFV<OutputType>::oldestSolutionStateRequested() const
{
  unsigned int state = 0;
  state = std::max(state, _element_data->oldestSolutionStateRequested());
  state = std::max(state, _neighbor_data->oldestSolutionStateRequested());
  return state;
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::clearAllDofIndices()
{
  _element_data->clearDofIndices();
  _neighbor_data->clearDofIndices();
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::ValueType
MooseVariableFV<OutputType>::evaluate(const FaceArg & face, const StateArg & state) const
{
  const FaceInfo * const fi = face.fi;
  mooseAssert(fi, "The face information must be non-null");
  if (isDirichletBoundaryFace(*fi, face.face_side, state))
    return getDirichletBoundaryFaceValue(*fi, face.face_side, state);
  else if (isExtrapolatedBoundaryFace(*fi, face.face_side, state))
  {
    bool two_term_boundary_expansion = _two_term_boundary_expansion;
    if (face.limiter_type == Moose::FV::LimiterType::Upwind)
      if ((face.elem_is_upwind && face.face_side == fi->elemPtr()) ||
          (!face.elem_is_upwind && face.face_side == fi->neighborPtr()))
        two_term_boundary_expansion = false;
    return getExtrapolatedBoundaryFaceValue(
        *fi, two_term_boundary_expansion, face.correct_skewness, face.face_side, state);
  }
  else
  {
    mooseAssert(this->isInternalFace(*fi),
                "We must be either Dirichlet, extrapolated, or internal");
    return Moose::FV::interpolate(*this, face, state);
  }
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::ValueType
MooseVariableFV<OutputType>::evaluate(const NodeArg & node_arg, const StateArg & state) const
{
  const auto & node_to_elem_map = this->_mesh.nodeToElemMap();
  const auto & elem_ids = libmesh_map_find(node_to_elem_map, node_arg.node->id());
  ValueType sum = 0;
  Real total_weight = 0;
  mooseAssert(elem_ids.size(), "There should always be at least one element connected to a node");
  for (const auto elem_id : elem_ids)
  {
    const Elem * const elem = this->_mesh.queryElemPtr(elem_id);
    mooseAssert(elem, "We should have this element available");
    if (!this->hasBlocks(elem->subdomain_id()))
      continue;
    const ElemPointArg elem_point{
        elem, *node_arg.node, _face_interp_method == Moose::FV::InterpMethod::SkewCorrectedAverage};
    const auto weight = 1 / (*node_arg.node - elem->vertex_average()).norm();
    sum += weight * (*this)(elem_point, state);
    total_weight += weight;
  }
  return sum / total_weight;
}

template <typename OutputType>
typename MooseVariableFV<OutputType>::DotType
MooseVariableFV<OutputType>::evaluateDot(const ElemArg &, const StateArg &) const
{
  mooseError("evaluateDot not implemented for this class of finite volume variables");
}

template <>
ADReal
MooseVariableFV<Real>::evaluateDot(const ElemArg & elem_arg, const StateArg & state) const
{
  const Elem * const elem = elem_arg.elem;
  mooseAssert(state.state == 0,
              "We dot not currently support any time derivative evaluations other than for the "
              "current time-step");
  mooseAssert(_time_integrator && _time_integrator->dt(),
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");

  Moose::initDofIndices(const_cast<MooseVariableFV<Real> &>(*this), *elem);

  mooseAssert(
      this->_dof_indices.size() == 1,
      "There should only be one dof-index for a constant monomial variable on any given element");

  const dof_id_type dof_index = this->_dof_indices[0];

  if (_var_kind == Moose::VAR_SOLVER)
  {
    ADReal dot = (*_solution)(dof_index);
    if (ADReal::do_derivatives && state.state == 0 &&
        _sys.number() == _subproblem.currentNlSysNum())
      Moose::derivInsert(dot.derivatives(), dof_index, 1.);
    _time_integrator->computeADTimeDerivatives(dot, dof_index, _ad_real_dummy);
    return dot;
  }
  else
    return (*_sys.solutionUDot())(dof_index);
}

template <>
ADReal
MooseVariableFV<Real>::evaluateDot(const FaceArg & face, const StateArg & state) const
{
  const FaceInfo * const fi = face.fi;
  mooseAssert(fi, "The face information must be non-null");
  if (isDirichletBoundaryFace(*fi, face.face_side, state))
    return ADReal(0.0); // No time derivative if boundary value is set
  else if (isExtrapolatedBoundaryFace(*fi, face.face_side, state))
  {
    mooseAssert(face.face_side && this->hasBlocks(face.face_side->subdomain_id()),
                "If we are an extrapolated boundary face, then our FunctorBase::checkFace method "
                "should have assigned a non-null element that we are defined on");
    const auto elem_arg = ElemArg({face.face_side, face.correct_skewness});
    // For extrapolated boundary faces, note that we take the value of the time derivative at the
    // cell in contact with the face
    return evaluateDot(elem_arg, state);
  }
  else
  {
    mooseAssert(this->isInternalFace(*fi),
                "We must be either Dirichlet, extrapolated, or internal");
    return Moose::FV::interpolate<ADReal, FunctorEvaluationKind::Dot>(*this, face, state);
  }
}

template <>
ADReal
MooseVariableFV<Real>::evaluateDot(const ElemQpArg & elem_qp, const StateArg & state) const
{
  return evaluateDot(ElemArg({elem_qp.elem, /*correct_skewness*/ false}), state);
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::prepareAux()
{
  _element_data->prepareAux();
  _neighbor_data->prepareAux();
}

template <typename OutputType>
void
MooseVariableFV<OutputType>::determineBoundaryToDirichletBCMap()
{
  _boundary_id_to_dirichlet_bc.clear();
  std::vector<FVDirichletBCBase *> bcs;

  // I believe because query() returns by value but condition returns by reference that binding to a
  // const lvalue reference results in the query() getting destructed and us holding onto a dangling
  // reference. I think that condition returned by value we would be able to bind to a const lvalue
  // reference here. But as it is we'll bind to a regular lvalue
  const auto base_query = this->_subproblem.getMooseApp()
                              .theWarehouse()
                              .query()
                              .template condition<AttribSystem>("FVDirichletBC")
                              .template condition<AttribThread>(_tid)
                              .template condition<AttribVar>(_var_num)
                              .template condition<AttribSysNum>(this->_sys.number());

  for (const auto bnd_id : this->_mesh.getBoundaryIDs())
  {
    auto base_query_copy = base_query;
    base_query_copy.template condition<AttribBoundaries>(std::set<BoundaryID>({bnd_id}))
        .queryInto(bcs);
    mooseAssert(bcs.size() <= 1, "cannot have multiple dirichlet BCs on the same boundary");
    if (!bcs.empty())
      _boundary_id_to_dirichlet_bc.emplace(bnd_id, bcs[0]);
  }

  _dirichlet_map_setup = true;
}

template class MooseVariableFV<Real>;
// TODO: implement vector fv variable support. This will require some template
// specializations for various member functions in this and the FV variable
// classes. And then you will need to uncomment out the line below:
// template class MooseVariableFV<RealVectorValue>;
