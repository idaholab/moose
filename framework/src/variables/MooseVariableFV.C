//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFV.h"
#include <typeinfo>
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"
#include "DisplacedSystem.h"
#include "SystemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "FVUtils.h"
#include "FVFluxBC.h"
#include "FVDirichletBCBase.h"

#include "libmesh/numeric_vector.h"

#include <climits>

registerMooseObject("MooseApp", MooseVariableFVReal);

template <typename OutputType>
InputParameters
MooseVariableFV<OutputType>::validParams()
{
  InputParameters params = MooseVariableField<OutputType>::validParams();
  params.set<bool>("fv") = true;
  params.set<MooseEnum>("family") = "MONOMIAL";
  params.set<MooseEnum>("order") = "CONSTANT";
#ifdef MOOSE_GLOBAL_AD_INDEXING
  params.template addParam<bool>("use_extended_stencil",
                                 false,
                                 "Whether to use an extended stencil for gradient computation.");
  params.template addParam<bool>(
      "two_term_boundary_expansion",
      false,
      "Whether to use a two-term Taylor expansion to calculate boundary face values. The default "
      "is to use one-term, e.g. the element centroid value will be used for the boundary face "
      "value. If the two-term expansion is used, then the boundary face value depends on the "
      "adjoining cell center gradient, which itself depends on the boundary face value. "
      "Consequently an implicit solve is used to simultaneously solve for the adjoining cell "
      "center gradient and boundary face value(s).");
#endif
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
    _two_term_boundary_expansion(this->isParamValid("two_term_boundary_expansion")
                                     ? this->template getParam<bool>("two_term_boundary_expansion")
                                     : false),
    // If the user doesn't specify a MooseVariableFV type in the input file, then we won't have
    // these parameters available
    _use_extended_stencil(this->isParamValid("use_extended_stencil")
                              ? this->template getParam<bool>("use_extended_stencil")
                              : false)
{
  _element_data = libmesh_make_unique<MooseVariableDataFV<OutputType>>(
      *this, _sys, _tid, Moose::ElementType::Element, this->_assembly.elem());
  _neighbor_data = libmesh_make_unique<MooseVariableDataFV<OutputType>>(
      *this, _sys, _tid, Moose::ElementType::Neighbor, this->_assembly.neighbor());
}

template <typename OutputType>
const std::set<SubdomainID> &
MooseVariableFV<OutputType>::activeSubdomains() const
{
  return this->_sys.system().variable(_var_num).active_subdomains();
}

template <typename OutputType>
Moose::VarFieldType
MooseVariableFV<OutputType>::fieldType() const
{
  if (std::is_same<OutputType, Real>::value)
    return Moose::VarFieldType::VAR_FIELD_STANDARD;
  else if (std::is_same<OutputType, RealVectorValue>::value)
    return Moose::VarFieldType::VAR_FIELD_VECTOR;
  else if (std::is_same<OutputType, RealEigenVector>::value)
    return Moose::VarFieldType::VAR_FIELD_ARRAY;
  else
    mooseError("Unknown variable field type");
}

template <typename OutputType>
bool
MooseVariableFV<OutputType>::activeOnSubdomain(SubdomainID subdomain) const
{
  return this->_sys.system().variable(_var_num).active_on_subdomain(subdomain);
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
MooseVariableFV<OutputType>::add(NumericVector<Number> & /*residual*/)
{
  mooseError("add not supported for FV variables");
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
MooseVariableFV<OutputType>::computeFaceValues(const FaceInfo & fi)
{
  _element_data->setGeometry(Moose::Face);
  _neighbor_data->setGeometry(Moose::Face);

  auto facetype = fi.faceType(_var_name);
  if (facetype == FaceInfo::VarFaceNeighbors::NEITHER)
    return;
  else if (facetype == FaceInfo::VarFaceNeighbors::BOTH)
  {
    _element_data->computeValuesFace(fi);
    _neighbor_data->computeValuesFace(fi);
  }
  else if (facetype == FaceInfo::VarFaceNeighbors::ELEM)
  {
    _element_data->computeValuesFace(fi);
    _neighbor_data->computeGhostValuesFace(fi, *_element_data);
  }
  else if (facetype == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    _neighbor_data->computeValuesFace(fi);
    _element_data->computeGhostValuesFace(fi, *_neighbor_data);
  }
  else
    mooseError("robert wrote broken MooseVariableFV code");
}

template <typename OutputType>
OutputType
MooseVariableFV<OutputType>::getValue(const Elem * elem) const
{
  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);
  mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
  OutputType value = (*this->_sys.currentSolution())(dof_indices[0]);
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
bool
MooseVariableFV<OutputType>::isVector() const
{
  return std::is_same<OutputType, RealVectorValue>::value;
}

template <typename OutputType>
std::pair<bool, const FVDirichletBCBase *>
MooseVariableFV<OutputType>::getDirichletBC(const FaceInfo & fi) const
{
  std::vector<FVDirichletBCBase *> bcs;

  this->_subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribSystem>("FVDirichletBC")
      .template condition<AttribThread>(_tid)
      .template condition<AttribBoundaries>(fi.boundaryIDs())
      .template condition<AttribVar>(_var_num)
      .template condition<AttribSysNum>(this->_sys.number())
      .queryInto(bcs);
  mooseAssert(bcs.size() <= 1, "cannot have multiple dirichlet BCs on the same boundary");

  bool has_dirichlet_bc = bcs.size() > 0;

  if (has_dirichlet_bc)
  {
    mooseAssert(bcs[0], "The FVDirichletBC is null!");

    return std::make_pair(true, bcs[0]);
  }
  else
    return std::make_pair(false, nullptr);
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
const ADReal &
MooseVariableFV<OutputType>::getVertexValue(const Node & vertex) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::getVertexValue only supported for global AD indexing");
#endif

  auto it = _vertex_to_value.find(&vertex);

  if (it != _vertex_to_value.end())
    return it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = _vertex_to_value.emplace(&vertex, 0);
  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");
  ADReal & value = emplace_ret.first->second;
  ADReal numerator = 0, denominator = 0;

  const auto node_elem_it = this->_mesh.nodeToElemMap().find(vertex.id());
  mooseAssert(node_elem_it != this->_mesh.nodeToElemMap().end(), "Should have found the node");
  const auto & connected_elems = node_elem_it->second;
  const MeshBase & lm_mesh = this->_mesh.getMesh();

  for (const auto elem_id : connected_elems)
  {
    const Elem * const elem = lm_mesh.elem_ptr(elem_id);
    mooseAssert(elem, "If the elem ID exists, then the elem shouldn't be null");

    if (this->hasBlocks(elem->subdomain_id()))
    {
      const auto & elem_value = getElemValue(elem);
      auto distance = (vertex - elem->centroid()).norm();
      numerator += elem_value / distance;
      denominator += 1. / distance;
    }
  }

  value = numerator / denominator;

  return value;
}

template <typename OutputType>
ADReal
MooseVariableFV<OutputType>::getElemValue(const Elem * const elem) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::getElemValue only supported for global AD indexing");
#endif

  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);

  mooseAssert(
      dof_indices.size() == 1,
      "There should only be one dof-index for a constant monomial variable on any given element");

  dof_id_type index = dof_indices[0];

  ADReal value = (*_solution)(index);

  if (ADReal::do_derivatives && _var_kind == Moose::VAR_NONLINEAR)
    Moose::derivInsert(value.derivatives(), index, 1.);

  return value;
}

template <typename OutputType>
ADReal
MooseVariableFV<OutputType>::getNeighborValue(const Elem * const neighbor,
                                              const FaceInfo & fi,
                                              const ADReal & elem_value) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::getNeighborValue only supported for global AD indexing");
#endif

  if (neighbor && this->hasBlocks(neighbor->subdomain_id()))
    return getElemValue(neighbor);
  else
    // If we don't have a neighbor, then we're along a boundary
    // Linear interpolation: face_value = (elem_value + neighbor_value) / 2. Note that weights of
    // 1/2 are perfectly appropriate here because we can arbitrarily put our ghost cell centroid
    // anywhere and by convention we locate it such that a line drawn between the ghost cell
    // centroid and the element centroid is perfectly bisected by the face centroid
    return 2. * getBoundaryFaceValue(fi) - elem_value;
}

