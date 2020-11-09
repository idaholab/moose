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
#include "FVDirichletBC.h"

#include "libmesh/numeric_vector.h"

registerMooseObject("MooseApp", MooseVariableFVReal);
registerMooseObject("MooseApp", MooseVariableFVArray);

template <typename OutputType>
InputParameters
MooseVariableFV<OutputType>::validParams()
{
  auto params = MooseVariableField<OutputType>::validParams();
#ifdef MOOSE_GLOBAL_AD_INDEXING
  params.template addParam<bool>("use_extended_stencil",
                                 false,
                                 "Whether to use an extended stencil for gradient computation.");
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
        this->_assembly.template feGradPhiNeighbor<OutputShape>(FEType(CONSTANT, MONOMIAL)))
#ifdef MOOSE_GLOBAL_AD_INDEXING
    ,
    // If the user doesn't specify a MooseVariableFV type in the input file, then we won't have this
    // parameter available
    _use_extended_stencil(this->isParamValid("use_extended_stencil")
                              ? this->template getParam<bool>("use_extended_stencil")
                              : false)
#endif
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
  DoFValue value;
  _element_data->buildDofValues(dof_indices, *(_sys.currentSolution()), value);
  return value[0];
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
bool
MooseVariableFV<OutputType>::hasDirichletBC(const FaceInfo & fi) const
{
  std::vector<FVBoundaryCondition *> bcs;

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
  return has_dirichlet_bc;
}

template <typename OutputType>
std::pair<bool, const typename Moose::FVDirichletBCType<OutputType>::type *>
MooseVariableFV<OutputType>::getDirichletBC(const FaceInfo & fi) const
{
  std::vector<typename Moose::FVDirichletBCType<OutputType>::type *> bcs;

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

#ifdef MOOSE_GLOBAL_AD_INDEXING

template <typename OutputType>
const typename Moose::ADType<typename MooseVariableFV<OutputType>::OutputData>::type &
MooseVariableFV<OutputType>::getVertexValue(const Node & vertex) const
{
  auto it = _vertex_to_value.find(&vertex);

  if (it != _vertex_to_value.end())
    return it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = _vertex_to_value.emplace(&vertex, typename Moose::ADType<OutputData>::type{});
  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");
  auto & value = emplace_ret.first->second;
  typename Moose::ADType<OutputData>::type numerator = {};
  ADReal denominator = 0;

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
      numerator += elem_value * (1.0 / distance);
      denominator += 1. / distance;
    }
  }

  value = numerator * (1.0 / denominator);

  return value;
}

template <typename OutputType>
typename Moose::ADType<typename MooseVariableFV<OutputType>::OutputData>::type
MooseVariableFV<OutputType>::getElemValue(const Elem * const elem) const
{
  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);

  mooseAssert(
      dof_indices.size() == 1,
      "There should only be one dof-index for a constant monomial variable on any given element");

  ADDoFValue vals;
  _element_data->buildADDofValues(dof_indices, *_solution, vals);
  return vals[0];
}

template <typename OutputType>
typename Moose::ADType<typename MooseVariableFV<OutputType>::OutputData>::type
MooseVariableFV<OutputType>::getNeighborValue(
    const Elem * const neighbor,
    const FaceInfo & fi,
    const typename Moose::ADType<OutputData>::type & elem_value) const
{
  if (neighbor && this->hasBlocks(neighbor->subdomain_id()))
    return getElemValue(neighbor);
  else
  {
    // If we don't have a neighbor, then we're along a boundary, and we may have a DirichletBC
    const auto & pr = getDirichletBC(fi);

    if (pr.first)
    {
      mooseAssert(pr.second, "The FVDirichletBC is null!");

      const auto & bc = *pr.second;

      // Linear interpolation: face_value = (elem_value + neighbor_value) / 2
      return 2. * bc.boundaryValue(fi) - elem_value;
    }
    else
      // No DirichletBC so we'll implicitly apply a zero gradient condition and assume that the
      // face value is equivalent to the element value
      return elem_value;
  }
}