template <typename OutputType>
bool
MooseVariableFV<OutputType>::isInternalFace(const FaceInfo & fi) const
{
  const bool is_internal_face = fi.faceType(this->name()) == FaceInfo::VarFaceNeighbors::BOTH;
  mooseAssert(is_internal_face == (this->hasBlocks(fi.elem().subdomain_id()) && fi.neighborPtr() &&
                                   this->hasBlocks(fi.neighborPtr()->subdomain_id())),
              "Sanity checking whether we are indeed an internal face");
  return is_internal_face;
}

template <typename OutputType>
const ADReal &
MooseVariableFV<OutputType>::getInternalFaceValue(const Elem * const neighbor,
                                                  const FaceInfo & fi,
                                                  const ADReal & elem_value) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::getInternalFaceValue only supported for global AD indexing");
#endif

  mooseAssert(isInternalFace(fi), "This function only be called on internal faces.");

  auto pr = _face_to_value.emplace(&fi, 0);

  if (!pr.second)
    // Insertion didn't happen...we already have a value ready to go
    return pr.first->second;

  ADReal & value = pr.first->second;

  if (_use_extended_stencil)
  {
    ADReal numerator = 0, denominator = 0;

    for (const Node * const vertex : fi.vertices())
    {
      auto distance = (*vertex - fi.faceCentroid()).norm();

      numerator += getVertexValue(*vertex) / distance;
      denominator += 1. / distance;
    }

    value = numerator / denominator;
  }
  else
  {
    // Compact stencil
    ADReal neighbor_value = getElemValue(neighbor);

    value = Moose::FV::linearInterpolation(
        elem_value, neighbor_value, fi, neighbor == fi.neighborPtr());
  }

  return value;
}

template <typename OutputType>
bool
MooseVariableFV<OutputType>::isDirichletBoundaryFace(const FaceInfo & fi) const
{
  const auto & pr = getDirichletBC(fi);

  // First member of this pair indicates whether we have a DirichletBC
  return pr.first;
}

template <typename OutputType>
const ADReal &
MooseVariableFV<OutputType>::getDirichletBoundaryFaceValue(const FaceInfo & fi) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(
      "MooseVariableFV::getDirichletBoundaryFaceValue only supported for global AD indexing");
#endif

  mooseAssert(isDirichletBoundaryFace(fi),
              "This function should only be called on Dirichlet boundary faces.");

  auto pr = _face_to_value.emplace(&fi, 0);

  if (!pr.second)
    // Insertion didn't happen...we already have a value ready to go
    return pr.first->second;

  ADReal & value = pr.first->second;

  const auto & diri_pr = getDirichletBC(fi);

  mooseAssert(diri_pr.first,
              "This functor should only be called if we are on a Dirichlet boundary face.");

  const FVDirichletBCBase & bc = *diri_pr.second;

  value = ADReal(bc.boundaryValue(fi));

  return value;
}

template <typename OutputType>
bool
MooseVariableFV<OutputType>::isExtrapolatedBoundaryFace(const FaceInfo & fi) const
{
  return !isDirichletBoundaryFace(fi) && !isInternalFace(fi);
}

template <typename OutputType>
const ADReal &
MooseVariableFV<OutputType>::getExtrapolatedBoundaryFaceValue(const FaceInfo & fi) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(
      "MooseVariableFV::getExtrapolatedBoundaryFaceValue only supported for global AD indexing");
#endif

  mooseAssert(isExtrapolatedBoundaryFace(fi),
              "This function should only be called on extrapolated boundary faces");

  const auto & tup = Moose::FV::determineElemOneAndTwo(fi, *this);
  const Elem * const elem = std::get<0>(tup);

  if (_two_term_boundary_expansion)
  {
    // We need to compute the gradient. That gradient computation will cache the boundary face value
    // for us
    adGradSln(elem);

    auto it = _face_to_value.find(&fi);
    mooseAssert(it != _face_to_value.end(),
                "adGradSln(elem) should have generated the boundary face value for us");

    return it->second;
  }
  else
  {
    // We are doing a one-term Taylor expansion and the face value is simply the centroid value
    const auto & pr = _face_to_value.emplace(&fi, getElemValue(elem));
    mooseAssert(pr.second, "This should have inserted a new key-value pair");
    return pr.first->second;
  }
}