template <typename OutputType>
typename Moose::ADType<typename MooseVariableFV<OutputType>::OutputData>::type
MooseVariableFV<OutputType>::getFaceValue(
    const Elem * const neighbor,
    const FaceInfo & fi,
    const typename Moose::ADType<OutputData>::type & elem_value) const
{
  // Are we on a boundary or an interface beyond which our variable doesn't exist?
  if (!neighbor || !this->hasBlocks(neighbor->subdomain_id()))
  {
    const auto & pr = getDirichletBC(fi);

    if (pr.first)
    {
      const auto & bc = *pr.second;
      return bc.boundaryValue(fi);
    }
    else
    {
      // No DirichletBC so we'll implicitly apply a zero gradient condition and assume that the
      // face value is equivalent to the element value
      return elem_value;
    }
  }

  if (_use_extended_stencil)
  {
    typename Moose::ADType<OutputData>::type numerator = {};
    ADReal denominator = 0;

    for (const Node * const vertex : fi.vertices())
    {
      auto distance = (*vertex - fi.faceCentroid()).norm();

      numerator += getVertexValue(*vertex) * (1.0 / distance);
      denominator += 1. / distance;
    }

    return numerator * (1.0 / denominator);
  }
  else
  {
    // Compact stencil
    auto neighbor_value = getElemValue(neighbor);

    return Moose::FV::linearInterpolation(elem_value, neighbor_value, fi);
  }
}