template <typename OutputType>
const ADReal &
MooseVariableFV<OutputType>::getBoundaryFaceValue(const FaceInfo & fi) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::getBoundaryFaceValue only supported for global AD indexing");
#endif

  mooseAssert(!isInternalFace(fi), "A boundary face value has been requested on an internal face.");

  // Check to see whether it's already in our cache
  auto it = _face_to_value.find(&fi);
  if (it != _face_to_value.end())
    return it->second;

  if (isDirichletBoundaryFace(fi))
    return getDirichletBoundaryFaceValue(fi);
  else if (isExtrapolatedBoundaryFace(fi))
    return getExtrapolatedBoundaryFaceValue(fi);

  mooseError("Unknown boundary face type!");
}

template <typename OutputType>
const VectorValue<ADReal> &
MooseVariableFV<OutputType>::adGradSln(const Elem * const elem) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::adGradSln only supported for global AD indexing");
#endif

  auto pr = _elem_to_grad.emplace(elem, 0);

  if (!pr.second)
    // Insertion didn't happen...we already have a gradient ready to go
    return pr.first->second;

  VectorValue<ADReal> & grad = pr.first->second;

  bool volume_set = false;
  Real volume = 0;

  ADReal elem_value = getElemValue(elem);

  // If we are performing a two term Taylor expansion for extrapolated boundary faces (faces on
  // boundaries that do not have associated Dirichlet conditions), then the element gradient depends
  // on the boundary face value and the boundary face value depends on the element gradient, so we
  // have a system of equations to solve. Here is the system:
  //
  // \nabla \phi_C - \frac{1}{V} \sum_{ebf} \phi_{ebf} \vec{S_f} =
  //   \frac{1}{V} \sum_{of} \phi_{of} \vec{S_f}                       eqn. 1
  //
  // \phi_{ebf} - \vec{d_{Cf}} \cdot \nabla \phi_C = \phi_C            eqn. 2
  //
  // where $C$ refers to the cell centroid, $ebf$ refers to an extrapolated boundary face, $of$
  // refers to "other faces", e.g. non-ebf faces, and $f$ is a general face. $d_{Cf}$ is the vector
  // drawn from the element centroid to the face centroid, and $\vec{S_f}$ is the surface vector,
  // e.g. the face area times the outward facing normal

  // We'll save off the extrapolated boundary faces (ebf) for later assignment to the cache (these
  // are the keys)
  std::vector<const FaceInfo *> ebf_faces;
  // ebf eqns: element gradient coefficients, e.g. eqn. 2, LHS term 2 coefficient
  std::vector<VectorValue<Real>> ebf_grad_coeffs;
  // ebf eqns: rhs b values. These will actually correspond to the elem_value so we can use a
  // pointer and avoid copying. This is the RHS of eqn. 2
  std::vector<const ADReal *> ebf_b;

  // elem grad eqns: ebf coefficients, e.g. eqn. 1, LHS term 2 coefficients
  std::vector<VectorValue<Real>> grad_ebf_coeffs;
  // elem grad eqns: rhs b value, e.g. eqn. 1 RHS
  VectorValue<ADReal> grad_b = 0;

  auto action_functor = [&volume_set,
                         &volume,
                         &elem_value,
#ifndef NDEBUG
                         &elem,
#endif
                         &ebf_faces,
                         &ebf_grad_coeffs,
                         &ebf_b,
                         &grad_ebf_coeffs,
                         &grad_b,
                         this](const Elem & functor_elem,
                               const Elem * const neighbor,
                               const FaceInfo * const fi,
                               const Point & surface_vector,
                               Real coord,
                               const bool elem_has_info) {
    mooseAssert(fi, "We need a FaceInfo for this action_functor");
    mooseAssert(elem == &functor_elem,
                "Just a sanity check that the element being passed in is the one we passed out.");

    if (isExtrapolatedBoundaryFace(*fi))
    {
      if (_two_term_boundary_expansion)
      {
        ebf_faces.push_back(fi);

        // eqn. 2
        ebf_grad_coeffs.push_back(-1. * (elem_has_info
                                             ? (fi->faceCentroid() - fi->elemCentroid())
                                             : (fi->faceCentroid() - fi->neighborCentroid())));
        ebf_b.push_back(&elem_value);

        // eqn. 1
        grad_ebf_coeffs.push_back(-surface_vector);
      }
      else
        // We are doing a one-term expansion for the extrapolated boundary faces, in which case we
        // have no eqn. 2 and we have no second term in the LHS of eqn. 1. Instead we apply the
        // element centroid value as the face value (one-term expansion) in the RHS of eqn. 1
        grad_b += surface_vector * elem_value;
    }
    else if (isInternalFace(*fi))
      grad_b += surface_vector * getInternalFaceValue(neighbor, *fi, elem_value);
    else
    {
      mooseAssert(isDirichletBoundaryFace(*fi), "We've run out of face types");
      grad_b += surface_vector * getDirichletBoundaryFaceValue(*fi);
    }

    if (!volume_set)
    {
      // We use the FaceInfo volumes because those values have been pre-computed and cached.
      // An explicit call to elem->volume() here would incur unnecessary expense
      if (elem_has_info)
      {
        coordTransformFactor(
            this->_subproblem, functor_elem.subdomain_id(), fi->elemCentroid(), coord);
        volume = fi->elemVolume() * coord;
      }
      else
      {
        coordTransformFactor(
            this->_subproblem, neighbor->subdomain_id(), fi->neighborCentroid(), coord);
        volume = fi->neighborVolume() * coord;
      }

      volume_set = true;
    }
  };

  Moose::FV::loopOverElemFaceInfo(*elem, this->_mesh, this->_subproblem, action_functor);

  mooseAssert(volume_set && volume > 0, "We should have set the volume");
  grad_b /= volume;

  const auto coord_system = this->_subproblem.getCoordSystem(elem->subdomain_id());
  if (coord_system == Moose::CoordinateSystemType::COORD_RZ)
  {
    const auto r_coord = this->_subproblem.getAxisymmetricRadialCoord();
    grad_b(r_coord) -= elem_value / elem->centroid()(r_coord);
  }

  mooseAssert(coord_system != Moose::CoordinateSystemType::COORD_RSPHERICAL,
              "We have not yet implemented the correct translation from gradient to divergence for "
              "spherical coordinates yet.");

  mooseAssert(ebf_faces.size() < UINT_MAX,
              "You've created a mystical element that has more faces than can be held by unsigned "
              "int. I applaud you.");
  const auto num_ebfs = static_cast<unsigned int>(ebf_faces.size());

  // test for simple case
  if (num_ebfs == 0)
    grad = grad_b;
  else
  {
    // We have to solve a system
    const unsigned int sys_dim = LIBMESH_DIM + num_ebfs;
    DenseVector<ADReal> x(sys_dim), b(sys_dim);
    DenseMatrix<ADReal> A(sys_dim, sys_dim);

    // Let's make i refer to LIBMESH_DIM indices, and j refer to num_ebfs indices

    // eqn. 1
    for (const auto i : make_range(unsigned(LIBMESH_DIM)))
    {
      // LHS term 1 coeffs
      A(i, i) = 1;

      // LHS term 2 coeffs
      for (const auto j : make_range(num_ebfs))
        A(i, LIBMESH_DIM + j) = grad_ebf_coeffs[j](i) / volume;

      // RHS
      b(i) = grad_b(i);
    }

    // eqn. 2
    for (const auto j : make_range(num_ebfs))
    {
      // LHS term 1 coeffs
      A(LIBMESH_DIM + j, LIBMESH_DIM + j) = 1;

      // LHS term 2 coeffs
      for (const auto i : make_range(unsigned(LIBMESH_DIM)))
        A(LIBMESH_DIM + j, i) = ebf_grad_coeffs[j](i);

      // RHS
      b(LIBMESH_DIM + j) = *ebf_b[j];
    }

    A.lu_solve(b, x);
    for (const auto i : make_range(unsigned(LIBMESH_DIM)))
      grad(i) = x(i);

    // Cache the face value information
    for (const auto j : make_range(num_ebfs))
      _face_to_value.emplace(ebf_faces[j], x(LIBMESH_DIM + j));
  }

  return grad;
}