template <typename OutputType>
const VectorValue<typename Moose::ADType<typename MooseVariableFV<OutputType>::OutputData>::type> &
MooseVariableFV<OutputType>::adGradSln(const Elem * const elem) const
{
  auto it = _elem_to_grad.find(elem);

  if (it != _elem_to_grad.end())
    return it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = _elem_to_grad.emplace(elem, 0);

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  auto & grad = emplace_ret.first->second;

  bool volume_set = false;
  Real volume = 0;

  auto elem_value = getElemValue(elem);

  auto action_functor =
      [&grad, &volume_set, &volume, &elem_value, this](const Elem & functor_elem,
                                                       const Elem * const neighbor,
                                                       const FaceInfo * const fi,
                                                       const Point & surface_vector,
                                                       Real coord,
                                                       const bool elem_has_info) {
        mooseAssert(fi, "We need a FaceInfo for this action_functor");

        grad += getFaceValue(neighbor, *fi, elem_value) * surface_vector;

        if (!volume_set)
        {
          // We use the FaceInfo volumes because those values have been pre-computed and cached. An
          // explicit call to elem->volume() here would incur unnecessary expense
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

  grad *= 1.0 / volume;

  const auto coord_system = this->_subproblem.getCoordSystem(elem->subdomain_id());
  if (coord_system == Moose::CoordinateSystemType::COORD_RZ)
  {
    const auto r_coord = this->_subproblem.getAxisymmetricRadialCoord();
    grad(r_coord) -= elem_value / elem->centroid()(r_coord);
  }

  mooseAssert(coord_system != Moose::CoordinateSystemType::COORD_RSPHERICAL,
              "We have not yet implemented the correct translation from gradient to divergence for "
              "spherical coordinates yet.");

  return grad;
}

template <typename OutputType>
const VectorValue<typename Moose::ADType<typename MooseVariableFV<OutputType>::OutputData>::type> &
MooseVariableFV<OutputType>::uncorrectedAdGradSln(const FaceInfo & fi) const
{
  auto it = _face_to_unc_grad.find(&fi);

  if (it != _face_to_unc_grad.end())
    return it->second;

  auto pr = Moose::FV::determineElemOneAndTwo(fi, *this);
  const Elem * const elem_one = pr.first;
  const Elem * const elem_two = pr.second;

  const auto & elem_one_grad = adGradSln(elem_one);

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = _face_to_unc_grad.emplace(&fi, elem_one_grad);

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  auto & unc_face_grad = emplace_ret.first->second;

  // If we have a neighbor then we interpolate between the two to the face. If we do not, then we
  // check for a Dirichlet BC. If we have a Dirichlet BC, then we will apply a zero Hessian
  // assumption. If we do not, then we know we are applying a zero gradient assumption elsehwere in
  // our calculations, so we should be consistent and apply a zero gradient assumption here as well
  if (elem_two && this->hasBlocks(elem_two->subdomain_id()))
  {
    const auto & elem_two_grad = adGradSln(elem_two);

    // Uncorrected gradient value
    unc_face_grad = Moose::FV::linearInterpolation(elem_one_grad, elem_two_grad, fi);
  }
  else
  {
    const auto & pr = getDirichletBC(fi);

    if (!pr.first)
      unc_face_grad = 0;
  }

  return unc_face_grad;
}

template <typename OutputType>
const VectorValue<typename Moose::ADType<typename MooseVariableFV<OutputType>::OutputData>::type> &
MooseVariableFV<OutputType>::adGradSln(const FaceInfo & fi) const
{
  auto it = _face_to_grad.find(&fi);

  if (it != _face_to_grad.end())
    return it->second;

  // Returns a pair with the first being an iterator pointing to the key-value pair and the second a
  // boolean denoting whether a new insertion took place
  auto emplace_ret = _face_to_grad.emplace(&fi, uncorrectedAdGradSln(fi));

  mooseAssert(emplace_ret.second, "We should have inserted a new key-value pair");

  auto & face_grad = emplace_ret.first->second;

  auto pr = Moose::FV::determineElemOneAndTwo(fi, *this);
  const Elem * const elem_one = pr.first;
  const Elem * const elem_two = pr.second;
  bool elem_is_elem_one = elem_one == &fi.elem();

  const auto elem_one_value = getElemValue(elem_one);
  const auto elem_two_value = getNeighborValue(elem_two, fi, elem_one_value);
  const auto & elem_value = elem_is_elem_one ? elem_one_value : elem_two_value;
  const auto & neighbor_value = elem_is_elem_one ? elem_two_value : elem_one_value;

  // perform the correction. Note that direction is important here because we have a minus sign.
  // Neighbor has to neighbor, and elem has to be elem. Hence all the elem_is_elem_one logic above
  face_grad +=
      ((neighbor_value - elem_value) * (1.0 / fi.dCFMag()) - face_grad * fi.eCF()) * fi.eCF();

  return face_grad;
}

template <>
const ADRealEigenVector &
MooseVariableFV<RealEigenVector>::getVertexValue(const Node & /*vertex*/) const
{
  mooseError("not implemented yet - we accept pull requests");
}

template <>
ADRealEigenVector
MooseVariableFV<RealEigenVector>::getElemValue(const Elem * const /*elem*/) const
{
  mooseError("not implemented yet - we accept pull requests");
}

template <>
ADRealEigenVector
MooseVariableFV<RealEigenVector>::getNeighborValue(const Elem * const /*neighbor*/,
                                                   const FaceInfo & /*fi*/,
                                                   const ADRealEigenVector & /*elem_value*/) const
{
  mooseError("not implemented yet - we accept pull requests");
}

template <>
ADRealEigenVector
MooseVariableFV<RealEigenVector>::getFaceValue(const Elem * const /*neighbor*/,
                                               const FaceInfo & /*fi*/,
                                               const ADRealEigenVector & /*elem_value*/) const
{
  mooseError("not implemented yet - we accept pull requests");
}

template <>
const VectorValue<ADRealEigenVector> &
MooseVariableFV<RealEigenVector>::adGradSln(const Elem * const /*elem*/) const
{
  mooseError("not implemented yet - we accept pull requests");
}

template <>
const VectorValue<ADRealEigenVector> &
MooseVariableFV<RealEigenVector>::uncorrectedAdGradSln(const FaceInfo & /*fi*/) const
{
  mooseError("not implemented yet - we accept pull requests");
}

template <>
const VectorValue<ADRealEigenVector> &
MooseVariableFV<RealEigenVector>::adGradSln(const FaceInfo & /*fi*/) const
{
  mooseError("not implemented yet - we accept pull requests");
}

#endif

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
#ifdef MOOSE_GLOBAL_AD_INDEXING
  _elem_to_grad.clear();
  _face_to_unc_grad.clear();
  _face_to_grad.clear();
  _vertex_to_value.clear();
#endif
}

template class MooseVariableFV<Real>;
template class MooseVariableFV<RealEigenVector>;
// TODO: implement vector fv variable support. This will require some template
// specializations for various member functions in this and the FV variable
// classes. And then you will need to uncomment out the line below:
// template class MooseVariableFV<RealVectorValue>;