template <typename OutputType>
const VectorValue<ADReal> &
MooseVariableFV<OutputType>::uncorrectedAdGradSln(const FaceInfo & fi) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::uncorrectedAdGradSln only supported for global AD indexing");
#endif

  auto it = _face_to_unc_grad.find(&fi);

  if (it != _face_to_unc_grad.end())
    return it->second;

  auto tup = Moose::FV::determineElemOneAndTwo(fi, *this);
  const Elem * const elem_one = std::get<0>(tup);
  const Elem * const elem_two = std::get<1>(tup);
  const bool elem_one_is_fi_elem = std::get<2>(tup);

  const VectorValue<ADReal> & elem_one_grad = adGradSln(elem_one);

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second
  // a boolean denoting whether a new insertion took place
  auto emplace_ret = _face_to_unc_grad.emplace(&fi, elem_one_grad);

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  VectorValue<ADReal> & unc_face_grad = emplace_ret.first->second;

  // If we have a neighbor then we interpolate between the two to the face. If we do not, then we
  // apply a zero Hessian assumption and use the element centroid gradient as the uncorrected face
  // gradient
  if (elem_two && this->hasBlocks(elem_two->subdomain_id()))
  {
    const VectorValue<ADReal> & elem_two_grad = adGradSln(elem_two);

    // Uncorrected gradient value
    unc_face_grad =
        Moose::FV::linearInterpolation(elem_one_grad, elem_two_grad, fi, elem_one_is_fi_elem);
  }

  return unc_face_grad;
}

template <typename OutputType>
const VectorValue<ADReal> &
MooseVariableFV<OutputType>::adGradSln(const FaceInfo & fi) const
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("MooseVariableFV::adGradSln only supported for global AD indexing");
#endif

  auto it = _face_to_grad.find(&fi);

  if (it != _face_to_grad.end())
    return it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second
  // a boolean denoting whether a new insertion took place
  auto emplace_ret = _face_to_grad.emplace(&fi, uncorrectedAdGradSln(fi));

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  VectorValue<ADReal> & face_grad = emplace_ret.first->second;

  auto tup = Moose::FV::determineElemOneAndTwo(fi, *this);
  const Elem * const elem_one = std::get<0>(tup);
  const Elem * const elem_two = std::get<1>(tup);
  const bool elem_is_elem_one = std::get<2>(tup);
  mooseAssert(elem_is_elem_one ? elem_one == &fi.elem() && elem_two == fi.neighborPtr()
                               : elem_one == fi.neighborPtr() && elem_two == &fi.elem(),
              "The determineElemOneAndTwo utility got the elem_is_elem_one value wrong.");

  const ADReal elem_one_value = getElemValue(elem_one);
  const ADReal elem_two_value = getNeighborValue(elem_two, fi, elem_one_value);
  const ADReal & elem_value = elem_is_elem_one ? elem_one_value : elem_two_value;
  const ADReal & neighbor_value = elem_is_elem_one ? elem_two_value : elem_one_value;

  // perform the correction. Note that direction is important here because we have a minus sign.
  // Neighbor has to be neighbor, and elem has to be elem. Hence all the elem_is_elem_one logic
  // above
  face_grad += ((neighbor_value - elem_value) / fi.dCFMag() - face_grad * fi.eCF()) * fi.eCF();

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
  _face_to_unc_grad.clear();
  _face_to_grad.clear();
  _vertex_to_value.clear();
  _face_to_value.clear();
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

template class MooseVariableFV<Real>;
// TODO: implement vector fv variable support. This will require some template
// specializations for various member functions in this and the FV variable
// classes. And then you will need to uncomment out the line below:
// template class MooseVariableFV<RealVectorValue>;
