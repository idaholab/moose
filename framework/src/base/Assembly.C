//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Assembly.h"

// MOOSE includes
#include "SubProblem.h"
#include "ArbitraryQuadrature.h"
#include "SystemBase.h"
#include "MooseTypes.h"
#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "XFEMInterface.h"
#include "DisplacedSystem.h"
#include "MooseMeshUtils.h"

// libMesh
#include "libmesh/coupling_matrix.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/equation_systems.h"
#include "libmesh/fe_interface.h"
#include "libmesh/node.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/tensor_value.h"
#include "libmesh/vector_value.h"
#include "libmesh/fe.h"

template <typename P, typename C>
void
coordTransformFactor(const SubProblem & s,
                     const SubdomainID sub_id,
                     const P & point,
                     C & factor,
                     const SubdomainID neighbor_sub_id)
{
  coordTransformFactor(s.mesh(), sub_id, point, factor, neighbor_sub_id);
}

template <typename P, typename C>
void
coordTransformFactor(const MooseMesh & mesh,
                     const SubdomainID sub_id,
                     const P & point,
                     C & factor,
                     const SubdomainID libmesh_dbg_var(neighbor_sub_id))
{
  mooseAssert(neighbor_sub_id != libMesh::Elem::invalid_subdomain_id
                  ? mesh.getCoordSystem(sub_id) == mesh.getCoordSystem(neighbor_sub_id)
                  : true,
              "Coordinate systems must be the same between element and neighbor");
  const auto coord_type = mesh.getCoordSystem(sub_id);
  MooseMeshUtils::coordTransformFactor(
      point,
      factor,
      coord_type,
      coord_type == Moose::COORD_RZ ? mesh.getAxisymmetricRadialCoord() : libMesh::invalid_uint);
}

Assembly::Assembly(SystemBase & sys, THREAD_ID tid)
  : _sys(sys),
    _subproblem(_sys.subproblem()),
    _displaced(dynamic_cast<DisplacedSystem *>(&sys) ? true : false),
    _nonlocal_cm(_subproblem.nonlocalCouplingMatrix(_sys.number())),
    _computing_residual(_subproblem.currentlyComputingResidual()),
    _computing_jacobian(_subproblem.currentlyComputingJacobian()),
    _computing_residual_and_jacobian(_subproblem.currentlyComputingResidualAndJacobian()),
    _dof_map(_sys.dofMap()),
    _tid(tid),
    _mesh(sys.mesh()),
    _mesh_dimension(_mesh.dimension()),
    _current_qrule(nullptr),
    _current_qrule_volume(nullptr),
    _current_qrule_arbitrary(nullptr),
    _coord_type(Moose::COORD_XYZ),
    _current_qrule_face(nullptr),
    _current_qface_arbitrary(nullptr),
    _current_qrule_neighbor(nullptr),
    _need_JxW_neighbor(false),
    _qrule_msm(nullptr),
    _custom_mortar_qrule(false),
    _current_qrule_lower(nullptr),

    _current_elem(nullptr),
    _current_elem_volume(0),
    _current_side(0),
    _current_side_elem(nullptr),
    _current_side_volume(0),
    _current_neighbor_elem(nullptr),
    _current_neighbor_side(0),
    _current_neighbor_side_elem(nullptr),
    _need_neighbor_elem_volume(false),
    _current_neighbor_volume(0),
    _current_node(nullptr),
    _current_neighbor_node(nullptr),
    _current_elem_volume_computed(false),
    _current_side_volume_computed(false),

    _current_lower_d_elem(nullptr),
    _current_neighbor_lower_d_elem(nullptr),
    _need_lower_d_elem_volume(false),
    _need_neighbor_lower_d_elem_volume(false),
    _need_dual(false),

    _residual_vector_tags(_subproblem.getVectorTags(Moose::VECTOR_TAG_RESIDUAL)),
    _cached_residual_values(2), // The 2 is for TIME and NONTIME
    _cached_residual_rows(2),   // The 2 is for TIME and NONTIME
    _max_cached_residuals(0),
    _max_cached_jacobians(0),

    _block_diagonal_matrix(false),
    _calculate_xyz(false),
    _calculate_face_xyz(false),
    _calculate_curvatures(false),
    _calculate_ad_coord(false)
{
  Order helper_order = _mesh.hasSecondOrderElements() ? SECOND : FIRST;
  // Build fe's for the helpers
  buildFE(FEType(helper_order, LAGRANGE));
  buildFaceFE(FEType(helper_order, LAGRANGE));
  buildNeighborFE(FEType(helper_order, LAGRANGE));
  buildFaceNeighborFE(FEType(helper_order, LAGRANGE));
  buildLowerDFE(FEType(helper_order, LAGRANGE));

  // Build an FE helper object for this type for each dimension up to the dimension of the current
  // mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    _holder_fe_helper[dim] = &_fe[dim][FEType(helper_order, LAGRANGE)];
    (*_holder_fe_helper[dim])->get_phi();
    (*_holder_fe_helper[dim])->get_dphi();
    (*_holder_fe_helper[dim])->get_xyz();
    (*_holder_fe_helper[dim])->get_JxW();

    _holder_fe_face_helper[dim] = &_fe_face[dim][FEType(helper_order, LAGRANGE)];
    (*_holder_fe_face_helper[dim])->get_phi();
    (*_holder_fe_face_helper[dim])->get_dphi();
    (*_holder_fe_face_helper[dim])->get_xyz();
    (*_holder_fe_face_helper[dim])->get_JxW();
    (*_holder_fe_face_helper[dim])->get_normals();

    _holder_fe_face_neighbor_helper[dim] = &_fe_face_neighbor[dim][FEType(helper_order, LAGRANGE)];
    (*_holder_fe_face_neighbor_helper[dim])->get_xyz();
    (*_holder_fe_face_neighbor_helper[dim])->get_JxW();
    (*_holder_fe_face_neighbor_helper[dim])->get_normals();

    _holder_fe_neighbor_helper[dim] = &_fe_neighbor[dim][FEType(helper_order, LAGRANGE)];
    (*_holder_fe_neighbor_helper[dim])->get_xyz();
    (*_holder_fe_neighbor_helper[dim])->get_JxW();
  }

  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
  {
    _holder_fe_lower_helper[dim] = &_fe_lower[dim][FEType(helper_order, LAGRANGE)];
    // We need these computations in order to compute correct lower-d element volumes in curvilinear
    // coordinates
    (*_holder_fe_lower_helper[dim])->get_xyz();
    (*_holder_fe_lower_helper[dim])->get_JxW();
  }

  // For 3D mortar, mortar segments are always TRI3 elements so we want FIRST LAGRANGE regardless
  // of discretization
  _fe_msm = (_mesh_dimension == 2)
                ? FEGenericBase<Real>::build(_mesh_dimension - 1, FEType(helper_order, LAGRANGE))
                : FEGenericBase<Real>::build(_mesh_dimension - 1, FEType(FIRST, LAGRANGE));
  _JxW_msm = &_fe_msm->get_JxW();
  // Prerequest xyz so that it is computed for _fe_msm so that it can be used for calculating
  // _coord_msm
  _fe_msm->get_xyz();

  _extra_elem_ids.resize(_mesh.getMesh().n_elem_integers() + 1);
  _neighbor_extra_elem_ids.resize(_mesh.getMesh().n_elem_integers() + 1);
}

Assembly::~Assembly()
{
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _fe[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _fe_face[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _fe_neighbor[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _fe_face_neighbor[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
    for (auto & it : _fe_lower[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _vector_fe[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _vector_fe_face[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _vector_fe_neighbor[dim])
      delete it.second;

  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    for (auto & it : _vector_fe_face_neighbor[dim])
      delete it.second;

  for (auto & it : _ad_grad_phi_data)
    it.second.release();

  for (auto & it : _ad_vector_grad_phi_data)
    it.second.release();

  for (auto & it : _ad_grad_phi_data_face)
    it.second.release();

  for (auto & it : _ad_vector_grad_phi_data_face)
    it.second.release();

  _current_physical_points.release();

  _coord.release();
  _coord_neighbor.release();
  _coord_msm.release();

  _ad_JxW.release();
  _ad_q_points.release();
  _ad_JxW_face.release();
  _ad_normals.release();
  _ad_q_points_face.release();
  _curvatures.release();
  _ad_curvatures.release();
  _ad_coord.release();

  delete _qrule_msm;
}

const MooseArray<Real> &
Assembly::JxWNeighbor() const
{
  _need_JxW_neighbor = true;
  return _current_JxW_neighbor;
}

void
Assembly::buildFE(FEType type) const
{
  if (!_fe_shape_data[type])
    _fe_shape_data[type] = std::make_unique<FEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe[dim][type])
      _fe[dim][type] = FEGenericBase<Real>::build(dim, type).release();

    _fe[dim][type]->get_phi();
    _fe[dim][type]->get_dphi();
    // Pre-request xyz.  We have always computed xyz, but due to
    // recent optimizations in libmesh, we now need to explicity
    // request it, since apps (Yak) may rely on it being computed.
    _fe[dim][type]->get_xyz();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _fe[dim][type]->get_d2phi();
  }
}

void
Assembly::buildFaceFE(FEType type) const
{
  if (!_fe_shape_data_face[type])
    _fe_shape_data_face[type] = std::make_unique<FEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe_face[dim][type])
      _fe_face[dim][type] = FEGenericBase<Real>::build(dim, type).release();

    _fe_face[dim][type]->get_phi();
    _fe_face[dim][type]->get_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _fe_face[dim][type]->get_d2phi();
  }
}

void
Assembly::buildNeighborFE(FEType type) const
{
  if (!_fe_shape_data_neighbor[type])
    _fe_shape_data_neighbor[type] = std::make_unique<FEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe_neighbor[dim][type])
      _fe_neighbor[dim][type] = FEGenericBase<Real>::build(dim, type).release();

    _fe_neighbor[dim][type]->get_phi();
    _fe_neighbor[dim][type]->get_dphi();
    if (_need_second_derivative_neighbor.find(type) != _need_second_derivative_neighbor.end())
      _fe_neighbor[dim][type]->get_d2phi();
  }
}

void
Assembly::buildFaceNeighborFE(FEType type) const
{
  if (!_fe_shape_data_face_neighbor[type])
    _fe_shape_data_face_neighbor[type] = std::make_unique<FEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe_face_neighbor[dim][type])
      _fe_face_neighbor[dim][type] = FEGenericBase<Real>::build(dim, type).release();

    _fe_face_neighbor[dim][type]->get_phi();
    _fe_face_neighbor[dim][type]->get_dphi();
    if (_need_second_derivative_neighbor.find(type) != _need_second_derivative_neighbor.end())
      _fe_face_neighbor[dim][type]->get_d2phi();
  }
}

void
Assembly::buildLowerDFE(FEType type) const
{
  if (!_fe_shape_data_lower[type])
    _fe_shape_data_lower[type] = std::make_unique<FEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of
  // the current mesh minus one (because this is for lower-dimensional
  // elements!)
  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
  {
    if (!_fe_lower[dim][type])
      _fe_lower[dim][type] = FEGenericBase<Real>::build(dim, type).release();

    _fe_lower[dim][type]->get_phi();
    _fe_lower[dim][type]->get_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _fe_lower[dim][type]->get_d2phi();
  }
}

void
Assembly::buildLowerDDualFE(FEType type) const
{
  if (!_fe_shape_data_dual_lower[type])
    _fe_shape_data_dual_lower[type] = std::make_unique<FEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of
  // the current mesh minus one (because this is for lower-dimensional
  // elements!)
  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
  {
    if (!_fe_lower[dim][type])
      _fe_lower[dim][type] = FEGenericBase<Real>::build(dim, type).release();

    _fe_lower[dim][type]->get_dual_phi();
    _fe_lower[dim][type]->get_dual_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _fe_lower[dim][type]->get_dual_d2phi();
  }
}

void
Assembly::buildVectorLowerDFE(FEType type) const
{
  if (!_vector_fe_shape_data_lower[type])
    _vector_fe_shape_data_lower[type] = std::make_unique<VectorFEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of
  // the current mesh minus one (because this is for lower-dimensional
  // elements!)
  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
  {
    if (!_vector_fe_lower[dim][type])
      _vector_fe_lower[dim][type] = FEVectorBase::build(dim, type).release();

    _vector_fe_lower[dim][type]->get_phi();
    _vector_fe_lower[dim][type]->get_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _vector_fe_lower[dim][type]->get_d2phi();
  }
}

void
Assembly::buildVectorDualLowerDFE(FEType type) const
{
  if (!_vector_fe_shape_data_dual_lower[type])
    _vector_fe_shape_data_dual_lower[type] = std::make_unique<VectorFEShapeData>();

  // Build an FE object for this type for each dimension up to the dimension of
  // the current mesh minus one (because this is for lower-dimensional
  // elements!)
  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
  {
    if (!_vector_fe_lower[dim][type])
      _vector_fe_lower[dim][type] = FEVectorBase::build(dim, type).release();

    _vector_fe_lower[dim][type]->get_dual_phi();
    _vector_fe_lower[dim][type]->get_dual_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _vector_fe_lower[dim][type]->get_dual_d2phi();
  }
}

void
Assembly::buildVectorFE(FEType type) const
{
  if (!_vector_fe_shape_data[type])
    _vector_fe_shape_data[type] = std::make_unique<VectorFEShapeData>();

  // Note that NEDELEC_ONE elements can only be built for dimension > 2
  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC || type.family == MONOMIAL_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an FE object for this type for each dimension from the min_dim up to the dimension of the
  // current mesh
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe[dim][type])
      _vector_fe[dim][type] = FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe[dim][type]->get_phi();
    _vector_fe[dim][type]->get_dphi();
    if (type.family == NEDELEC_ONE)
      _vector_fe[dim][type]->get_curl_phi();
    // Pre-request xyz.  We have always computed xyz, but due to
    // recent optimizations in libmesh, we now need to explicity
    // request it, since apps (Yak) may rely on it being computed.
    _vector_fe[dim][type]->get_xyz();
  }
}

void
Assembly::buildVectorFaceFE(FEType type) const
{
  if (!_vector_fe_shape_data_face[type])
    _vector_fe_shape_data_face[type] = std::make_unique<VectorFEShapeData>();

  // Note that NEDELEC_ONE elements can only be built for dimension > 2
  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC || type.family == MONOMIAL_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an FE object for this type for each dimension from the min_dim up to the dimension of the
  // current mesh
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe_face[dim][type])
      _vector_fe_face[dim][type] = FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe_face[dim][type]->get_phi();
    _vector_fe_face[dim][type]->get_dphi();
    if (type.family == NEDELEC_ONE)
      _vector_fe_face[dim][type]->get_curl_phi();
  }
}

void
Assembly::buildVectorNeighborFE(FEType type) const
{
  if (!_vector_fe_shape_data_neighbor[type])
    _vector_fe_shape_data_neighbor[type] = std::make_unique<VectorFEShapeData>();

  // Note that NEDELEC_ONE elements can only be built for dimension > 2
  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC || type.family == MONOMIAL_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an FE object for this type for each dimension from the min_dim up to the dimension of the
  // current mesh
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe_neighbor[dim][type])
      _vector_fe_neighbor[dim][type] = FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe_neighbor[dim][type]->get_phi();
    _vector_fe_neighbor[dim][type]->get_dphi();
    if (type.family == NEDELEC_ONE)
      _vector_fe_neighbor[dim][type]->get_curl_phi();
  }
}

void
Assembly::buildVectorFaceNeighborFE(FEType type) const
{
  if (!_vector_fe_shape_data_face_neighbor[type])
    _vector_fe_shape_data_face_neighbor[type] = std::make_unique<VectorFEShapeData>();

  // Note that NEDELEC_ONE elements can only be built for dimension > 2
  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC || type.family == MONOMIAL_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an FE object for this type for each dimension from the min_dim up to the dimension of the
  // current mesh
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe_face_neighbor[dim][type])
      _vector_fe_face_neighbor[dim][type] =
          FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe_face_neighbor[dim][type]->get_phi();
    _vector_fe_face_neighbor[dim][type]->get_dphi();
    if (type.family == NEDELEC_ONE)
      _vector_fe_face_neighbor[dim][type]->get_curl_phi();
  }
}

void
Assembly::bumpVolumeQRuleOrder(Order volume_order, SubdomainID block)
{
  auto & qdefault = _qrules[Moose::ANY_BLOCK_ID];
  mooseAssert(qdefault.size() > 0, "default quadrature must be initialized before order bumps");

  unsigned int ndims = _mesh_dimension + 1; // must account for 0-dimensional quadrature.
  auto & qvec = _qrules[block];
  if (qvec.size() != ndims || !qvec[0].vol)
    createQRules(qdefault[0].vol->type(),
                 qdefault[0].arbitrary_vol->get_order(),
                 volume_order,
                 qdefault[0].face->get_order(),
                 block);
  else if (qvec[0].vol->get_order() < volume_order)
    createQRules(qvec[0].vol->type(),
                 qvec[0].arbitrary_vol->get_order(),
                 volume_order,
                 qvec[0].face->get_order(),
                 block);
  // otherwise do nothing - quadrature order is already as high as requested
}

void
Assembly::bumpAllQRuleOrder(Order order, SubdomainID block)
{
  auto & qdefault = _qrules[Moose::ANY_BLOCK_ID];
  mooseAssert(qdefault.size() > 0, "default quadrature must be initialized before order bumps");

  unsigned int ndims = _mesh_dimension + 1; // must account for 0-dimensional quadrature.
  auto & qvec = _qrules[block];
  if (qvec.size() != ndims || !qvec[0].vol)
    createQRules(qdefault[0].vol->type(), order, order, order, block);
  else if (qvec[0].vol->get_order() < order || qvec[0].face->get_order() < order)
    createQRules(qvec[0].vol->type(),
                 std::max(order, qvec[0].arbitrary_vol->get_order()),
                 std::max(order, qvec[0].vol->get_order()),
                 std::max(order, qvec[0].face->get_order()),
                 block);
  // otherwise do nothing - quadrature order is already as high as requested
}

void
Assembly::createQRules(QuadratureType type,
                       Order order,
                       Order volume_order,
                       Order face_order,
                       SubdomainID block,
                       bool allow_negative_qweights)
{
  auto & qvec = _qrules[block];
  unsigned int ndims = _mesh_dimension + 1; // must account for 0-dimensional quadrature.
  if (qvec.size() != ndims)
    qvec.resize(ndims);

  for (unsigned int i = 0; i < qvec.size(); i++)
  {
    int dim = i;
    auto & q = qvec[dim];
    q.vol = QBase::build(type, dim, volume_order);
    q.vol->allow_rules_with_negative_weights = allow_negative_qweights;
    q.face = QBase::build(type, dim - 1, face_order);
    q.face->allow_rules_with_negative_weights = allow_negative_qweights;
    q.fv_face = QBase::build(QMONOMIAL, dim - 1, CONSTANT);
    q.fv_face->allow_rules_with_negative_weights = allow_negative_qweights;
    q.neighbor = std::make_unique<ArbitraryQuadrature>(dim - 1, face_order);
    q.neighbor->allow_rules_with_negative_weights = allow_negative_qweights;
    q.arbitrary_vol = std::make_unique<ArbitraryQuadrature>(dim, order);
    q.arbitrary_vol->allow_rules_with_negative_weights = allow_negative_qweights;
    q.arbitrary_face = std::make_unique<ArbitraryQuadrature>(dim - 1, face_order);
    q.arbitrary_face->allow_rules_with_negative_weights = allow_negative_qweights;
  }

  delete _qrule_msm;
  _custom_mortar_qrule = false;
  _qrule_msm = QBase::build(type, _mesh_dimension - 1, face_order).release();
  _qrule_msm->allow_rules_with_negative_weights = allow_negative_qweights;
  _fe_msm->attach_quadrature_rule(_qrule_msm);
}

void
Assembly::setVolumeQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule = qrule;

  if (qrule) // Don't set a NULL qrule
  {
    for (auto & it : _fe[dim])
      it.second->attach_quadrature_rule(qrule);
    for (auto & it : _vector_fe[dim])
      it.second->attach_quadrature_rule(qrule);
  }
}

void
Assembly::setFaceQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_face = qrule;

  for (auto & it : _fe_face[dim])
    it.second->attach_quadrature_rule(qrule);
  for (auto & it : _vector_fe_face[dim])
    it.second->attach_quadrature_rule(qrule);
}

void
Assembly::setLowerQRule(QBase * qrule, unsigned int dim)
{
  // The lower-dimensional quadrature rule matches the face quadrature rule
  setFaceQRule(qrule, dim);

  _current_qrule_lower = qrule;

  for (auto & it : _fe_lower[dim])
    it.second->attach_quadrature_rule(qrule);
  for (auto & it : _vector_fe_lower[dim])
    it.second->attach_quadrature_rule(qrule);
}

void
Assembly::setNeighborQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_neighbor = qrule;

  for (auto & it : _fe_face_neighbor[dim])
    it.second->attach_quadrature_rule(qrule);
  for (auto & it : _vector_fe_face_neighbor[dim])
    it.second->attach_quadrature_rule(qrule);
}

void
Assembly::setMortarQRule(Order order)
{
  if (order != _qrule_msm->get_order())
  {
    // If custom mortar qrule has not yet been specified
    if (!_custom_mortar_qrule)
    {
      _custom_mortar_qrule = true;
      const unsigned int dim = _qrule_msm->get_dim();
      const QuadratureType type = _qrule_msm->type();
      delete _qrule_msm;

      _qrule_msm = QBase::build(type, dim, order).release();
      _fe_msm->attach_quadrature_rule(_qrule_msm);
    }
    else
      mooseError("Mortar quadrature_order: ",
                 order,
                 " does not match previously specified quadrature_order: ",
                 _qrule_msm->get_order(),
                 ". Quadrature_order (when specified) must match for all mortar constraints.");
  }
}

void
Assembly::reinitFE(const Elem * elem)
{
  unsigned int dim = elem->dim();

  for (const auto & it : _fe[dim])
  {
    FEBase & fe = *it.second;
    const FEType & fe_type = it.first;

    _current_fe[fe_type] = &fe;

    FEShapeData & fesd = *_fe_shape_data[fe_type];

    fe.reinit(elem);

    fesd._phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe.get_d2phi()));
  }
  for (const auto & it : _vector_fe[dim])
  {
    FEVectorBase & fe = *it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe[fe_type] = &fe;

    VectorFEShapeData & fesd = *_vector_fe_shape_data[fe_type];

    fe.reinit(elem);

    fesd._phi.shallowCopy(const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(fe.get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd._curl_phi.shallowCopy(
          const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe.get_curl_phi()));
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_helper[dim])->get_xyz()));
  _current_JxW.shallowCopy(const_cast<std::vector<Real> &>((*_holder_fe_helper[dim])->get_JxW()));

  if (_subproblem.haveADObjects())
  {
    auto n_qp = _current_qrule->n_points();
    resizeADMappingObjects(n_qp, dim);
    if (_displaced)
    {
      const auto & qw = _current_qrule->get_weights();
      for (unsigned int qp = 0; qp != n_qp; qp++)
        computeSinglePointMapAD(elem, qw, qp, *_holder_fe_helper[dim]);
    }
    else
      for (unsigned qp = 0; qp < n_qp; ++qp)
      {
        _ad_JxW[qp] = _current_JxW[qp];
        if (_calculate_xyz)
          _ad_q_points[qp] = _current_q_points[qp];
      }

    for (const auto & it : _fe[dim])
    {
      FEBase & fe = *it.second;
      auto fe_type = it.first;
      auto num_shapes = fe.n_shape_functions();
      auto & grad_phi = _ad_grad_phi_data[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      if (_displaced)
        computeGradPhiAD(elem, n_qp, grad_phi, &fe);
      else
      {
        const auto & regular_grad_phi = _fe_shape_data[fe_type]->_grad_phi;
        for (unsigned qp = 0; qp < n_qp; ++qp)
          for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
            grad_phi[i][qp] = regular_grad_phi[i][qp];
      }
    }
    for (const auto & it : _vector_fe[dim])
    {
      FEVectorBase & fe = *it.second;
      auto fe_type = it.first;
      auto num_shapes = fe.n_shape_functions();
      auto & grad_phi = _ad_vector_grad_phi_data[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      if (_displaced)
        computeGradPhiAD(elem, n_qp, grad_phi, &fe);
      else
      {
        const auto & regular_grad_phi = _vector_fe_shape_data[fe_type]->_grad_phi;
        for (unsigned qp = 0; qp < n_qp; ++qp)
          for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
            grad_phi[i][qp] = regular_grad_phi[i][qp];
      }
    }
  }

  auto n = _extra_elem_ids.size() - 1;
  for (auto i : make_range(n))
    _extra_elem_ids[i] = _current_elem->get_extra_integer(i);
  _extra_elem_ids[n] = _current_elem->subdomain_id();

  if (_xfem != nullptr)
    modifyWeightsDueToXFEM(elem);
}

template <typename OutputType>
void
Assembly::computeGradPhiAD(const Elem * elem,
                           unsigned int n_qp,
                           ADTemplateVariablePhiGradient<OutputType> & grad_phi,
                           FEGenericBase<OutputType> * fe)
{
  // This function relies on the fact that FE::reinit has already been called. FE::reinit will
  // importantly have already called FEMap::init_shape_functions which will have computed
  // these quantities at the integration/quadrature points: dphidxi,
  // dphideta, and dphidzeta (e.g. \nabla phi w.r.t. reference coordinates). These *phi* quantities
  // are independent of mesh displacements when using a quadrature rule.
  //
  // Note that a user could have specified custom integration points (e.g. independent of a
  // quadrature rule) which could very well depend on displacements. In that case even the *phi*
  // quantities from the above paragraph would be a function of the displacements and we would be
  // missing that derivative information in the calculations below

  auto dim = elem->dim();
  const auto & dphidxi = fe->get_dphidxi();
  const auto & dphideta = fe->get_dphideta();
  const auto & dphidzeta = fe->get_dphidzeta();
  auto num_shapes = grad_phi.size();

  switch (dim)
  {
    case 0:
    {
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        for (unsigned qp = 0; qp < n_qp; ++qp)
          grad_phi[i][qp] = 0;
      break;
    }

    case 1:
    {
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        for (unsigned qp = 0; qp < n_qp; ++qp)
        {
          grad_phi[i][qp].slice(0) = dphidxi[i][qp] * _ad_dxidx_map[qp];
          grad_phi[i][qp].slice(1) = dphidxi[i][qp] * _ad_dxidy_map[qp];
          grad_phi[i][qp].slice(2) = dphidxi[i][qp] * _ad_dxidz_map[qp];
        }
      break;
    }

    case 2:
    {
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        for (unsigned qp = 0; qp < n_qp; ++qp)
        {
          grad_phi[i][qp].slice(0) =
              dphidxi[i][qp] * _ad_dxidx_map[qp] + dphideta[i][qp] * _ad_detadx_map[qp];
          grad_phi[i][qp].slice(1) =
              dphidxi[i][qp] * _ad_dxidy_map[qp] + dphideta[i][qp] * _ad_detady_map[qp];
          grad_phi[i][qp].slice(2) =
              dphidxi[i][qp] * _ad_dxidz_map[qp] + dphideta[i][qp] * _ad_detadz_map[qp];
        }
      break;
    }

    case 3:
    {
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        for (unsigned qp = 0; qp < n_qp; ++qp)
        {
          grad_phi[i][qp].slice(0) = dphidxi[i][qp] * _ad_dxidx_map[qp] +
                                     dphideta[i][qp] * _ad_detadx_map[qp] +
                                     dphidzeta[i][qp] * _ad_dzetadx_map[qp];
          grad_phi[i][qp].slice(1) = dphidxi[i][qp] * _ad_dxidy_map[qp] +
                                     dphideta[i][qp] * _ad_detady_map[qp] +
                                     dphidzeta[i][qp] * _ad_dzetady_map[qp];
          grad_phi[i][qp].slice(2) = dphidxi[i][qp] * _ad_dxidz_map[qp] +
                                     dphideta[i][qp] * _ad_detadz_map[qp] +
                                     dphidzeta[i][qp] * _ad_dzetadz_map[qp];
        }
      break;
    }
  }
}

void
Assembly::resizeADMappingObjects(unsigned int n_qp, unsigned int dim)
{
  _ad_dxyzdxi_map.resize(n_qp);
  _ad_dxidx_map.resize(n_qp);
  _ad_dxidy_map.resize(n_qp); // 1D element may live in 2D ...
  _ad_dxidz_map.resize(n_qp); // ... or 3D

  if (dim > 1)
  {
    _ad_dxyzdeta_map.resize(n_qp);
    _ad_detadx_map.resize(n_qp);
    _ad_detady_map.resize(n_qp);
    _ad_detadz_map.resize(n_qp);

    if (dim > 2)
    {
      _ad_dxyzdzeta_map.resize(n_qp);
      _ad_dzetadx_map.resize(n_qp);
      _ad_dzetady_map.resize(n_qp);
      _ad_dzetadz_map.resize(n_qp);
    }
  }

  _ad_jac.resize(n_qp);
  _ad_JxW.resize(n_qp);
  if (_calculate_xyz)
    _ad_q_points.resize(n_qp);
}

void
Assembly::computeSinglePointMapAD(const Elem * elem,
                                  const std::vector<Real> & qw,
                                  unsigned p,
                                  FEBase * fe)
{
  // This function relies on the fact that FE::reinit has already been called. FE::reinit will
  // importantly have already called FEMap::init_reference_to_physical_map which will have computed
  // these quantities at the integration/quadrature points: phi_map, dphidxi_map,
  // dphideta_map, and dphidzeta_map (e.g. phi and \nabla phi w.r.t reference coordinates). *_map is
  // used to denote that quantities are in reference to a mapping Lagrange FE object. The FE<Dim,
  // LAGRANGE> objects used for mapping will in general have an order matching the order of the
  // mesh. These *phi*_map quantities are independent of mesh displacements when using a quadrature
  // rule.
  //
  // Note that a user could have specified custom integration points (e.g. independent of a
  // quadrature rule) which could very well depend on displacements. In that case even the *phi*_map
  // quantities from the above paragraph would be a function of the displacements and we would be
  // missing that derivative information in the calculations below
  //
  // Important quantities calculated by this method:
  //   - _ad_JxW;
  //   - _ad_q_points;
  // And the following quantities are important because they are used in the computeGradPhiAD method
  // to calculate the shape function gradients with respect to the physical coordinates
  // dphi/dphys = dphi/dref * dref/dphys:
  //   - _ad_dxidx_map;
  //   - _ad_dxidy_map;
  //   - _ad_dxidz_map;
  //   - _ad_detadx_map;
  //   - _ad_detady_map;
  //   - _ad_detadz_map;
  //   - _ad_dzetadx_map;
  //   - _ad_dzetady_map;
  //   - _ad_dzetadz_map;
  //
  // Some final notes. This method will be called both when we are reinit'ing in the volume and on
  // faces. When reinit'ing on faces, computation of _ad_JxW will be garbage because we will be
  // using dummy quadrature weights. _ad_q_points computation is also currently extraneous during
  // face reinit because we compute _ad_q_points_face in the computeFaceMap method. However,
  // computation of dref/dphys is absolutely necessary (and the reason we call this method for the
  // face case) for both volume and face reinit

  auto dim = elem->dim();
  const auto & elem_nodes = elem->get_nodes();
  auto num_shapes = fe->n_shape_functions();
  const auto & phi_map = fe->get_fe_map().get_phi_map();
  const auto & dphidxi_map = fe->get_fe_map().get_dphidxi_map();
  const auto & dphideta_map = fe->get_fe_map().get_dphideta_map();
  const auto & dphidzeta_map = fe->get_fe_map().get_dphidzeta_map();
  const auto sys_num = _sys.number();
  const bool do_derivatives =
      ADReal::do_derivatives && _sys.number() == _subproblem.currentNlSysNum();

  switch (dim)
  {
    case 0:
    {
      _ad_jac[p] = 1.0;
      _ad_JxW[p] = qw[p];
      if (_calculate_xyz)
        _ad_q_points[p] = *elem_nodes[0];
      break;
    }

    case 1:
    {
      if (_calculate_xyz)
        _ad_q_points[p].zero();

      _ad_dxyzdxi_map[p].zero();

      for (std::size_t i = 0; i < num_shapes; i++)
      {
        libmesh_assert(elem_nodes[i]);
        const Node & node = *elem_nodes[i];
        libMesh::VectorValue<DualReal> elem_point = node;
        if (do_derivatives)
          for (const auto & [disp_num, direction] : _disp_numbers_and_directions)
            if (node.n_dofs(sys_num, disp_num))
              Moose::derivInsert(
                  elem_point(direction).derivatives(), node.dof_number(sys_num, disp_num, 0), 1.);

        _ad_dxyzdxi_map[p].add_scaled(elem_point, dphidxi_map[i][p]);

        if (_calculate_xyz)
          _ad_q_points[p].add_scaled(elem_point, phi_map[i][p]);
      }

      _ad_jac[p] = _ad_dxyzdxi_map[p].norm();

      if (_ad_jac[p].value() <= -TOLERANCE * TOLERANCE)
      {
        static bool failing = false;
        if (!failing)
        {
          failing = true;
          elem->print_info(libMesh::err);
          libmesh_error_msg("ERROR: negative Jacobian " << _ad_jac[p].value() << " at point index "
                                                        << p << " in element " << elem->id());
        }
        else
          return;
      }

      const auto jacm2 = 1. / _ad_jac[p] / _ad_jac[p];
      _ad_dxidx_map[p] = jacm2 * _ad_dxyzdxi_map[p](0);
      _ad_dxidy_map[p] = jacm2 * _ad_dxyzdxi_map[p](1);
      _ad_dxidz_map[p] = jacm2 * _ad_dxyzdxi_map[p](2);

      _ad_JxW[p] = _ad_jac[p] * qw[p];

      break;
    }

    case 2:
    {
      if (_calculate_xyz)
        _ad_q_points[p].zero();
      _ad_dxyzdxi_map[p].zero();
      _ad_dxyzdeta_map[p].zero();

      for (std::size_t i = 0; i < num_shapes; i++)
      {
        libmesh_assert(elem_nodes[i]);
        const Node & node = *elem_nodes[i];
        libMesh::VectorValue<DualReal> elem_point = node;
        if (do_derivatives)
          for (const auto & [disp_num, direction] : _disp_numbers_and_directions)
            Moose::derivInsert(
                elem_point(direction).derivatives(), node.dof_number(sys_num, disp_num, 0), 1.);

        _ad_dxyzdxi_map[p].add_scaled(elem_point, dphidxi_map[i][p]);
        _ad_dxyzdeta_map[p].add_scaled(elem_point, dphideta_map[i][p]);

        if (_calculate_xyz)
          _ad_q_points[p].add_scaled(elem_point, phi_map[i][p]);
      }

      const auto &dx_dxi = _ad_dxyzdxi_map[p](0), dx_deta = _ad_dxyzdeta_map[p](0),
                 dy_dxi = _ad_dxyzdxi_map[p](1), dy_deta = _ad_dxyzdeta_map[p](1),
                 dz_dxi = _ad_dxyzdxi_map[p](2), dz_deta = _ad_dxyzdeta_map[p](2);

      const auto g11 = (dx_dxi * dx_dxi + dy_dxi * dy_dxi + dz_dxi * dz_dxi);

      const auto g12 = (dx_dxi * dx_deta + dy_dxi * dy_deta + dz_dxi * dz_deta);

      const auto g21 = g12;

      const auto g22 = (dx_deta * dx_deta + dy_deta * dy_deta + dz_deta * dz_deta);

      auto det = (g11 * g22 - g12 * g21);

      if (det.value() <= -TOLERANCE * TOLERANCE)
      {
        static bool failing = false;
        if (!failing)
        {
          failing = true;
          elem->print_info(libMesh::err);
          libmesh_error_msg("ERROR: negative Jacobian " << det << " at point index " << p
                                                        << " in element " << elem->id());
        }
        else
          return;
      }
      else if (det.value() <= 0.)
        det.value() = TOLERANCE * TOLERANCE;

      const auto inv_det = 1. / det;
      _ad_jac[p] = std::sqrt(det);

      _ad_JxW[p] = _ad_jac[p] * qw[p];

      const auto g11inv = g22 * inv_det;
      const auto g12inv = -g12 * inv_det;
      const auto g21inv = -g21 * inv_det;
      const auto g22inv = g11 * inv_det;

      _ad_dxidx_map[p] = g11inv * dx_dxi + g12inv * dx_deta;
      _ad_dxidy_map[p] = g11inv * dy_dxi + g12inv * dy_deta;
      _ad_dxidz_map[p] = g11inv * dz_dxi + g12inv * dz_deta;

      _ad_detadx_map[p] = g21inv * dx_dxi + g22inv * dx_deta;
      _ad_detady_map[p] = g21inv * dy_dxi + g22inv * dy_deta;
      _ad_detadz_map[p] = g21inv * dz_dxi + g22inv * dz_deta;

      break;
    }

    case 3:
    {
      if (_calculate_xyz)
        _ad_q_points[p].zero();
      _ad_dxyzdxi_map[p].zero();
      _ad_dxyzdeta_map[p].zero();
      _ad_dxyzdzeta_map[p].zero();

      for (std::size_t i = 0; i < num_shapes; i++)
      {
        libmesh_assert(elem_nodes[i]);
        const Node & node = *elem_nodes[i];
        libMesh::VectorValue<DualReal> elem_point = node;
        if (do_derivatives)
          for (const auto & [disp_num, direction] : _disp_numbers_and_directions)
            Moose::derivInsert(
                elem_point(direction).derivatives(), node.dof_number(sys_num, disp_num, 0), 1.);

        _ad_dxyzdxi_map[p].add_scaled(elem_point, dphidxi_map[i][p]);
        _ad_dxyzdeta_map[p].add_scaled(elem_point, dphideta_map[i][p]);
        _ad_dxyzdzeta_map[p].add_scaled(elem_point, dphidzeta_map[i][p]);

        if (_calculate_xyz)
          _ad_q_points[p].add_scaled(elem_point, phi_map[i][p]);
      }

      const auto dx_dxi = _ad_dxyzdxi_map[p](0), dy_dxi = _ad_dxyzdxi_map[p](1),
                 dz_dxi = _ad_dxyzdxi_map[p](2), dx_deta = _ad_dxyzdeta_map[p](0),
                 dy_deta = _ad_dxyzdeta_map[p](1), dz_deta = _ad_dxyzdeta_map[p](2),
                 dx_dzeta = _ad_dxyzdzeta_map[p](0), dy_dzeta = _ad_dxyzdzeta_map[p](1),
                 dz_dzeta = _ad_dxyzdzeta_map[p](2);

      _ad_jac[p] = (dx_dxi * (dy_deta * dz_dzeta - dz_deta * dy_dzeta) +
                    dy_dxi * (dz_deta * dx_dzeta - dx_deta * dz_dzeta) +
                    dz_dxi * (dx_deta * dy_dzeta - dy_deta * dx_dzeta));

      if (_ad_jac[p].value() <= -TOLERANCE * TOLERANCE)
      {
        static bool failing = false;
        if (!failing)
        {
          failing = true;
          elem->print_info(libMesh::err);
          libmesh_error_msg("ERROR: negative Jacobian " << _ad_jac[p].value() << " at point index "
                                                        << p << " in element " << elem->id());
        }
        else
          return;
      }

      _ad_JxW[p] = _ad_jac[p] * qw[p];

      const auto inv_jac = 1. / _ad_jac[p];

      _ad_dxidx_map[p] = (dy_deta * dz_dzeta - dz_deta * dy_dzeta) * inv_jac;
      _ad_dxidy_map[p] = (dz_deta * dx_dzeta - dx_deta * dz_dzeta) * inv_jac;
      _ad_dxidz_map[p] = (dx_deta * dy_dzeta - dy_deta * dx_dzeta) * inv_jac;

      _ad_detadx_map[p] = (dz_dxi * dy_dzeta - dy_dxi * dz_dzeta) * inv_jac;
      _ad_detady_map[p] = (dx_dxi * dz_dzeta - dz_dxi * dx_dzeta) * inv_jac;
      _ad_detadz_map[p] = (dy_dxi * dx_dzeta - dx_dxi * dy_dzeta) * inv_jac;

      _ad_dzetadx_map[p] = (dy_dxi * dz_deta - dz_dxi * dy_deta) * inv_jac;
      _ad_dzetady_map[p] = (dz_dxi * dx_deta - dx_dxi * dz_deta) * inv_jac;
      _ad_dzetadz_map[p] = (dx_dxi * dy_deta - dy_dxi * dx_deta) * inv_jac;

      break;
    }

    default:
      libmesh_error_msg("Invalid dim = " << dim);
  }
}

void
Assembly::reinitFEFace(const Elem * elem, unsigned int side)
{
  unsigned int dim = elem->dim();

  for (const auto & it : _fe_face[dim])
  {
    FEBase & fe_face = *it.second;
    const FEType & fe_type = it.first;
    FEShapeData & fesd = *_fe_shape_data_face[fe_type];
    fe_face.reinit(elem, side);
    _current_fe_face[fe_type] = &fe_face;

    fesd._phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_face.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face.get_d2phi()));
  }
  for (const auto & it : _vector_fe_face[dim])
  {
    FEVectorBase & fe_face = *it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_face[fe_type] = &fe_face;

    VectorFEShapeData & fesd = *_vector_fe_shape_data_face[fe_type];

    fe_face.reinit(elem, side);

    fesd._phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(fe_face.get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd._curl_phi.shallowCopy(
          const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face.get_curl_phi()));
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points_face.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_xyz()));
  _current_JxW_face.shallowCopy(
      const_cast<std::vector<Real> &>((*_holder_fe_face_helper[dim])->get_JxW()));
  _current_normals.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_normals()));

  _mapped_normals.resize(_current_normals.size(), Eigen::Map<RealDIMValue>(nullptr));
  for (unsigned int i = 0; i < _current_normals.size(); i++)
    // Note: this does NOT do any allocation.  It is "reconstructing" the object in place
    new (&_mapped_normals[i]) Eigen::Map<RealDIMValue>(const_cast<Real *>(&_current_normals[i](0)));

  if (_calculate_curvatures)
    _curvatures.shallowCopy(
        const_cast<std::vector<Real> &>((*_holder_fe_face_helper[dim])->get_curvatures()));

  computeADFace(*elem, side);

  if (_xfem != nullptr)
    modifyFaceWeightsDueToXFEM(elem, side);

  auto n = _extra_elem_ids.size() - 1;
  for (auto i : make_range(n))
    _extra_elem_ids[i] = _current_elem->get_extra_integer(i);
  _extra_elem_ids[n] = _current_elem->subdomain_id();
}

void
Assembly::computeFaceMap(const Elem & elem, const unsigned int side, const std::vector<Real> & qw)
{
  // Important quantities calculated by this method:
  //   - _ad_JxW_face
  //   - _ad_q_points_face
  //   - _ad_normals
  //   - _ad_curvatures

  const Elem & side_elem = _compute_face_map_side_elem_builder(elem, side);
  const auto dim = elem.dim();
  const auto n_qp = qw.size();
  const auto & dpsidxi_map = (*_holder_fe_face_helper[dim])->get_fe_map().get_dpsidxi();
  const auto & dpsideta_map = (*_holder_fe_face_helper[dim])->get_fe_map().get_dpsideta();
  const auto & psi_map = (*_holder_fe_face_helper[dim])->get_fe_map().get_psi();
  std::vector<std::vector<Real>> const * d2psidxi2_map = nullptr;
  std::vector<std::vector<Real>> const * d2psidxideta_map = nullptr;
  std::vector<std::vector<Real>> const * d2psideta2_map = nullptr;
  const auto sys_num = _sys.number();
  const bool do_derivatives = ADReal::do_derivatives && sys_num == _subproblem.currentNlSysNum();

  if (_calculate_curvatures)
  {
    d2psidxi2_map = &(*_holder_fe_face_helper[dim])->get_fe_map().get_d2psidxi2();
    d2psidxideta_map = &(*_holder_fe_face_helper[dim])->get_fe_map().get_d2psidxideta();
    d2psideta2_map = &(*_holder_fe_face_helper[dim])->get_fe_map().get_d2psideta2();
  }

  switch (dim)
  {
    case 1:
    {
      if (!n_qp)
        break;

      if (side_elem.node_id(0) == elem.node_id(0))
        _ad_normals[0] = Point(-1.);
      else
        _ad_normals[0] = Point(1.);

      VectorValue<DualReal> side_point;
      if (_calculate_face_xyz)
      {
        const Node & node = side_elem.node_ref(0);
        side_point = node;

        if (do_derivatives)
          for (const auto & [disp_num, direction] : _disp_numbers_and_directions)
            Moose::derivInsert(
                side_point(direction).derivatives(), node.dof_number(sys_num, disp_num, 0), 1.);
      }

      for (unsigned int p = 0; p < n_qp; p++)
      {
        if (_calculate_face_xyz)
        {
          _ad_q_points_face[p].zero();
          _ad_q_points_face[p].add_scaled(side_point, psi_map[0][p]);
        }

        _ad_normals[p] = _ad_normals[0];
        _ad_JxW_face[p] = 1.0 * qw[p];
      }

      break;
    }

    case 2:
    {
      _ad_dxyzdxi_map.resize(n_qp);
      if (_calculate_curvatures)
        _ad_d2xyzdxi2_map.resize(n_qp);

      for (unsigned int p = 0; p < n_qp; p++)
      {
        _ad_dxyzdxi_map[p].zero();
        if (_calculate_face_xyz)
          _ad_q_points_face[p].zero();
        if (_calculate_curvatures)
          _ad_d2xyzdxi2_map[p].zero();
      }

      const auto n_mapping_shape_functions =
          FE<2, LAGRANGE>::n_shape_functions(side_elem.type(), side_elem.default_order());

      for (unsigned int i = 0; i < n_mapping_shape_functions; i++)
      {
        const Node & node = side_elem.node_ref(i);
        VectorValue<DualReal> side_point = node;

        if (do_derivatives)
          for (const auto & [disp_num, direction] : _disp_numbers_and_directions)
            Moose::derivInsert(
                side_point(direction).derivatives(), node.dof_number(sys_num, disp_num, 0), 1.);

        for (unsigned int p = 0; p < n_qp; p++)
        {
          _ad_dxyzdxi_map[p].add_scaled(side_point, dpsidxi_map[i][p]);
          if (_calculate_face_xyz)
            _ad_q_points_face[p].add_scaled(side_point, psi_map[i][p]);
          if (_calculate_curvatures)
            _ad_d2xyzdxi2_map[p].add_scaled(side_point, (*d2psidxi2_map)[i][p]);
        }
      }

      for (unsigned int p = 0; p < n_qp; p++)
      {
        _ad_normals[p] =
            (VectorValue<DualReal>(_ad_dxyzdxi_map[p](1), -_ad_dxyzdxi_map[p](0), 0.)).unit();
        const auto the_jac = _ad_dxyzdxi_map[p].norm();
        _ad_JxW_face[p] = the_jac * qw[p];
        if (_calculate_curvatures)
        {
          const auto numerator = _ad_d2xyzdxi2_map[p] * _ad_normals[p];
          const auto denominator = _ad_dxyzdxi_map[p].norm_sq();
          libmesh_assert_not_equal_to(denominator, 0);
          _ad_curvatures[p] = numerator / denominator;
        }
      }

      break;
    }

    case 3:
    {
      _ad_dxyzdxi_map.resize(n_qp);
      _ad_dxyzdeta_map.resize(n_qp);
      if (_calculate_curvatures)
      {
        _ad_d2xyzdxi2_map.resize(n_qp);
        _ad_d2xyzdxideta_map.resize(n_qp);
        _ad_d2xyzdeta2_map.resize(n_qp);
      }

      for (unsigned int p = 0; p < n_qp; p++)
      {
        _ad_dxyzdxi_map[p].zero();
        _ad_dxyzdeta_map[p].zero();
        if (_calculate_face_xyz)
          _ad_q_points_face[p].zero();
        if (_calculate_curvatures)
        {
          _ad_d2xyzdxi2_map[p].zero();
          _ad_d2xyzdxideta_map[p].zero();
          _ad_d2xyzdeta2_map[p].zero();
        }
      }

      const unsigned int n_mapping_shape_functions =
          FE<3, LAGRANGE>::n_shape_functions(side_elem.type(), side_elem.default_order());

      for (unsigned int i = 0; i < n_mapping_shape_functions; i++)
      {
        const Node & node = side_elem.node_ref(i);
        VectorValue<DualReal> side_point = node;

        if (do_derivatives)
          for (const auto & [disp_num, direction] : _disp_numbers_and_directions)
            Moose::derivInsert(
                side_point(direction).derivatives(), node.dof_number(sys_num, disp_num, 0), 1.);

        for (unsigned int p = 0; p < n_qp; p++)
        {
          _ad_dxyzdxi_map[p].add_scaled(side_point, dpsidxi_map[i][p]);
          _ad_dxyzdeta_map[p].add_scaled(side_point, dpsideta_map[i][p]);
          if (_calculate_face_xyz)
            _ad_q_points_face[p].add_scaled(side_point, psi_map[i][p]);
          if (_calculate_curvatures)
          {
            _ad_d2xyzdxi2_map[p].add_scaled(side_point, (*d2psidxi2_map)[i][p]);
            _ad_d2xyzdxideta_map[p].add_scaled(side_point, (*d2psidxideta_map)[i][p]);
            _ad_d2xyzdeta2_map[p].add_scaled(side_point, (*d2psideta2_map)[i][p]);
          }
        }
      }

      for (unsigned int p = 0; p < n_qp; p++)
      {
        _ad_normals[p] = _ad_dxyzdxi_map[p].cross(_ad_dxyzdeta_map[p]).unit();

        const auto &dxdxi = _ad_dxyzdxi_map[p](0), dxdeta = _ad_dxyzdeta_map[p](0),
                   dydxi = _ad_dxyzdxi_map[p](1), dydeta = _ad_dxyzdeta_map[p](1),
                   dzdxi = _ad_dxyzdxi_map[p](2), dzdeta = _ad_dxyzdeta_map[p](2);

        const auto g11 = (dxdxi * dxdxi + dydxi * dydxi + dzdxi * dzdxi);

        const auto g12 = (dxdxi * dxdeta + dydxi * dydeta + dzdxi * dzdeta);

        const auto g21 = g12;

        const auto g22 = (dxdeta * dxdeta + dydeta * dydeta + dzdeta * dzdeta);

        const auto the_jac = std::sqrt(g11 * g22 - g12 * g21);

        _ad_JxW_face[p] = the_jac * qw[p];

        if (_calculate_curvatures)
        {
          const auto L = -_ad_d2xyzdxi2_map[p] * _ad_normals[p];
          const auto M = -_ad_d2xyzdxideta_map[p] * _ad_normals[p];
          const auto N = -_ad_d2xyzdeta2_map[p] * _ad_normals[p];
          const auto E = _ad_dxyzdxi_map[p].norm_sq();
          const auto F = _ad_dxyzdxi_map[p] * _ad_dxyzdeta_map[p];
          const auto G = _ad_dxyzdeta_map[p].norm_sq();

          const auto numerator = E * N - 2. * F * M + G * L;
          const auto denominator = E * G - F * F;
          libmesh_assert_not_equal_to(denominator, 0.);
          _ad_curvatures[p] = 0.5 * numerator / denominator;
        }
      }

      break;
    }

    default:
      mooseError("Invalid dimension dim = ", dim);
  }
}

void
Assembly::reinitFEFaceNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points)
{
  unsigned int neighbor_dim = neighbor->dim();

  // reinit neighbor face
  for (const auto & it : _fe_face_neighbor[neighbor_dim])
  {
    FEBase & fe_face_neighbor = *it.second;
    FEType fe_type = it.first;
    FEShapeData & fesd = *_fe_shape_data_face_neighbor[fe_type];

    fe_face_neighbor.reinit(neighbor, &reference_points);

    _current_fe_face_neighbor[fe_type] = &fe_face_neighbor;

    fesd._phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_face_neighbor.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_face_neighbor.get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face_neighbor.get_d2phi()));
  }
  for (const auto & it : _vector_fe_face_neighbor[neighbor_dim])
  {
    FEVectorBase & fe_face_neighbor = *it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_face_neighbor[fe_type] = &fe_face_neighbor;

    VectorFEShapeData & fesd = *_vector_fe_shape_data_face_neighbor[fe_type];

    fe_face_neighbor.reinit(neighbor, &reference_points);

    fesd._phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face_neighbor.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face_neighbor.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(
          fe_face_neighbor.get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd._curl_phi.shallowCopy(const_cast<std::vector<std::vector<VectorValue<Real>>> &>(
          fe_face_neighbor.get_curl_phi()));
  }
}

void
Assembly::reinitFENeighbor(const Elem * neighbor, const std::vector<Point> & reference_points)
{
  unsigned int neighbor_dim = neighbor->dim();

  // reinit neighbor face
  for (const auto & it : _fe_neighbor[neighbor_dim])
  {
    FEBase & fe_neighbor = *it.second;
    FEType fe_type = it.first;
    FEShapeData & fesd = *_fe_shape_data_neighbor[fe_type];

    fe_neighbor.reinit(neighbor, &reference_points);

    _current_fe_neighbor[fe_type] = &fe_neighbor;

    fesd._phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_neighbor.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_neighbor.get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_neighbor.get_d2phi()));
  }
  for (const auto & it : _vector_fe_neighbor[neighbor_dim])
  {
    FEVectorBase & fe_neighbor = *it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_neighbor[fe_type] = &fe_neighbor;

    VectorFEShapeData & fesd = *_vector_fe_shape_data_neighbor[fe_type];

    fe_neighbor.reinit(neighbor, &reference_points);

    fesd._phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_neighbor.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_neighbor.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(fe_neighbor.get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd._curl_phi.shallowCopy(
          const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_neighbor.get_curl_phi()));
  }
}

void
Assembly::reinitNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points)
{
  unsigned int neighbor_dim = neighbor->dim();

  ArbitraryQuadrature * neighbor_rule = qrules(neighbor_dim).neighbor.get();
  neighbor_rule->setPoints(reference_points);
  setNeighborQRule(neighbor_rule, neighbor_dim);

  _current_neighbor_elem = neighbor;
  mooseAssert(_current_neighbor_subdomain_id == _current_neighbor_elem->subdomain_id(),
              "current neighbor subdomain has been set incorrectly");

  // Calculate the volume of the neighbor
  if (_need_neighbor_elem_volume)
  {
    unsigned int dim = neighbor->dim();
    FEBase & fe = **_holder_fe_neighbor_helper[dim];
    QBase * qrule = qrules(dim).vol.get();

    fe.attach_quadrature_rule(qrule);
    fe.reinit(neighbor);

    const std::vector<Real> & JxW = fe.get_JxW();
    MooseArray<Point> q_points;
    q_points.shallowCopy(const_cast<std::vector<Point> &>(fe.get_xyz()));

    setCoordinateTransformation(qrule, q_points, _coord_neighbor, _current_neighbor_subdomain_id);

    _current_neighbor_volume = 0.;
    for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
      _current_neighbor_volume += JxW[qp] * _coord_neighbor[qp];
  }

  auto n = _neighbor_extra_elem_ids.size() - 1;
  for (auto i : make_range(n))
    _neighbor_extra_elem_ids[i] = _current_neighbor_elem->get_extra_integer(i);
  _neighbor_extra_elem_ids[n] = _current_neighbor_elem->subdomain_id();
}

template <typename Points, typename Coords>
void
Assembly::setCoordinateTransformation(const QBase * qrule,
                                      const Points & q_points,
                                      Coords & coord,
                                      SubdomainID sub_id)
{

  mooseAssert(qrule, "The quadrature rule is null in Assembly::setCoordinateTransformation");
  auto n_points = qrule->n_points();
  mooseAssert(n_points == q_points.size(),
              "The number of points in the quadrature rule doesn't match the number of passed-in "
              "points in Assembly::setCoordinateTransformation");

  // Make sure to honor the name of this method and set the _coord_type member because users may
  // make use of the const Moose::CoordinateSystem & coordTransformation() { return _coord_type; }
  // API. MaterialBase for example uses it
  _coord_type = _subproblem.getCoordSystem(sub_id);

  coord.resize(n_points);
  for (unsigned int qp = 0; qp < n_points; qp++)
    coordTransformFactor(_subproblem, sub_id, q_points[qp], coord[qp]);
}

void
Assembly::computeCurrentElemVolume()
{
  if (_current_elem_volume_computed)
    return;

  setCoordinateTransformation(
      _current_qrule, _current_q_points, _coord, _current_elem->subdomain_id());
  if (_calculate_ad_coord)
    setCoordinateTransformation(
        _current_qrule, _ad_q_points, _ad_coord, _current_elem->subdomain_id());

  _current_elem_volume = 0.;
  for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
    _current_elem_volume += _current_JxW[qp] * _coord[qp];

  _current_elem_volume_computed = true;
}

void
Assembly::computeCurrentFaceVolume()
{
  if (_current_side_volume_computed)
    return;

  setCoordinateTransformation(
      _current_qrule_face, _current_q_points_face, _coord, _current_elem->subdomain_id());
  if (_calculate_ad_coord)
    setCoordinateTransformation(
        _current_qrule_face, _ad_q_points_face, _ad_coord, _current_elem->subdomain_id());

  _current_side_volume = 0.;
  for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
    _current_side_volume += _current_JxW_face[qp] * _coord[qp];

  _current_side_volume_computed = true;
}

void
Assembly::reinitAtPhysical(const Elem * elem, const std::vector<Point> & physical_points)
{
  _current_elem = elem;
  _current_neighbor_elem = nullptr;
  mooseAssert(_current_subdomain_id == _current_elem->subdomain_id(),
              "current subdomain has been set incorrectly");
  _current_elem_volume_computed = false;

  FEInterface::inverse_map(elem->dim(),
                           (*_holder_fe_helper[elem->dim()])->get_fe_type(),
                           elem,
                           physical_points,
                           _temp_reference_points);

  reinit(elem, _temp_reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

void
Assembly::reinit(const Elem * elem)
{
  _current_elem = elem;
  _current_neighbor_elem = nullptr;
  mooseAssert(_current_subdomain_id == _current_elem->subdomain_id(),
              "current subdomain has been set incorrectly");
  _current_elem_volume_computed = false;

  unsigned int elem_dimension = elem->dim();

  _current_qrule_volume = qrules(elem_dimension).vol.get();

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_volume)
    setVolumeQRule(_current_qrule_volume, elem_dimension);

  reinitFE(elem);

  computeCurrentElemVolume();
}

void
Assembly::reinit(const Elem * elem, const std::vector<Point> & reference_points)
{
  _current_elem = elem;
  _current_neighbor_elem = nullptr;
  mooseAssert(_current_subdomain_id == _current_elem->subdomain_id(),
              "current subdomain has been set incorrectly");
  _current_elem_volume_computed = false;

  unsigned int elem_dimension = _current_elem->dim();

  _current_qrule_arbitrary = qrules(elem_dimension).arbitrary_vol.get();

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_arbitrary)
    setVolumeQRule(_current_qrule_arbitrary, elem_dimension);

  _current_qrule_arbitrary->setPoints(reference_points);

  reinitFE(elem);

  computeCurrentElemVolume();
}

void
Assembly::reinitFVFace(const FaceInfo & fi)
{
  _current_elem = &fi.elem();
  _current_neighbor_elem = fi.neighborPtr();
  _current_side = fi.elemSideID();
  _current_neighbor_side = fi.neighborSideID();
  mooseAssert(_current_subdomain_id == _current_elem->subdomain_id(),
              "current subdomain has been set incorrectly");

  _current_elem_volume_computed = false;
  _current_side_volume_computed = false;

  prepareResidual();
  prepareNeighbor();
  prepareJacobianBlock();

  unsigned int dim = _current_elem->dim();
  if (_current_qrule_face != qrules(dim).fv_face.get())
  {
    setFaceQRule(qrules(dim).fv_face.get(), dim);
    // The order of the element that is used for initing here doesn't matter since this will just be
    // used for constant monomials (which only need a single integration point)
    if (dim == 3)
      _current_qrule_face->init(QUAD4);
    else
      _current_qrule_face->init(EDGE2);
  }

  _current_side_elem = &_current_side_elem_builder(*_current_elem, _current_side);

  // We've initialized the reference points. Now we need to compute the physical location of the
  // quadrature points. We do not do any FE initialization so we cannot simply copy over FE results
  // like we do in reinitFEFace. Instead we handle the computation of the physical locations
  // manually
  const auto num_qp = _current_qrule_face->n_points();
  _current_q_points_face.resize(num_qp);
  const auto & ref_points = _current_qrule_face->get_points();
  for (const auto qp : make_range(num_qp))
    _current_q_points_face[qp] =
        FEMap::map(_current_side_elem->dim(), _current_side_elem, ref_points[qp]);
}

QBase *
Assembly::qruleFace(const Elem * elem, unsigned int side)
{
  return qruleFaceHelper<QBase>(elem, side, [](QRules & q) { return q.face.get(); });
}

ArbitraryQuadrature *
Assembly::qruleArbitraryFace(const Elem * elem, unsigned int side)
{
  return qruleFaceHelper<ArbitraryQuadrature>(
      elem, side, [](QRules & q) { return q.arbitrary_face.get(); });
}

void
Assembly::reinit(const Elem * elem, unsigned int side)
{
  _current_elem = elem;
  _current_neighbor_elem = nullptr;
  mooseAssert(_current_subdomain_id == _current_elem->subdomain_id(),
              "current subdomain has been set incorrectly");
  _current_side = side;
  _current_elem_volume_computed = false;
  _current_side_volume_computed = false;

  unsigned int elem_dimension = elem->dim();

  _current_side_elem = &_current_side_elem_builder(*elem, side);

  //// Make sure the qrule is the right one
  auto rule = qruleFace(elem, side);
  if (_current_qrule_face != rule)
    setFaceQRule(rule, elem_dimension);

  reinitFEFace(elem, side);

  computeCurrentFaceVolume();
}

void
Assembly::reinit(const Elem * elem, unsigned int side, const std::vector<Point> & reference_points)
{
  _current_elem = elem;
  _current_neighbor_elem = nullptr;
  mooseAssert(_current_subdomain_id == _current_elem->subdomain_id(),
              "current subdomain has been set incorrectly");
  _current_side = side;
  _current_elem_volume_computed = false;
  _current_side_volume_computed = false;

  unsigned int elem_dimension = _current_elem->dim();

  _current_qrule_arbitrary_face = qruleArbitraryFace(elem, side);

  // Make sure the qrule is the right one
  if (_current_qrule_face != _current_qrule_arbitrary_face)
    setFaceQRule(_current_qrule_arbitrary_face, elem_dimension);

  _current_qrule_arbitrary->setPoints(reference_points);

  _current_side_elem = &_current_side_elem_builder(*elem, side);

  reinitFEFace(elem, side);

  computeCurrentFaceVolume();
}

void
Assembly::reinit(const Node * node)
{
  _current_node = node;
  _current_neighbor_node = NULL;
}

void
Assembly::reinitElemAndNeighbor(const Elem * elem,
                                unsigned int side,
                                const Elem * neighbor,
                                unsigned int neighbor_side,
                                const std::vector<Point> * neighbor_reference_points)
{
  _current_neighbor_side = neighbor_side;

  reinit(elem, side);

  unsigned int neighbor_dim = neighbor->dim();

  const std::vector<Point> * reference_points_ptr;
  std::vector<Point> reference_points;

  if (neighbor_reference_points)
    reference_points_ptr = neighbor_reference_points;
  else
  {
    FEInterface::inverse_map(
        neighbor_dim, FEType(), neighbor, _current_q_points_face.stdVector(), reference_points);
    reference_points_ptr = &reference_points;
  }

  _current_neighbor_side_elem = &_current_neighbor_side_elem_builder(*neighbor, neighbor_side);

  if (_need_JxW_neighbor)
  {
    // first do the side element. We need to do this to at a minimum get the correct JxW for the
    // neighbor face.
    reinitFEFaceNeighbor(_current_neighbor_side_elem, *reference_points_ptr);

    // compute JxW on the neighbor's face
    _current_JxW_neighbor.shallowCopy(const_cast<std::vector<Real> &>(
        (*_holder_fe_face_neighbor_helper[_current_neighbor_side_elem->dim()])->get_JxW()));
  }

  reinitFEFaceNeighbor(neighbor, *reference_points_ptr);
  reinitNeighbor(neighbor, *reference_points_ptr);
}

void
Assembly::reinitElemFaceRef(const Elem * elem,
                            unsigned int elem_side,
                            Real tolerance,
                            const std::vector<Point> * const pts,
                            const std::vector<Real> * const weights)
{
  _current_elem = elem;

  unsigned int elem_dim = elem->dim();

  // Attach the quadrature rules
  if (pts)
  {
    auto face_rule = qruleArbitraryFace(elem, elem_side);
    face_rule->setPoints(*pts);
    setFaceQRule(face_rule, elem_dim);
  }
  else
  {
    auto rule = qruleFace(elem, elem_side);
    if (_current_qrule_face != rule)
      setFaceQRule(rule, elem_dim);
  }

  // reinit face
  for (const auto & it : _fe_face[elem_dim])
  {
    FEBase & fe_face = *it.second;
    FEType fe_type = it.first;
    FEShapeData & fesd = *_fe_shape_data_face[fe_type];

    fe_face.reinit(elem, elem_side, tolerance, pts, weights);

    _current_fe_face[fe_type] = &fe_face;

    fesd._phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_face.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_face.get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face.get_d2phi()));
  }
  for (const auto & it : _vector_fe_face[elem_dim])
  {
    FEVectorBase & fe_face = *it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_face[fe_type] = &fe_face;

    VectorFEShapeData & fesd = *_vector_fe_shape_data_face[fe_type];

    fe_face.reinit(elem, elem_side, tolerance, pts, weights);

    fesd._phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(fe_face.get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd._curl_phi.shallowCopy(
          const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face.get_curl_phi()));
  }
  // During that last loop the helper objects will have been reinitialized
  _current_q_points_face.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_face_helper[elem_dim])->get_xyz()));
  _current_normals.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_face_helper[elem_dim])->get_normals()));
  _current_tangents.shallowCopy(const_cast<std::vector<std::vector<Point>> &>(
      (*_holder_fe_face_helper[elem_dim])->get_tangents()));
  // Note that if the user did pass in points and not weights to this method, JxW will be garbage
  // and should not be used
  _current_JxW_face.shallowCopy(
      const_cast<std::vector<Real> &>((*_holder_fe_face_helper[elem_dim])->get_JxW()));
  if (_calculate_curvatures)
    _curvatures.shallowCopy(
        const_cast<std::vector<Real> &>((*_holder_fe_face_helper[elem_dim])->get_curvatures()));

  computeADFace(*elem, elem_side);
}

void
Assembly::computeADFace(const Elem & elem, const unsigned int side)
{
  const auto dim = elem.dim();

  if (_subproblem.haveADObjects())
  {
    auto n_qp = _current_qrule_face->n_points();
    resizeADMappingObjects(n_qp, dim);
    _ad_normals.resize(n_qp);
    _ad_JxW_face.resize(n_qp);
    if (_calculate_face_xyz)
      _ad_q_points_face.resize(n_qp);
    if (_calculate_curvatures)
      _ad_curvatures.resize(n_qp);

    if (_displaced)
    {
      const auto & qw = _current_qrule_face->get_weights();
      computeFaceMap(elem, side, qw);
      const std::vector<Real> dummy_qw(n_qp, 1.);

      for (unsigned int qp = 0; qp != n_qp; qp++)
        computeSinglePointMapAD(&elem, dummy_qw, qp, *_holder_fe_face_helper[dim]);
    }
    else
      for (unsigned qp = 0; qp < n_qp; ++qp)
      {
        _ad_JxW_face[qp] = _current_JxW_face[qp];
        if (_calculate_face_xyz)
          _ad_q_points_face[qp] = _current_q_points_face[qp];
        _ad_normals[qp] = _current_normals[qp];
        if (_calculate_curvatures)
          _ad_curvatures[qp] = _curvatures[qp];
      }

    for (const auto & it : _fe_face[dim])
    {
      FEBase & fe = *it.second;
      auto fe_type = it.first;
      auto num_shapes = fe.n_shape_functions();
      auto & grad_phi = _ad_grad_phi_data_face[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      const auto & regular_grad_phi = _fe_shape_data_face[fe_type]->_grad_phi;

      if (_displaced)
        computeGradPhiAD(&elem, n_qp, grad_phi, &fe);
      else
        for (unsigned qp = 0; qp < n_qp; ++qp)
          for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
            grad_phi[i][qp] = regular_grad_phi[i][qp];
    }
    for (const auto & it : _vector_fe_face[dim])
    {
      FEVectorBase & fe = *it.second;
      auto fe_type = it.first;
      auto num_shapes = fe.n_shape_functions();
      auto & grad_phi = _ad_vector_grad_phi_data_face[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      const auto & regular_grad_phi = _vector_fe_shape_data_face[fe_type]->_grad_phi;

      if (_displaced)
        computeGradPhiAD(&elem, n_qp, grad_phi, &fe);
      else
        for (unsigned qp = 0; qp < n_qp; ++qp)
          for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
            grad_phi[i][qp] = regular_grad_phi[i][qp];
    }
  }
}

void
Assembly::reinitNeighborFaceRef(const Elem * neighbor,
                                unsigned int neighbor_side,
                                Real tolerance,
                                const std::vector<Point> * const pts,
                                const std::vector<Real> * const weights)
{
  _current_neighbor_elem = neighbor;

  unsigned int neighbor_dim = neighbor->dim();

  ArbitraryQuadrature * neighbor_rule = qrules(neighbor_dim).neighbor.get();
  neighbor_rule->setPoints(*pts);

  // Attach this quadrature rule to all the _fe_face_neighbor FE objects. This
  // has to have garbage quadrature weights but that's ok because we never
  // actually use the JxW coming from these FE reinit'd objects, e.g. we use the
  // JxW coming from the element face reinit for DGKernels or we use the JxW
  // coming from reinit of the mortar segment element in the case of mortar
  setNeighborQRule(neighbor_rule, neighbor_dim);

  // reinit neighbor face
  for (const auto & it : _fe_face_neighbor[neighbor_dim])
  {
    FEBase & fe_face_neighbor = *it.second;
    FEType fe_type = it.first;
    FEShapeData & fesd = *_fe_shape_data_face_neighbor[fe_type];

    fe_face_neighbor.reinit(neighbor, neighbor_side, tolerance, pts, weights);

    _current_fe_face_neighbor[fe_type] = &fe_face_neighbor;

    fesd._phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_face_neighbor.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_face_neighbor.get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd._second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face_neighbor.get_d2phi()));
  }
  for (const auto & it : _vector_fe_face_neighbor[neighbor_dim])
  {
    FEVectorBase & fe_face_neighbor = *it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_face_neighbor[fe_type] = &fe_face_neighbor;

    VectorFEShapeData & fesd = *_vector_fe_shape_data_face_neighbor[fe_type];

    fe_face_neighbor.reinit(neighbor, neighbor_side, tolerance, pts, weights);

    fesd._phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face_neighbor.get_phi()));
    fesd._grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face_neighbor.get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd._second_phi.shallowCopy(const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(
          fe_face_neighbor.get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd._curl_phi.shallowCopy(const_cast<std::vector<std::vector<VectorValue<Real>>> &>(
          fe_face_neighbor.get_curl_phi()));
  }
  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points from it
  _current_q_points_face_neighbor.shallowCopy(const_cast<std::vector<Point> &>(
      (*_holder_fe_face_neighbor_helper[neighbor_dim])->get_xyz()));
  _current_neighbor_normals.shallowCopy(const_cast<std::vector<Point> &>(
      (*_holder_fe_face_neighbor_helper[neighbor_dim])->get_normals()));
}

void
Assembly::reinitDual(const Elem * elem,
                     const std::vector<Point> & pts,
                     const std::vector<Real> & JxW)
{
  const unsigned int elem_dim = elem->dim();
  mooseAssert(elem_dim == _mesh_dimension - 1,
              "Dual shape functions should only be computed on lower dimensional face elements");

  for (const auto & it : _fe_lower[elem_dim])
  {
    FEBase & fe_lower = *it.second;
    // We use customized quadrature rule for integration along the mortar segment elements
    fe_lower.set_calculate_default_dual_coeff(false);
    fe_lower.reinit_dual_shape_coeffs(elem, pts, JxW);
  }
}

void
Assembly::reinitLowerDElem(const Elem * elem,
                           const std::vector<Point> * const pts,
                           const std::vector<Real> * const weights)
{
  _current_lower_d_elem = elem;

  const unsigned int elem_dim = elem->dim();
  mooseAssert(elem_dim < _mesh_dimension,
              "The lower dimensional element should truly be a lower dimensional element");

  if (pts)
  {
    // Lower rule matches the face rule for the higher dimensional element
    ArbitraryQuadrature * lower_rule = qrules(elem_dim + 1).arbitrary_face.get();

    // This also sets the quadrature weights to unity
    lower_rule->setPoints(*pts);

    if (weights)
      lower_rule->setWeights(*weights);

    setLowerQRule(lower_rule, elem_dim);
  }
  else if (_current_qrule_lower != qrules(elem_dim + 1).face.get())
    setLowerQRule(qrules(elem_dim + 1).face.get(), elem_dim);

  for (const auto & it : _fe_lower[elem_dim])
  {
    FEBase & fe_lower = *it.second;
    FEType fe_type = it.first;

    fe_lower.reinit(elem);

    if (FEShapeData * fesd = _fe_shape_data_lower[fe_type].get())
    {
      fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_lower.get_phi()));
      fesd->_grad_phi.shallowCopy(
          const_cast<std::vector<std::vector<RealGradient>> &>(fe_lower.get_dphi()));
      if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
        fesd->_second_phi.shallowCopy(
            const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_lower.get_d2phi()));
    }

    // Dual shape functions need to be computed after primal basis being initialized
    if (FEShapeData * fesd = _fe_shape_data_dual_lower[fe_type].get())
    {
      fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_lower.get_dual_phi()));
      fesd->_grad_phi.shallowCopy(
          const_cast<std::vector<std::vector<RealGradient>> &>(fe_lower.get_dual_dphi()));
      if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
        fesd->_second_phi.shallowCopy(
            const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_lower.get_dual_d2phi()));
    }
  }

  if (!_need_lower_d_elem_volume)
    return;

  if (pts && !weights)
  {
    // We only have dummy weights so the JxWs computed during our FE reinits are meaningless and we
    // cannot use them

    if (_subproblem.getCoordSystem(elem->subdomain_id()) == Moose::CoordinateSystemType::COORD_XYZ)
      // We are in a Cartesian coordinate system and we can just use the element volume method which
      // has fast computation for certain element types
      _current_lower_d_elem_volume = elem->volume();
    else
      // We manually compute the volume taking the curvilinear coordinate transformations into
      // account
      _current_lower_d_elem_volume = elementVolume(elem);
  }
  else
  {
    // During that last loop the helper objects will have been reinitialized as well
    FEBase & helper_fe = **_holder_fe_lower_helper[elem_dim];
    const auto & physical_q_points = helper_fe.get_xyz();
    const auto & JxW = helper_fe.get_JxW();
    MooseArray<Real> coord;
    setCoordinateTransformation(
        _current_qrule_lower, physical_q_points, coord, elem->subdomain_id());
    _current_lower_d_elem_volume = 0;
    for (const auto qp : make_range(_current_qrule_lower->n_points()))
      _current_lower_d_elem_volume += JxW[qp] * coord[qp];
  }
}

void
Assembly::reinitNeighborLowerDElem(const Elem * elem)
{
  mooseAssert(elem->dim() < _mesh_dimension,
              "You should be calling reinitNeighborLowerDElem on a lower dimensional element");

  _current_neighbor_lower_d_elem = elem;

  if (!_need_neighbor_lower_d_elem_volume)
    return;

  if (_subproblem.getCoordSystem(elem->subdomain_id()) == Moose::CoordinateSystemType::COORD_XYZ)
    // We are in a Cartesian coordinate system and we can just use the element volume method which
    // has fast computation for certain element types
    _current_neighbor_lower_d_elem_volume = elem->volume();
  else
    // We manually compute the volume taking the curvilinear coordinate transformations into
    // account
    _current_neighbor_lower_d_elem_volume = elementVolume(elem);
}

void
Assembly::reinitMortarElem(const Elem * elem)
{
  mooseAssert(elem->dim() == _mesh_dimension - 1,
              "You should be calling reinitMortarElem on a lower dimensional element");

  _fe_msm->reinit(elem);
  _msm_elem = elem;

  MooseArray<Point> array_q_points;
  array_q_points.shallowCopy(const_cast<std::vector<Point> &>(_fe_msm->get_xyz()));
  setCoordinateTransformation(_qrule_msm, array_q_points, _coord_msm, elem->subdomain_id());
}

void
Assembly::reinitNeighborAtPhysical(const Elem * neighbor,
                                   unsigned int neighbor_side,
                                   const std::vector<Point> & physical_points)
{
  _current_neighbor_side_elem = &_current_neighbor_side_elem_builder(*neighbor, neighbor_side);

  std::vector<Point> reference_points;

  unsigned int neighbor_dim = neighbor->dim();
  FEInterface::inverse_map(neighbor_dim, FEType(), neighbor, physical_points, reference_points);

  // first do the side element
  reinitFEFaceNeighbor(_current_neighbor_side_elem, reference_points);
  reinitNeighbor(_current_neighbor_side_elem, reference_points);
  // compute JxW on the neighbor's face
  unsigned int neighbor_side_dim = _current_neighbor_side_elem->dim();
  _current_JxW_neighbor.shallowCopy(const_cast<std::vector<Real> &>(
      (*_holder_fe_face_neighbor_helper[neighbor_side_dim])->get_JxW()));

  reinitFEFaceNeighbor(neighbor, reference_points);
  reinitNeighbor(neighbor, reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

void
Assembly::reinitNeighborAtPhysical(const Elem * neighbor,
                                   const std::vector<Point> & physical_points)
{
  std::vector<Point> reference_points;

  unsigned int neighbor_dim = neighbor->dim();
  FEInterface::inverse_map(neighbor_dim, FEType(), neighbor, physical_points, reference_points);

  reinitFENeighbor(neighbor, reference_points);
  reinitNeighbor(neighbor, reference_points);
  // Save off the physical points
  _current_physical_points = physical_points;
}

void
Assembly::init(const CouplingMatrix * cm)
{
  _cm = cm;

  unsigned int n_vars = _sys.nVariables();

  _cm_ss_entry.clear();
  _cm_sf_entry.clear();
  _cm_fs_entry.clear();
  _cm_ff_entry.clear();

  auto & vars = _sys.getVariables(_tid);

  _block_diagonal_matrix = true;
  for (auto & ivar : vars)
  {
    auto i = ivar->number();
    if (i >= _component_block_diagonal.size())
      _component_block_diagonal.resize(i + 1, true);

    auto ivar_start = _cm_ff_entry.size();
    for (unsigned int k = 0; k < ivar->count(); ++k)
    {
      unsigned int iv = i + k;
      for (const auto & j : ConstCouplingRow(iv, *_cm))
      {
        if (_sys.isScalarVariable(j))
        {
          auto & jvar = _sys.getScalarVariable(_tid, j);
          _cm_fs_entry.push_back(std::make_pair(ivar, &jvar));
          _block_diagonal_matrix = false;
        }
        else
        {
          auto & jvar = _sys.getVariable(_tid, j);
          auto pair = std::make_pair(ivar, &jvar);
          auto c = ivar_start;
          // check if the pair has been pushed or not
          bool has_pair = false;
          for (; c < _cm_ff_entry.size(); ++c)
            if (_cm_ff_entry[c] == pair)
            {
              has_pair = true;
              break;
            }
          if (!has_pair)
            _cm_ff_entry.push_back(pair);
          // only set having diagonal matrix to false when ivar and jvar numbers are different
          // Note: for array variables, since we save the entire local Jacobian of all components,
          //       even there are couplings among components of the same array variable, we still
          //       do not set the flag to false.
          if (i != jvar.number())
            _block_diagonal_matrix = false;
          else if (iv != j)
            _component_block_diagonal[i] = false;
        }
      }
    }
  }

  auto & scalar_vars = _sys.getScalarVariables(_tid);

  for (auto & ivar : scalar_vars)
  {
    auto i = ivar->number();
    if (i >= _component_block_diagonal.size())
      _component_block_diagonal.resize(i + 1, true);

    for (const auto & j : ConstCouplingRow(i, *_cm))
      if (_sys.isScalarVariable(j))
      {
        auto & jvar = _sys.getScalarVariable(_tid, j);
        _cm_ss_entry.push_back(std::make_pair(ivar, &jvar));
      }
      else
      {
        auto & jvar = _sys.getVariable(_tid, j);
        _cm_sf_entry.push_back(std::make_pair(ivar, &jvar));
      }
  }

  if (_block_diagonal_matrix && scalar_vars.size() != 0)
    _block_diagonal_matrix = false;

  auto num_vector_tags = _residual_vector_tags.size();

  _sub_Re.resize(num_vector_tags);
  _sub_Rn.resize(num_vector_tags);
  _sub_Rl.resize(num_vector_tags);
  for (MooseIndex(_sub_Re) i = 0; i < _sub_Re.size(); i++)
  {
    _sub_Re[i].resize(n_vars);
    _sub_Rn[i].resize(n_vars);
    _sub_Rl[i].resize(n_vars);
  }

  _cached_residual_values.resize(num_vector_tags);
  _cached_residual_rows.resize(num_vector_tags);

  auto num_matrix_tags = _subproblem.numMatrixTags();

  _cached_jacobian_values.resize(num_matrix_tags);
  _cached_jacobian_rows.resize(num_matrix_tags);
  _cached_jacobian_cols.resize(num_matrix_tags);

  // Element matrices
  _sub_Kee.resize(num_matrix_tags);
  _sub_Keg.resize(num_matrix_tags);
  _sub_Ken.resize(num_matrix_tags);
  _sub_Kne.resize(num_matrix_tags);
  _sub_Knn.resize(num_matrix_tags);
  _sub_Kll.resize(num_matrix_tags);
  _sub_Kle.resize(num_matrix_tags);
  _sub_Kln.resize(num_matrix_tags);
  _sub_Kel.resize(num_matrix_tags);
  _sub_Knl.resize(num_matrix_tags);

  _jacobian_block_used.resize(num_matrix_tags);
  _jacobian_block_neighbor_used.resize(num_matrix_tags);
  _jacobian_block_lower_used.resize(num_matrix_tags);
  _jacobian_block_nonlocal_used.resize(num_matrix_tags);

  for (MooseIndex(num_matrix_tags) tag = 0; tag < num_matrix_tags; tag++)
  {
    _sub_Keg[tag].resize(n_vars);
    _sub_Ken[tag].resize(n_vars);
    _sub_Kne[tag].resize(n_vars);
    _sub_Knn[tag].resize(n_vars);
    _sub_Kee[tag].resize(n_vars);
    _sub_Kll[tag].resize(n_vars);
    _sub_Kle[tag].resize(n_vars);
    _sub_Kln[tag].resize(n_vars);
    _sub_Kel[tag].resize(n_vars);
    _sub_Knl[tag].resize(n_vars);

    _jacobian_block_used[tag].resize(n_vars);
    _jacobian_block_neighbor_used[tag].resize(n_vars);
    _jacobian_block_lower_used[tag].resize(n_vars);
    _jacobian_block_nonlocal_used[tag].resize(n_vars);
    for (MooseIndex(n_vars) i = 0; i < n_vars; ++i)
    {
      if (!_block_diagonal_matrix)
      {
        _sub_Kee[tag][i].resize(n_vars);
        _sub_Keg[tag][i].resize(n_vars);
        _sub_Ken[tag][i].resize(n_vars);
        _sub_Kne[tag][i].resize(n_vars);
        _sub_Knn[tag][i].resize(n_vars);
        _sub_Kll[tag][i].resize(n_vars);
        _sub_Kle[tag][i].resize(n_vars);
        _sub_Kln[tag][i].resize(n_vars);
        _sub_Kel[tag][i].resize(n_vars);
        _sub_Knl[tag][i].resize(n_vars);

        _jacobian_block_used[tag][i].resize(n_vars);
        _jacobian_block_neighbor_used[tag][i].resize(n_vars);
        _jacobian_block_lower_used[tag][i].resize(n_vars);
        _jacobian_block_nonlocal_used[tag][i].resize(n_vars);
      }
      else
      {
        _sub_Kee[tag][i].resize(1);
        _sub_Keg[tag][i].resize(1);
        _sub_Ken[tag][i].resize(1);
        _sub_Kne[tag][i].resize(1);
        _sub_Knn[tag][i].resize(1);
        _sub_Kll[tag][i].resize(1);
        _sub_Kle[tag][i].resize(1);
        _sub_Kln[tag][i].resize(1);
        _sub_Kel[tag][i].resize(1);
        _sub_Knl[tag][i].resize(1);

        _jacobian_block_used[tag][i].resize(1);
        _jacobian_block_neighbor_used[tag][i].resize(1);
        _jacobian_block_lower_used[tag][i].resize(1);
        _jacobian_block_nonlocal_used[tag][i].resize(1);
      }
    }
  }
}

void
Assembly::initNonlocalCoupling()
{
  _cm_nonlocal_entry.clear();

  auto & vars = _sys.getVariables(_tid);

  for (auto & ivar : vars)
  {
    auto i = ivar->number();
    auto ivar_start = _cm_nonlocal_entry.size();
    for (unsigned int k = 0; k < ivar->count(); ++k)
    {
      unsigned int iv = i + k;
      for (const auto & j : ConstCouplingRow(iv, _nonlocal_cm))
        if (!_sys.isScalarVariable(j))
        {
          auto & jvar = _sys.getVariable(_tid, j);
          auto pair = std::make_pair(ivar, &jvar);
          auto c = ivar_start;
          // check if the pair has been pushed or not
          bool has_pair = false;
          for (; c < _cm_nonlocal_entry.size(); ++c)
            if (_cm_nonlocal_entry[c] == pair)
            {
              has_pair = true;
              break;
            }
          if (!has_pair)
            _cm_nonlocal_entry.push_back(pair);
        }
    }
  }
}

void
Assembly::prepareJacobianBlock()
{
  for (const auto & it : _cm_ff_entry)
  {
    MooseVariableFEBase & ivar = *(it.first);
    MooseVariableFEBase & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
    {
      jacobianBlock(vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndices().size() * ivar.count(), jvar.dofIndices().size() * jcount);
      jacobianBlockUsed(tag, vi, vj, false);
    }
  }
}

void
Assembly::prepareResidual()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (auto & tag_Re : _sub_Re)
      tag_Re[var->number()].resize(var->dofIndices().size() * var->count());
}

void
Assembly::prepare()
{
  prepareJacobianBlock();
  prepareResidual();
}

void
Assembly::prepareNonlocal()
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    MooseVariableFEBase & ivar = *(it.first);
    MooseVariableFEBase & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
    {
      jacobianBlockNonlocal(vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndices().size() * ivar.count(), jvar.allDofIndices().size() * jcount);
      jacobianBlockNonlocalUsed(tag, vi, vj, false);
    }
  }
}

void
Assembly::prepareVariable(MooseVariableFEBase * var)
{
  for (const auto & it : _cm_ff_entry)
  {
    MooseVariableFEBase & ivar = *(it.first);
    MooseVariableFEBase & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    if (vi == var->number() || vj == var->number())
    {
      for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(vi, vj, LocalDataKey{}, tag)
            .resize(ivar.dofIndices().size() * ivar.count(), jvar.dofIndices().size() * jcount);
        jacobianBlockUsed(tag, vi, vj, false);
      }
    }
  }

  for (auto & tag_Re : _sub_Re)
    tag_Re[var->number()].resize(var->dofIndices().size() * var->count());
}

void
Assembly::prepareVariableNonlocal(MooseVariableFEBase * var)
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    MooseVariableFEBase & ivar = *(it.first);
    MooseVariableFEBase & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    if (vi == var->number() || vj == var->number())
    {
      for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
           tag < _jacobian_block_nonlocal_used.size();
           tag++)
      {
        jacobianBlockNonlocal(vi, vj, LocalDataKey{}, tag)
            .resize(ivar.dofIndices().size() * ivar.count(), jvar.allDofIndices().size() * jcount);
        jacobianBlockNonlocalUsed(tag, vi, vj);
      }
    }
  }
}

void
Assembly::prepareNeighbor()
{
  for (const auto & it : _cm_ff_entry)
  {
    MooseVariableFEBase & ivar = *(it.first);
    MooseVariableFEBase & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    for (MooseIndex(_jacobian_block_neighbor_used) tag = 0;
         tag < _jacobian_block_neighbor_used.size();
         tag++)
    {
      jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndices().size() * ivar.count(),
                  jvar.dofIndicesNeighbor().size() * jcount);

      jacobianBlockNeighbor(Moose::NeighborElement, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndicesNeighbor().size() * ivar.count(),
                  jvar.dofIndices().size() * jcount);

      jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndicesNeighbor().size() * ivar.count(),
                  jvar.dofIndicesNeighbor().size() * jcount);

      jacobianBlockNeighborUsed(tag, vi, vj, false);
    }
  }

  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (auto & tag_Rn : _sub_Rn)
      tag_Rn[var->number()].resize(var->dofIndicesNeighbor().size() * var->count());
}

void
Assembly::prepareLowerD()
{
  for (const auto & it : _cm_ff_entry)
  {
    MooseVariableFEBase & ivar = *(it.first);
    MooseVariableFEBase & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    for (MooseIndex(_jacobian_block_lower_used) tag = 0; tag < _jacobian_block_lower_used.size();
         tag++)
    {
      // To cover all possible cases we should have 9 combinations below for every 2-permutation of
      // Lower,Secondary,Primary. However, 4 cases will in general be covered by calls to prepare()
      // and prepareNeighbor(). These calls will cover SecondarySecondary (ElementElement),
      // SecondaryPrimary (ElementNeighbor), PrimarySecondary (NeighborElement), and PrimaryPrimary
      // (NeighborNeighbor). With these covered we only need to prepare the 5 remaining below

      // derivatives w.r.t. lower dimensional residuals
      jacobianBlockMortar(Moose::LowerLower, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndicesLower().size() * ivar.count(),
                  jvar.dofIndicesLower().size() * jcount);

      jacobianBlockMortar(Moose::LowerSecondary, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndicesLower().size() * ivar.count(),
                  jvar.dofIndices().size() * jvar.count());

      jacobianBlockMortar(Moose::LowerPrimary, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndicesLower().size() * ivar.count(),
                  jvar.dofIndicesNeighbor().size() * jvar.count());

      // derivatives w.r.t. interior secondary residuals
      jacobianBlockMortar(Moose::SecondaryLower, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndices().size() * ivar.count(),
                  jvar.dofIndicesLower().size() * jvar.count());

      // derivatives w.r.t. interior primary residuals
      jacobianBlockMortar(Moose::PrimaryLower, vi, vj, LocalDataKey{}, tag)
          .resize(ivar.dofIndicesNeighbor().size() * ivar.count(),
                  jvar.dofIndicesLower().size() * jvar.count());

      jacobianBlockLowerUsed(tag, vi, vj, false);
    }
  }

  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (auto & tag_Rl : _sub_Rl)
      tag_Rl[var->number()].resize(var->dofIndicesLower().size() * var->count());
}

void
Assembly::prepareBlock(unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices)
{
  const auto & iv = _sys.getVariable(_tid, ivar);
  const auto & jv = _sys.getVariable(_tid, jvar);
  const unsigned int ivn = iv.number();
  const unsigned int jvn = jv.number();
  const unsigned int icount = iv.count();
  unsigned int jcount = jv.count();
  if (ivn == jvn && _component_block_diagonal[ivn])
    jcount = 1;

  for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
  {
    jacobianBlock(ivn, jvn, LocalDataKey{}, tag)
        .resize(dof_indices.size() * icount, dof_indices.size() * jcount);
    jacobianBlockUsed(tag, ivn, jvn, false);
  }

  for (auto & tag_Re : _sub_Re)
    tag_Re[ivn].resize(dof_indices.size() * icount);
}

void
Assembly::prepareBlockNonlocal(unsigned int ivar,
                               unsigned int jvar,
                               const std::vector<dof_id_type> & idof_indices,
                               const std::vector<dof_id_type> & jdof_indices)
{
  const auto & iv = _sys.getVariable(_tid, ivar);
  const auto & jv = _sys.getVariable(_tid, jvar);
  const unsigned int ivn = iv.number();
  const unsigned int jvn = jv.number();
  const unsigned int icount = iv.count();
  unsigned int jcount = jv.count();
  if (ivn == jvn && _component_block_diagonal[ivn])
    jcount = 1;

  for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
       tag < _jacobian_block_nonlocal_used.size();
       tag++)
  {
    jacobianBlockNonlocal(ivn, jvn, LocalDataKey{}, tag)
        .resize(idof_indices.size() * icount, jdof_indices.size() * jcount);

    jacobianBlockNonlocalUsed(tag, ivn, jvn, false);
  }
}

void
Assembly::prepareScalar()
{
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (const auto & ivar : vars)
  {
    auto idofs = ivar->dofIndices().size();

    for (auto & tag_Re : _sub_Re)
      tag_Re[ivar->number()].resize(idofs);

    for (const auto & jvar : vars)
    {
      auto jdofs = jvar->dofIndices().size();

      for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(ivar->number(), jvar->number(), LocalDataKey{}, tag).resize(idofs, jdofs);
        jacobianBlockUsed(tag, ivar->number(), jvar->number(), false);
      }
    }
  }
}

void
Assembly::prepareOffDiagScalar()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);

  for (const auto & ivar : scalar_vars)
  {
    auto idofs = ivar->dofIndices().size();

    for (const auto & jvar : vars)
    {
      auto jdofs = jvar->dofIndices().size() * jvar->count();
      for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(ivar->number(), jvar->number(), LocalDataKey{}, tag).resize(idofs, jdofs);
        jacobianBlockUsed(tag, ivar->number(), jvar->number(), false);

        jacobianBlock(jvar->number(), ivar->number(), LocalDataKey{}, tag).resize(jdofs, idofs);
        jacobianBlockUsed(tag, jvar->number(), ivar->number(), false);
      }
    }
  }
}

template <typename T>
void
Assembly::copyShapes(MooseVariableField<T> & v)
{
  phi(v).shallowCopy(v.phi());
  gradPhi(v).shallowCopy(v.gradPhi());
  if (v.computingSecond())
    secondPhi(v).shallowCopy(v.secondPhi());
}

void
Assembly::copyShapes(unsigned int var)
{
  auto & v = _sys.getVariable(_tid, var);
  if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    auto & v = _sys.getActualFieldVariable<Real>(_tid, var);
    copyShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    auto & v = _sys.getActualFieldVariable<RealEigenVector>(_tid, var);
    copyShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_VECTOR)
  {
    auto & v = _sys.getActualFieldVariable<RealVectorValue>(_tid, var);
    copyShapes(v);
    if (v.computingCurl())
      curlPhi(v).shallowCopy(v.curlPhi());
  }
  else
    mooseError("Unsupported variable field type!");
}

template <typename T>
void
Assembly::copyFaceShapes(MooseVariableField<T> & v)
{
  phiFace(v).shallowCopy(v.phiFace());
  gradPhiFace(v).shallowCopy(v.gradPhiFace());
  if (v.computingSecond())
    secondPhiFace(v).shallowCopy(v.secondPhiFace());
}

void
Assembly::copyFaceShapes(unsigned int var)
{
  auto & v = _sys.getVariable(_tid, var);
  if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    auto & v = _sys.getActualFieldVariable<Real>(_tid, var);
    copyFaceShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    auto & v = _sys.getActualFieldVariable<RealEigenVector>(_tid, var);
    copyFaceShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_VECTOR)
  {
    auto & v = _sys.getActualFieldVariable<RealVectorValue>(_tid, var);
    copyFaceShapes(v);
    if (v.computingCurl())
      _vector_curl_phi_face.shallowCopy(v.curlPhi());
  }
  else
    mooseError("Unsupported variable field type!");
}

template <typename T>
void
Assembly::copyNeighborShapes(MooseVariableField<T> & v)
{
  if (v.usesPhiNeighbor())
  {
    phiFaceNeighbor(v).shallowCopy(v.phiFaceNeighbor());
    phiNeighbor(v).shallowCopy(v.phiNeighbor());
  }
  if (v.usesGradPhiNeighbor())
  {
    gradPhiFaceNeighbor(v).shallowCopy(v.gradPhiFaceNeighbor());
    gradPhiNeighbor(v).shallowCopy(v.gradPhiNeighbor());
  }
  if (v.usesSecondPhiNeighbor())
  {
    secondPhiFaceNeighbor(v).shallowCopy(v.secondPhiFaceNeighbor());
    secondPhiNeighbor(v).shallowCopy(v.secondPhiNeighbor());
  }
}

void
Assembly::copyNeighborShapes(unsigned int var)
{
  auto & v = _sys.getVariable(_tid, var);
  if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    auto & v = _sys.getActualFieldVariable<Real>(_tid, var);
    copyNeighborShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    auto & v = _sys.getActualFieldVariable<RealEigenVector>(_tid, var);
    copyNeighborShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_VECTOR)
  {
    auto & v = _sys.getActualFieldVariable<RealVectorValue>(_tid, var);
    copyNeighborShapes(v);
  }
  else
    mooseError("Unsupported variable field type!");
}

DenseMatrix<Number> &
Assembly::jacobianBlockNeighbor(
    Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar, LocalDataKey, TagID tag)
{
  if (type == Moose::ElementElement)
    jacobianBlockUsed(tag, ivar, jvar, true);
  else
    jacobianBlockNeighborUsed(tag, ivar, jvar, true);

  if (_block_diagonal_matrix)
  {
    switch (type)
    {
      default:
      case Moose::ElementElement:
        return _sub_Kee[tag][ivar][0];
      case Moose::ElementNeighbor:
        return _sub_Ken[tag][ivar][0];
      case Moose::NeighborElement:
        return _sub_Kne[tag][ivar][0];
      case Moose::NeighborNeighbor:
        return _sub_Knn[tag][ivar][0];
    }
  }
  else
  {
    switch (type)
    {
      default:
      case Moose::ElementElement:
        return _sub_Kee[tag][ivar][jvar];
      case Moose::ElementNeighbor:
        return _sub_Ken[tag][ivar][jvar];
      case Moose::NeighborElement:
        return _sub_Kne[tag][ivar][jvar];
      case Moose::NeighborNeighbor:
        return _sub_Knn[tag][ivar][jvar];
    }
  }
}

DenseMatrix<Number> &
Assembly::jacobianBlockMortar(Moose::ConstraintJacobianType type,
                              unsigned int ivar,
                              unsigned int jvar,
                              LocalDataKey,
                              TagID tag)
{
  jacobianBlockLowerUsed(tag, ivar, jvar, true);
  if (_block_diagonal_matrix)
  {
    switch (type)
    {
      default:
      case Moose::LowerLower:
        return _sub_Kll[tag][ivar][0];
      case Moose::LowerSecondary:
        return _sub_Kle[tag][ivar][0];
      case Moose::LowerPrimary:
        return _sub_Kln[tag][ivar][0];
      case Moose::SecondaryLower:
        return _sub_Kel[tag][ivar][0];
      case Moose::SecondarySecondary:
        return _sub_Kee[tag][ivar][0];
      case Moose::SecondaryPrimary:
        return _sub_Ken[tag][ivar][0];
      case Moose::PrimaryLower:
        return _sub_Knl[tag][ivar][0];
      case Moose::PrimarySecondary:
        return _sub_Kne[tag][ivar][0];
      case Moose::PrimaryPrimary:
        return _sub_Knn[tag][ivar][0];
    }
  }
  else
  {
    switch (type)
    {
      default:
      case Moose::LowerLower:
        return _sub_Kll[tag][ivar][jvar];
      case Moose::LowerSecondary:
        return _sub_Kle[tag][ivar][jvar];
      case Moose::LowerPrimary:
        return _sub_Kln[tag][ivar][jvar];
      case Moose::SecondaryLower:
        return _sub_Kel[tag][ivar][jvar];
      case Moose::SecondarySecondary:
        return _sub_Kee[tag][ivar][jvar];
      case Moose::SecondaryPrimary:
        return _sub_Ken[tag][ivar][jvar];
      case Moose::PrimaryLower:
        return _sub_Knl[tag][ivar][jvar];
      case Moose::PrimarySecondary:
        return _sub_Kne[tag][ivar][jvar];
      case Moose::PrimaryPrimary:
        return _sub_Knn[tag][ivar][jvar];
    }
  }
}

void
Assembly::processLocalResidual(DenseVector<Number> & res_block,
                               std::vector<dof_id_type> & dof_indices,
                               const std::vector<Real> & scaling_factor,
                               bool is_nodal)
{
  // For an array variable, ndof is the number of dofs of the zero-th component and
  // ntdof is the number of dofs of all components.
  // For standard or vector variables, ndof will be the same as ntdof.
  auto ndof = dof_indices.size();
  auto ntdof = res_block.size();
  auto count = ntdof / ndof;
  mooseAssert(count == scaling_factor.size(), "Inconsistent of number of components");
  mooseAssert(count * ndof == ntdof, "Inconsistent of number of components");
  if (count > 1)
  {
    // expanding dof indices
    dof_indices.resize(ntdof);
    unsigned int p = 0;
    for (MooseIndex(count) j = 0; j < count; ++j)
      for (MooseIndex(ndof) i = 0; i < ndof; ++i)
      {
        dof_indices[p] = dof_indices[i] + (is_nodal ? j : j * ndof);
        res_block(p) *= scaling_factor[j];
        ++p;
      }
  }
  else
  {
    if (scaling_factor[0] != 1.0)
      res_block *= scaling_factor[0];
  }

  _dof_map.constrain_element_vector(res_block, dof_indices, false);
}

void
Assembly::addResidualBlock(NumericVector<Number> & residual,
                           DenseVector<Number> & res_block,
                           const std::vector<dof_id_type> & dof_indices,
                           const std::vector<Real> & scaling_factor,
                           bool is_nodal)
{
  if (dof_indices.size() > 0 && res_block.size())
  {
    _temp_dof_indices = dof_indices;
    _tmp_Re = res_block;
    processLocalResidual(_tmp_Re, _temp_dof_indices, scaling_factor, is_nodal);
    residual.add_vector(_tmp_Re, _temp_dof_indices);
  }
}

void
Assembly::cacheResidualBlock(std::vector<Real> & cached_residual_values,
                             std::vector<dof_id_type> & cached_residual_rows,
                             DenseVector<Number> & res_block,
                             const std::vector<dof_id_type> & dof_indices,
                             const std::vector<Real> & scaling_factor,
                             bool is_nodal)
{
  if (dof_indices.size() > 0 && res_block.size())
  {
    _temp_dof_indices = dof_indices;
    _tmp_Re = res_block;
    processLocalResidual(_tmp_Re, _temp_dof_indices, scaling_factor, is_nodal);

    for (MooseIndex(_tmp_Re) i = 0; i < _tmp_Re.size(); i++)
    {
      cached_residual_values.push_back(_tmp_Re(i));
      cached_residual_rows.push_back(_temp_dof_indices[i]);
    }
  }

  res_block.zero();
}

void
Assembly::setResidualBlock(NumericVector<Number> & residual,
                           DenseVector<Number> & res_block,
                           const std::vector<dof_id_type> & dof_indices,
                           const std::vector<Real> & scaling_factor,
                           bool is_nodal)
{
  if (dof_indices.size() > 0)
  {
    std::vector<dof_id_type> di(dof_indices);
    _tmp_Re = res_block;
    processLocalResidual(_tmp_Re, di, scaling_factor, is_nodal);
    residual.insert(_tmp_Re, di);
  }
}

void
Assembly::addResidual(const VectorTag & vector_tag)
{
  mooseAssert(vector_tag._type == Moose::VECTOR_TAG_RESIDUAL,
              "Non-residual tag in Assembly::addResidual");

  auto & tag_Re = _sub_Re[vector_tag._type_id];
  NumericVector<Number> & residual = _sys.getVector(vector_tag._id);
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual,
                     tag_Re[var->number()],
                     var->dofIndices(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

void
Assembly::addResidual(GlobalDataKey, const std::vector<VectorTag> & vector_tags)
{
  for (const auto & vector_tag : vector_tags)
    if (_sys.hasVector(vector_tag._id))
      addResidual(vector_tag);
}

void
Assembly::addResidualNeighbor(const VectorTag & vector_tag)
{
  mooseAssert(vector_tag._type == Moose::VECTOR_TAG_RESIDUAL,
              "Non-residual tag in Assembly::addResidualNeighbor");

  auto & tag_Rn = _sub_Rn[vector_tag._type_id];
  NumericVector<Number> & residual = _sys.getVector(vector_tag._id);
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual,
                     tag_Rn[var->number()],
                     var->dofIndicesNeighbor(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

void
Assembly::addResidualNeighbor(GlobalDataKey, const std::vector<VectorTag> & vector_tags)
{
  for (const auto & vector_tag : vector_tags)
    if (_sys.hasVector(vector_tag._id))
      addResidualNeighbor(vector_tag);
}

void
Assembly::addResidualLower(const VectorTag & vector_tag)
{
  mooseAssert(vector_tag._type == Moose::VECTOR_TAG_RESIDUAL,
              "Non-residual tag in Assembly::addResidualLower");

  auto & tag_Rl = _sub_Rl[vector_tag._type_id];
  NumericVector<Number> & residual = _sys.getVector(vector_tag._id);
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual,
                     tag_Rl[var->number()],
                     var->dofIndicesLower(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

void
Assembly::addResidualLower(GlobalDataKey, const std::vector<VectorTag> & vector_tags)
{
  for (const auto & vector_tag : vector_tags)
    if (_sys.hasVector(vector_tag._id))
      addResidualLower(vector_tag);
}

// private method, so no key required
void
Assembly::addResidualScalar(const VectorTag & vector_tag)
{
  mooseAssert(vector_tag._type == Moose::VECTOR_TAG_RESIDUAL,
              "Non-residual tag in Assembly::addResidualScalar");

  // add the scalar variables residuals
  auto & tag_Re = _sub_Re[vector_tag._type_id];
  NumericVector<Number> & residual = _sys.getVector(vector_tag._id);
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(
        residual, tag_Re[var->number()], var->dofIndices(), var->arrayScalingFactor(), false);
}

void
Assembly::addResidualScalar(GlobalDataKey, const std::vector<VectorTag> & vector_tags)
{
  for (const auto & vector_tag : vector_tags)
    if (_sys.hasVector(vector_tag._id))
      addResidualScalar(vector_tag);
}

void
Assembly::cacheResidual(GlobalDataKey, const std::vector<VectorTag> & tags)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (const auto & vector_tag : tags)
      if (_sys.hasVector(vector_tag._id))
        cacheResidualBlock(_cached_residual_values[vector_tag._type_id],
                           _cached_residual_rows[vector_tag._type_id],
                           _sub_Re[vector_tag._type_id][var->number()],
                           var->dofIndices(),
                           var->arrayScalingFactor(),
                           var->isNodal());
}

// private method, so no key required
void
Assembly::cacheResidual(dof_id_type dof, Real value, TagID tag_id)
{
  const VectorTag & tag = _subproblem.getVectorTag(tag_id);

  _cached_residual_values[tag._type_id].push_back(value);
  _cached_residual_rows[tag._type_id].push_back(dof);
}

// private method, so no key required
void
Assembly::cacheResidual(dof_id_type dof, Real value, const std::set<TagID> & tags)
{
  for (auto & tag : tags)
    cacheResidual(dof, value, tag);
}

void
Assembly::cacheResidualNodes(const DenseVector<Number> & res,
                             const std::vector<dof_id_type> & dof_index,
                             LocalDataKey,
                             TagID tag)
{
  // Add the residual value and dof_index to cached_residual_values and cached_residual_rows
  // respectively.
  // This is used by NodalConstraint.C to cache the residual calculated for primary and secondary
  // node.
  const VectorTag & vector_tag = _subproblem.getVectorTag(tag);
  for (MooseIndex(dof_index) i = 0; i < dof_index.size(); ++i)
  {
    _cached_residual_values[vector_tag._type_id].push_back(res(i));
    _cached_residual_rows[vector_tag._type_id].push_back(dof_index[i]);
  }
}

void
Assembly::cacheResidualNeighbor(GlobalDataKey, const std::vector<VectorTag> & tags)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (const auto & vector_tag : tags)
      if (_sys.hasVector(vector_tag._id))
        cacheResidualBlock(_cached_residual_values[vector_tag._type_id],
                           _cached_residual_rows[vector_tag._type_id],
                           _sub_Rn[vector_tag._type_id][var->number()],
                           var->dofIndicesNeighbor(),
                           var->arrayScalingFactor(),
                           var->isNodal());
}

void
Assembly::cacheResidualLower(GlobalDataKey, const std::vector<VectorTag> & tags)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (const auto & vector_tag : tags)
      if (_sys.hasVector(vector_tag._id))
        cacheResidualBlock(_cached_residual_values[vector_tag._type_id],
                           _cached_residual_rows[vector_tag._type_id],
                           _sub_Rl[vector_tag._type_id][var->number()],
                           var->dofIndicesLower(),
                           var->arrayScalingFactor(),
                           var->isNodal());
}

void
Assembly::addCachedResiduals(GlobalDataKey, const std::vector<VectorTag> & tags)
{
  for (const auto & vector_tag : tags)
  {
    if (!_sys.hasVector(vector_tag._id))
    {
      _cached_residual_values[vector_tag._type_id].clear();
      _cached_residual_rows[vector_tag._type_id].clear();
      continue;
    }
    addCachedResidualDirectly(_sys.getVector(vector_tag._id), GlobalDataKey{}, vector_tag);
  }
}

void Assembly::clearCachedResiduals(GlobalDataKey)
{
  for (const auto & vector_tag : _residual_vector_tags)
    clearCachedResiduals(vector_tag);
}

// private method, so no key required
void
Assembly::clearCachedResiduals(const VectorTag & vector_tag)
{
  auto & values = _cached_residual_values[vector_tag._type_id];
  auto & rows = _cached_residual_rows[vector_tag._type_id];

  mooseAssert(values.size() == rows.size(),
              "Number of cached residuals and number of rows must match!");

  // Keep track of the largest size so we can use it to reserve and avoid
  // as much dynamic allocation as possible
  if (_max_cached_residuals < values.size())
    _max_cached_residuals = values.size();

  // Clear both vectors (keeps the capacity the same)
  values.clear();
  rows.clear();
  // And then reserve: use 2 as a fudge factor to *really* avoid dynamic allocation!
  values.reserve(_max_cached_residuals * 2);
  rows.reserve(_max_cached_residuals * 2);
}

void
Assembly::addCachedResidualDirectly(NumericVector<Number> & residual,
                                    GlobalDataKey,
                                    const VectorTag & vector_tag)
{
  const auto & values = _cached_residual_values[vector_tag._type_id];
  const auto & rows = _cached_residual_rows[vector_tag._type_id];

  mooseAssert(values.size() == rows.size(),
              "Number of cached residuals and number of rows must match!");

  if (!values.empty())
  {
    residual.add_vector(values, rows);
    clearCachedResiduals(vector_tag);
  }
}

void
Assembly::setResidual(NumericVector<Number> & residual, GlobalDataKey, const VectorTag & vector_tag)
{
  auto & tag_Re = _sub_Re[vector_tag._type_id];
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(residual,
                     tag_Re[var->number()],
                     var->dofIndices(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

void
Assembly::setResidualNeighbor(NumericVector<Number> & residual,
                              GlobalDataKey,
                              const VectorTag & vector_tag)
{
  auto & tag_Rn = _sub_Rn[vector_tag._type_id];
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(residual,
                     tag_Rn[var->number()],
                     var->dofIndicesNeighbor(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

// private method, so no key required
void
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian,
                           DenseMatrix<Number> & jac_block,
                           const MooseVariableBase & ivar,
                           const MooseVariableBase & jvar,
                           const std::vector<dof_id_type> & idof_indices,
                           const std::vector<dof_id_type> & jdof_indices)
{
  if (idof_indices.size() == 0 || jdof_indices.size() == 0)
    return;
  if (jac_block.n() == 0 || jac_block.m() == 0)
    return;

  auto & scaling_factor = ivar.arrayScalingFactor();

  for (unsigned int i = 0; i < ivar.count(); ++i)
  {
    unsigned int iv = ivar.number();
    for (const auto & jt : ConstCouplingRow(iv + i, *_cm))
    {
      unsigned int jv = jvar.number();
      if (jt < jv || jt >= jv + jvar.count())
        continue;
      unsigned int j = jt - jv;

      auto di = ivar.componentDofIndices(idof_indices, i);
      auto dj = jvar.componentDofIndices(jdof_indices, j);
      auto indof = di.size();
      auto jndof = dj.size();

      unsigned int jj = j;
      if (iv == jv && _component_block_diagonal[iv])
        // here i must be equal to j
        jj = 0;

      auto sub = jac_block.sub_matrix(i * indof, indof, jj * jndof, jndof);
      if (scaling_factor[i] != 1.0)
        sub *= scaling_factor[i];

      // If we're computing the jacobian for automatically scaling variables we do not want
      // to constrain the element matrix because it introduces 1s on the diagonal for the
      // constrained dofs
      if (!_sys.computingScalingJacobian())
        _dof_map.constrain_element_matrix(sub, di, dj, false);

      jacobian.add_matrix(sub, di, dj);
    }
  }
}

// private method, so no key required
void
Assembly::cacheJacobianBlock(DenseMatrix<Number> & jac_block,
                             const MooseVariableBase & ivar,
                             const MooseVariableBase & jvar,
                             const std::vector<dof_id_type> & idof_indices,
                             const std::vector<dof_id_type> & jdof_indices,
                             TagID tag)
{
  if (idof_indices.size() == 0 || jdof_indices.size() == 0)
    return;
  if (jac_block.n() == 0 || jac_block.m() == 0)
    return;
  if (!_sys.hasMatrix(tag))
    return;

  auto & scaling_factor = ivar.arrayScalingFactor();

  for (unsigned int i = 0; i < ivar.count(); ++i)
  {
    unsigned int iv = ivar.number();
    for (const auto & jt : ConstCouplingRow(iv + i, *_cm))
    {
      unsigned int jv = jvar.number();
      if (jt < jv || jt >= jv + jvar.count())
        continue;
      unsigned int j = jt - jv;

      auto di = ivar.componentDofIndices(idof_indices, i);
      auto dj = jvar.componentDofIndices(jdof_indices, j);
      auto indof = di.size();
      auto jndof = dj.size();

      unsigned int jj = j;
      if (iv == jv && _component_block_diagonal[iv])
        // here i must be equal to j
        jj = 0;

      auto sub = jac_block.sub_matrix(i * indof, indof, jj * jndof, jndof);
      if (scaling_factor[i] != 1.0)
        sub *= scaling_factor[i];

      // If we're computing the jacobian for automatically scaling variables we do not want
      // to constrain the element matrix because it introduces 1s on the diagonal for the
      // constrained dofs
      if (!_sys.computingScalingJacobian())
        _dof_map.constrain_element_matrix(sub, di, dj, false);

      for (MooseIndex(di) i = 0; i < di.size(); i++)
        for (MooseIndex(dj) j = 0; j < dj.size(); j++)
        {
          _cached_jacobian_values[tag].push_back(sub(i, j));
          _cached_jacobian_rows[tag].push_back(di[i]);
          _cached_jacobian_cols[tag].push_back(dj[j]);
        }
    }
  }

  jac_block.zero();
}

// private method, so no key required
void
Assembly::cacheJacobianBlockNonzero(DenseMatrix<Number> & jac_block,
                                    const MooseVariableBase & ivar,
                                    const MooseVariableBase & jvar,
                                    const std::vector<dof_id_type> & idof_indices,
                                    const std::vector<dof_id_type> & jdof_indices,
                                    TagID tag)
{
  if (idof_indices.size() == 0 || jdof_indices.size() == 0)
    return;
  if (jac_block.n() == 0 || jac_block.m() == 0)
    return;
  if (!_sys.hasMatrix(tag))
    return;

  auto & scaling_factor = ivar.arrayScalingFactor();

  for (unsigned int i = 0; i < ivar.count(); ++i)
  {
    unsigned int iv = ivar.number();
    for (const auto & jt : ConstCouplingRow(iv + i, *_cm))
    {
      unsigned int jv = jvar.number();
      if (jt < jv || jt >= jv + jvar.count())
        continue;
      unsigned int j = jt - jv;

      auto di = ivar.componentDofIndices(idof_indices, i);
      auto dj = jvar.componentDofIndices(jdof_indices, j);
      auto indof = di.size();
      auto jndof = dj.size();

      unsigned int jj = j;
      if (iv == jv && _component_block_diagonal[iv])
        // here i must be equal to j
        jj = 0;

      auto sub = jac_block.sub_matrix(i * indof, indof, jj * jndof, jndof);
      if (scaling_factor[i] != 1.0)
        sub *= scaling_factor[i];

      _dof_map.constrain_element_matrix(sub, di, dj, false);

      for (MooseIndex(di) i = 0; i < di.size(); i++)
        for (MooseIndex(dj) j = 0; j < dj.size(); j++)
          if (sub(i, j) != 0.0) // no storage allocated for unimplemented jacobian terms,
                                // maintaining maximum sparsity possible
          {
            _cached_jacobian_values[tag].push_back(sub(i, j));
            _cached_jacobian_rows[tag].push_back(di[i]);
            _cached_jacobian_cols[tag].push_back(dj[j]);
          }
    }
  }

  jac_block.zero();
}

void
Assembly::cacheJacobianBlock(DenseMatrix<Number> & jac_block,
                             const std::vector<dof_id_type> & idof_indices,
                             const std::vector<dof_id_type> & jdof_indices,
                             Real scaling_factor,
                             LocalDataKey,
                             TagID tag)
{
  // Only cache data when the matrix exists
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0) && jac_block.n() && jac_block.m() &&
      _sys.hasMatrix(tag))
  {
    std::vector<dof_id_type> di(idof_indices);
    std::vector<dof_id_type> dj(jdof_indices);

    // If we're computing the jacobian for automatically scaling variables we do not want to
    // constrain the element matrix because it introduces 1s on the diagonal for the constrained
    // dofs
    if (!_sys.computingScalingJacobian())
      _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
      jac_block *= scaling_factor;

    for (MooseIndex(di) i = 0; i < di.size(); i++)
      for (MooseIndex(dj) j = 0; j < dj.size(); j++)
      {
        _cached_jacobian_values[tag].push_back(jac_block(i, j));
        _cached_jacobian_rows[tag].push_back(di[i]);
        _cached_jacobian_cols[tag].push_back(dj[j]);
      }
  }
  jac_block.zero();
}

Real
Assembly::elementVolume(const Elem * elem) const
{
  FEType fe_type(elem->default_order(), LAGRANGE);
  std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));

  // references to the quadrature points and weights
  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<Point> & q_points = fe->get_xyz();

  // The default quadrature rule should integrate the mass matrix,
  // thus it should be plenty to compute the volume
  QGauss qrule(elem->dim(), fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  fe->reinit(elem);

  // perform a sanity check to ensure that size of quad rule and size of q_points is
  // identical
  mooseAssert(qrule.n_points() == q_points.size(),
              "The number of points in the quadrature rule doesn't match the number of passed-in "
              "points in Assembly::setCoordinateTransformation");

  // compute the coordinate transformation
  Real vol = 0;
  for (unsigned int qp = 0; qp < qrule.n_points(); ++qp)
  {
    Real coord;
    coordTransformFactor(_subproblem, elem->subdomain_id(), q_points[qp], coord);
    vol += JxW[qp] * coord;
  }
  return vol;
}

void Assembly::addCachedJacobian(GlobalDataKey)
{
  if (!_subproblem.checkNonlocalCouplingRequirement())
  {
    mooseAssert(_cached_jacobian_rows.size() == _cached_jacobian_cols.size(),
                "Error: Cached data sizes MUST be the same!");
    for (MooseIndex(_cached_jacobian_rows) i = 0; i < _cached_jacobian_rows.size(); i++)
      mooseAssert(_cached_jacobian_rows[i].size() == _cached_jacobian_cols[i].size(),
                  "Error: Cached data sizes MUST be the same for a given tag!");
  }

  for (MooseIndex(_cached_jacobian_rows) i = 0; i < _cached_jacobian_rows.size(); i++)
    if (_sys.hasMatrix(i))
      for (MooseIndex(_cached_jacobian_rows[i]) j = 0; j < _cached_jacobian_rows[i].size(); j++)
        _sys.getMatrix(i).add(_cached_jacobian_rows[i][j],
                              _cached_jacobian_cols[i][j],
                              _cached_jacobian_values[i][j]);

  for (MooseIndex(_cached_jacobian_rows) i = 0; i < _cached_jacobian_rows.size(); i++)
  {
    if (!_sys.hasMatrix(i))
      continue;

    if (_max_cached_jacobians < _cached_jacobian_values[i].size())
      _max_cached_jacobians = _cached_jacobian_values[i].size();

    // Try to be more efficient from now on
    // The 2 is just a fudge factor to keep us from having to grow the vector during assembly
    _cached_jacobian_values[i].clear();
    _cached_jacobian_values[i].reserve(_max_cached_jacobians * 2);

    _cached_jacobian_rows[i].clear();
    _cached_jacobian_rows[i].reserve(_max_cached_jacobians * 2);

    _cached_jacobian_cols[i].clear();
    _cached_jacobian_cols[i].reserve(_max_cached_jacobians * 2);
  }
}

inline void
Assembly::addJacobianCoupledVarPair(const MooseVariableBase & ivar, const MooseVariableBase & jvar)
{
  auto i = ivar.number();
  auto j = jvar.number();
  for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
    if (jacobianBlockUsed(tag, i, j) && _sys.hasMatrix(tag))
      addJacobianBlock(_sys.getMatrix(tag),
                       jacobianBlock(i, j, LocalDataKey{}, tag),
                       ivar,
                       jvar,
                       ivar.dofIndices(),
                       jvar.dofIndices());
}

void Assembly::addJacobian(GlobalDataKey)
{
  for (const auto & it : _cm_ff_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_sf_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_fs_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);
}

void Assembly::addJacobianNonlocal(GlobalDataKey)
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
      if (jacobianBlockNonlocalUsed(tag, i, j) && _sys.hasMatrix(tag))
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNonlocal(i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->allDofIndices());
  }
}

void Assembly::addJacobianNeighbor(GlobalDataKey)
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (MooseIndex(_jacobian_block_neighbor_used) tag = 0;
         tag < _jacobian_block_neighbor_used.size();
         tag++)
      if (jacobianBlockNeighborUsed(tag, i, j) && _sys.hasMatrix(tag))
      {
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::ElementNeighbor, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->dofIndicesNeighbor());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborElement, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndices());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborNeighbor, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndicesNeighbor());
      }
  }
}

void Assembly::addJacobianNeighborLowerD(GlobalDataKey)
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (MooseIndex(_jacobian_block_lower_used) tag = 0; tag < _jacobian_block_lower_used.size();
         tag++)
      if (jacobianBlockLowerUsed(tag, i, j) && _sys.hasMatrix(tag))
      {
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::LowerLower, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndicesLower());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::LowerSecondary, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndicesNeighbor());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::LowerPrimary, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndices());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::SecondaryLower, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndicesLower());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::PrimaryLower, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->dofIndicesLower());
      }

    for (MooseIndex(_jacobian_block_neighbor_used) tag = 0;
         tag < _jacobian_block_neighbor_used.size();
         tag++)
      if (jacobianBlockNeighborUsed(tag, i, j) && _sys.hasMatrix(tag))
      {
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::ElementNeighbor, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->dofIndicesNeighbor());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborElement, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndices());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborNeighbor, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndicesNeighbor());
      }
  }
}

void Assembly::addJacobianLowerD(GlobalDataKey)
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (MooseIndex(_jacobian_block_lower_used) tag = 0; tag < _jacobian_block_lower_used.size();
         tag++)
      if (jacobianBlockLowerUsed(tag, i, j) && _sys.hasMatrix(tag))
      {
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::LowerLower, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndicesLower());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::LowerSecondary, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndices());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockMortar(Moose::SecondaryLower, i, j, LocalDataKey{}, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->dofIndicesLower());
      }
  }
}

void Assembly::cacheJacobian(GlobalDataKey)
{
  for (const auto & it : _cm_ff_entry)
    cacheJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_fs_entry)
    cacheJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_sf_entry)
    cacheJacobianCoupledVarPair(*it.first, *it.second);
}

// private method, so no key required
void
Assembly::cacheJacobianCoupledVarPair(const MooseVariableBase & ivar,
                                      const MooseVariableBase & jvar)
{
  auto i = ivar.number();
  auto j = jvar.number();
  for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
    if (jacobianBlockUsed(tag, i, j) && _sys.hasMatrix(tag))
      cacheJacobianBlock(jacobianBlock(i, j, LocalDataKey{}, tag),
                         ivar,
                         jvar,
                         ivar.dofIndices(),
                         jvar.dofIndices(),
                         tag);
}

void Assembly::cacheJacobianNonlocal(GlobalDataKey)
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
      if (jacobianBlockNonlocalUsed(tag, i, j) && _sys.hasMatrix(tag))
        cacheJacobianBlockNonzero(jacobianBlockNonlocal(i, j, LocalDataKey{}, tag),
                                  *ivar,
                                  *jvar,
                                  ivar->dofIndices(),
                                  jvar->allDofIndices(),
                                  tag);
  }
}

void Assembly::cacheJacobianNeighbor(GlobalDataKey)
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();

    for (MooseIndex(_jacobian_block_neighbor_used) tag = 0;
         tag < _jacobian_block_neighbor_used.size();
         tag++)
      if (jacobianBlockNeighborUsed(tag, i, j) && _sys.hasMatrix(tag))
      {
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::ElementNeighbor, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndices(),
                           jvar->dofIndicesNeighbor(),
                           tag);
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborElement, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndices(),
                           tag);
        cacheJacobianBlock(
            jacobianBlockNeighbor(Moose::NeighborNeighbor, i, j, LocalDataKey{}, tag),
            *ivar,
            *jvar,
            ivar->dofIndicesNeighbor(),
            jvar->dofIndicesNeighbor(),
            tag);
      }
  }
}

void Assembly::cacheJacobianMortar(GlobalDataKey)
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (MooseIndex(_jacobian_block_lower_used) tag = 0; tag < _jacobian_block_lower_used.size();
         tag++)
      if (jacobianBlockLowerUsed(tag, i, j) && _sys.hasMatrix(tag))
      {
        cacheJacobianBlock(jacobianBlockMortar(Moose::LowerLower, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesLower(),
                           jvar->dofIndicesLower(),
                           tag);

        cacheJacobianBlock(jacobianBlockMortar(Moose::LowerSecondary, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesLower(),
                           jvar->dofIndices(),
                           tag);

        cacheJacobianBlock(jacobianBlockMortar(Moose::LowerPrimary, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesLower(),
                           jvar->dofIndicesNeighbor(),
                           tag);

        cacheJacobianBlock(jacobianBlockMortar(Moose::SecondaryLower, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndices(),
                           jvar->dofIndicesLower(),
                           tag);

        cacheJacobianBlock(
            jacobianBlockMortar(Moose::SecondarySecondary, i, j, LocalDataKey{}, tag),
            *ivar,
            *jvar,
            ivar->dofIndices(),
            jvar->dofIndices(),
            tag);

        cacheJacobianBlock(jacobianBlockMortar(Moose::SecondaryPrimary, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndices(),
                           jvar->dofIndicesNeighbor(),
                           tag);

        cacheJacobianBlock(jacobianBlockMortar(Moose::PrimaryLower, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndicesLower(),
                           tag);

        cacheJacobianBlock(jacobianBlockMortar(Moose::PrimarySecondary, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndices(),
                           tag);

        cacheJacobianBlock(jacobianBlockMortar(Moose::PrimaryPrimary, i, j, LocalDataKey{}, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndicesNeighbor(),
                           tag);
      }
  }
}

void
Assembly::addJacobianBlockTags(SparseMatrix<Number> & jacobian,
                               unsigned int ivar,
                               unsigned int jvar,
                               const DofMap & dof_map,
                               std::vector<dof_id_type> & dof_indices,
                               GlobalDataKey,
                               const std::set<TagID> & tags)
{
  for (auto tag : tags)
    addJacobianBlock(jacobian, ivar, jvar, dof_map, dof_indices, GlobalDataKey{}, tag);
}

void
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian,
                           unsigned int ivar,
                           unsigned int jvar,
                           const DofMap & dof_map,
                           std::vector<dof_id_type> & dof_indices,
                           GlobalDataKey,
                           TagID tag)
{
  if (dof_indices.size() == 0)
    return;
  if (!(*_cm)(ivar, jvar))
    return;

  auto & iv = _sys.getVariable(_tid, ivar);
  auto & jv = _sys.getVariable(_tid, jvar);
  auto & scaling_factor = iv.arrayScalingFactor();

  const unsigned int ivn = iv.number();
  const unsigned int jvn = jv.number();
  auto & ke = jacobianBlock(ivn, jvn, LocalDataKey{}, tag);

  // It is guaranteed by design iv.number <= ivar since iv is obtained
  // through SystemBase::getVariable with ivar.
  // Most of times ivar will just be equal to iv.number except for array variables,
  // where ivar could be a number for a component of an array variable but calling
  // getVariable will return the array variable that has the number of the 0th component.
  // It is the same for jvar.
  const unsigned int i = ivar - ivn;
  const unsigned int j = jvar - jvn;

  // DoF indices are independently given
  auto di = dof_indices;
  auto dj = dof_indices;

  auto indof = di.size();
  auto jndof = dj.size();

  unsigned int jj = j;
  if (ivar == jvar && _component_block_diagonal[ivn])
    jj = 0;

  auto sub = ke.sub_matrix(i * indof, indof, jj * jndof, jndof);
  // If we're computing the jacobian for automatically scaling variables we do not want to
  // constrain the element matrix because it introduces 1s on the diagonal for the constrained
  // dofs
  if (!_sys.computingScalingJacobian())
    dof_map.constrain_element_matrix(sub, di, dj, false);

  if (scaling_factor[i] != 1.0)
    sub *= scaling_factor[i];

  jacobian.add_matrix(sub, di, dj);
}

void
Assembly::addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                   const unsigned int ivar,
                                   const unsigned int jvar,
                                   const DofMap & dof_map,
                                   const std::vector<dof_id_type> & idof_indices,
                                   const std::vector<dof_id_type> & jdof_indices,
                                   GlobalDataKey,
                                   const TagID tag)
{
  if (idof_indices.size() == 0 || jdof_indices.size() == 0)
    return;
  if (jacobian.n() == 0 || jacobian.m() == 0)
    return;
  if (!(*_cm)(ivar, jvar))
    return;

  auto & iv = _sys.getVariable(_tid, ivar);
  auto & jv = _sys.getVariable(_tid, jvar);
  auto & scaling_factor = iv.arrayScalingFactor();

  const unsigned int ivn = iv.number();
  const unsigned int jvn = jv.number();
  auto & keg = jacobianBlockNonlocal(ivn, jvn, LocalDataKey{}, tag);

  // It is guaranteed by design iv.number <= ivar since iv is obtained
  // through SystemBase::getVariable with ivar.
  // Most of times ivar will just be equal to iv.number except for array variables,
  // where ivar could be a number for a component of an array variable but calling
  // getVariable will return the array variable that has the number of the 0th component.
  // It is the same for jvar.
  const unsigned int i = ivar - ivn;
  const unsigned int j = jvar - jvn;

  // DoF indices are independently given
  auto di = idof_indices;
  auto dj = jdof_indices;

  auto indof = di.size();
  auto jndof = dj.size();

  unsigned int jj = j;
  if (ivar == jvar && _component_block_diagonal[ivn])
    jj = 0;

  auto sub = keg.sub_matrix(i * indof, indof, jj * jndof, jndof);
  // If we're computing the jacobian for automatically scaling variables we do not want to
  // constrain the element matrix because it introduces 1s on the diagonal for the constrained
  // dofs
  if (!_sys.computingScalingJacobian())
    dof_map.constrain_element_matrix(sub, di, dj, false);

  if (scaling_factor[i] != 1.0)
    sub *= scaling_factor[i];

  jacobian.add_matrix(sub, di, dj);
}

void
Assembly::addJacobianBlockNonlocalTags(SparseMatrix<Number> & jacobian,
                                       const unsigned int ivar,
                                       const unsigned int jvar,
                                       const DofMap & dof_map,
                                       const std::vector<dof_id_type> & idof_indices,
                                       const std::vector<dof_id_type> & jdof_indices,
                                       GlobalDataKey,
                                       const std::set<TagID> & tags)
{
  for (auto tag : tags)
    addJacobianBlockNonlocal(
        jacobian, ivar, jvar, dof_map, idof_indices, jdof_indices, GlobalDataKey{}, tag);
}

void
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                              const unsigned int ivar,
                              const unsigned int jvar,
                              const DofMap & dof_map,
                              std::vector<dof_id_type> & dof_indices,
                              std::vector<dof_id_type> & neighbor_dof_indices,
                              GlobalDataKey,
                              const TagID tag)
{
  if (dof_indices.size() == 0 && neighbor_dof_indices.size() == 0)
    return;
  if (!(*_cm)(ivar, jvar))
    return;

  auto & iv = _sys.getVariable(_tid, ivar);
  auto & jv = _sys.getVariable(_tid, jvar);
  auto & scaling_factor = iv.arrayScalingFactor();

  const unsigned int ivn = iv.number();
  const unsigned int jvn = jv.number();
  auto & ken = jacobianBlockNeighbor(Moose::ElementNeighbor, ivn, jvn, LocalDataKey{}, tag);
  auto & kne = jacobianBlockNeighbor(Moose::NeighborElement, ivn, jvn, LocalDataKey{}, tag);
  auto & knn = jacobianBlockNeighbor(Moose::NeighborNeighbor, ivn, jvn, LocalDataKey{}, tag);

  // It is guaranteed by design iv.number <= ivar since iv is obtained
  // through SystemBase::getVariable with ivar.
  // Most of times ivar will just be equal to iv.number except for array variables,
  // where ivar could be a number for a component of an array variable but calling
  // getVariable will return the array variable that has the number of the 0th component.
  // It is the same for jvar.
  const unsigned int i = ivar - ivn;
  const unsigned int j = jvar - jvn;
  // DoF indices are independently given
  auto dc = dof_indices;
  auto dn = neighbor_dof_indices;
  auto cndof = dc.size();
  auto nndof = dn.size();

  unsigned int jj = j;
  if (ivar == jvar && _component_block_diagonal[ivn])
    jj = 0;

  auto suben = ken.sub_matrix(i * cndof, cndof, jj * nndof, nndof);
  auto subne = kne.sub_matrix(i * nndof, nndof, jj * cndof, cndof);
  auto subnn = knn.sub_matrix(i * nndof, nndof, jj * nndof, nndof);

  // If we're computing the jacobian for automatically scaling variables we do not want to
  // constrain the element matrix because it introduces 1s on the diagonal for the constrained
  // dofs
  if (!_sys.computingScalingJacobian())
  {
    dof_map.constrain_element_matrix(suben, dc, dn, false);
    dof_map.constrain_element_matrix(subne, dn, dc, false);
    dof_map.constrain_element_matrix(subnn, dn, dn, false);
  }

  if (scaling_factor[i] != 1.0)
  {
    suben *= scaling_factor[i];
    subne *= scaling_factor[i];
    subnn *= scaling_factor[i];
  }

  jacobian.add_matrix(suben, dc, dn);
  jacobian.add_matrix(subne, dn, dc);
  jacobian.add_matrix(subnn, dn, dn);
}

void
Assembly::addJacobianNeighborTags(SparseMatrix<Number> & jacobian,
                                  const unsigned int ivar,
                                  const unsigned int jvar,
                                  const DofMap & dof_map,
                                  std::vector<dof_id_type> & dof_indices,
                                  std::vector<dof_id_type> & neighbor_dof_indices,
                                  GlobalDataKey,
                                  const std::set<TagID> & tags)
{
  for (const auto tag : tags)
    addJacobianNeighbor(
        jacobian, ivar, jvar, dof_map, dof_indices, neighbor_dof_indices, GlobalDataKey{}, tag);
}

void Assembly::addJacobianScalar(GlobalDataKey)
{
  for (const auto & it : _cm_ss_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);
}

void
Assembly::addJacobianOffDiagScalar(unsigned int ivar, GlobalDataKey)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  MooseVariableScalar & var_i = _sys.getScalarVariable(_tid, ivar);
  for (const auto & var_j : vars)
    addJacobianCoupledVarPair(var_i, *var_j);
}

void
Assembly::cacheJacobian(
    numeric_index_type i, numeric_index_type j, Real value, LocalDataKey, TagID tag)
{
  _cached_jacobian_rows[tag].push_back(i);
  _cached_jacobian_cols[tag].push_back(j);
  _cached_jacobian_values[tag].push_back(value);
}

void
Assembly::cacheJacobian(numeric_index_type i,
                        numeric_index_type j,
                        Real value,
                        LocalDataKey,
                        const std::set<TagID> & tags)
{
  for (auto tag : tags)
    if (_sys.hasMatrix(tag))
      cacheJacobian(i, j, value, LocalDataKey{}, tag);
}

void Assembly::setCachedJacobian(GlobalDataKey)
{
  for (MooseIndex(_cached_jacobian_rows) tag = 0; tag < _cached_jacobian_rows.size(); tag++)
    if (_sys.hasMatrix(tag))
    {
      // First zero the rows (including the diagonals) to prepare for
      // setting the cached values.
      _sys.getMatrix(tag).zero_rows(_cached_jacobian_rows[tag], 0.0);

      // TODO: Use SparseMatrix::set_values() for efficiency
      for (MooseIndex(_cached_jacobian_values) i = 0; i < _cached_jacobian_values[tag].size(); ++i)
        _sys.getMatrix(tag).set(_cached_jacobian_rows[tag][i],
                                _cached_jacobian_cols[tag][i],
                                _cached_jacobian_values[tag][i]);
    }

  clearCachedJacobian();
}

void Assembly::zeroCachedJacobian(GlobalDataKey)
{
  for (MooseIndex(_cached_jacobian_rows) tag = 0; tag < _cached_jacobian_rows.size(); tag++)
    if (_sys.hasMatrix(tag))
      _sys.getMatrix(tag).zero_rows(_cached_jacobian_rows[tag], 0.0);

  clearCachedJacobian();
}

void
Assembly::clearCachedJacobian()
{
  for (MooseIndex(_cached_jacobian_rows) tag = 0; tag < _cached_jacobian_rows.size(); tag++)
  {
    _cached_jacobian_rows[tag].clear();
    _cached_jacobian_cols[tag].clear();
    _cached_jacobian_values[tag].clear();
  }
}

void
Assembly::modifyWeightsDueToXFEM(const Elem * elem)
{
  mooseAssert(_xfem != nullptr, "This function should not be called if xfem is inactive");

  if (_current_qrule == _current_qrule_arbitrary)
    return;

  MooseArray<Real> xfem_weight_multipliers;
  if (_xfem->getXFEMWeights(xfem_weight_multipliers, elem, _current_qrule, _current_q_points))
  {
    mooseAssert(xfem_weight_multipliers.size() == _current_JxW.size(),
                "Size of weight multipliers in xfem doesn't match number of quadrature points");
    for (unsigned i = 0; i < xfem_weight_multipliers.size(); i++)
      _current_JxW[i] = _current_JxW[i] * xfem_weight_multipliers[i];

    xfem_weight_multipliers.release();
  }
}

void
Assembly::modifyFaceWeightsDueToXFEM(const Elem * elem, unsigned int side)
{
  mooseAssert(_xfem != nullptr, "This function should not be called if xfem is inactive");

  if (_current_qrule_face == _current_qrule_arbitrary)
    return;

  MooseArray<Real> xfem_face_weight_multipliers;
  if (_xfem->getXFEMFaceWeights(
          xfem_face_weight_multipliers, elem, _current_qrule_face, _current_q_points_face, side))
  {
    mooseAssert(xfem_face_weight_multipliers.size() == _current_JxW_face.size(),
                "Size of weight multipliers in xfem doesn't match number of quadrature points");
    for (unsigned i = 0; i < xfem_face_weight_multipliers.size(); i++)
      _current_JxW_face[i] = _current_JxW_face[i] * xfem_face_weight_multipliers[i];

    xfem_face_weight_multipliers.release();
  }
}

void
Assembly::hasScalingVector()
{
  _scaling_vector = &_sys.getVector("scaling_factors");
}

void
Assembly::modifyArbitraryWeights(const std::vector<Real> & weights)
{
  mooseAssert(_current_qrule == _current_qrule_arbitrary, "Rule should be arbitrary");
  mooseAssert(weights.size() == _current_physical_points.size(), "Size mismatch");

  for (MooseIndex(weights.size()) i = 0; i < weights.size(); ++i)
    _current_JxW[i] = weights[i];
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhi<VectorValue<Real>>(FEType type) const
{
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhi<VectorValue<Real>>(FEType type) const
{
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhi<VectorValue<Real>>(FEType type) const
{
  _need_second_derivative[type] = true;
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiLower<VectorValue<Real>>(FEType type) const
{
  buildVectorFE(type);
  return _vector_fe_shape_data_lower[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::feDualPhiLower<VectorValue<Real>>(FEType type) const
{
  buildVectorFE(type);
  return _vector_fe_shape_data_dual_lower[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiLower<VectorValue<Real>>(FEType type) const
{
  buildVectorFE(type);
  return _vector_fe_shape_data_lower[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradDualPhiLower<VectorValue<Real>>(FEType type) const
{
  buildVectorFE(type);
  return _vector_fe_shape_data_dual_lower[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFace<VectorValue<Real>>(FEType type) const
{
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFace<VectorValue<Real>>(FEType type) const
{
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFace<VectorValue<Real>>(FEType type) const
{
  _need_second_derivative[type] = true;
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiNeighbor<VectorValue<Real>>(FEType type) const
{
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiNeighbor<VectorValue<Real>>(FEType type) const
{
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiNeighbor<VectorValue<Real>>(FEType type) const
{
  _need_second_derivative_neighbor[type] = true;
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFaceNeighbor<VectorValue<Real>>(FEType type) const
{
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFaceNeighbor<VectorValue<Real>>(FEType type) const
{
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFaceNeighbor<VectorValue<Real>>(FEType type) const
{
  _need_second_derivative_neighbor[type] = true;
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhi<VectorValue<Real>>(FEType type) const
{
  _need_curl[type] = true;
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_curl_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFace<VectorValue<Real>>(FEType type) const
{
  _need_curl[type] = true;
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_curl_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiNeighbor<VectorValue<Real>>(FEType type) const
{
  _need_curl[type] = true;
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_curl_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFaceNeighbor<VectorValue<Real>>(FEType type) const
{
  _need_curl[type] = true;
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_curl_phi;
}

template void coordTransformFactor<Point, Real>(const SubProblem & s,
                                                SubdomainID sub_id,
                                                const Point & point,
                                                Real & factor,
                                                SubdomainID neighbor_sub_id);
template void coordTransformFactor<ADPoint, ADReal>(const SubProblem & s,
                                                    SubdomainID sub_id,
                                                    const ADPoint & point,
                                                    ADReal & factor,
                                                    SubdomainID neighbor_sub_id);
template void coordTransformFactor<Point, Real>(const MooseMesh & mesh,
                                                SubdomainID sub_id,
                                                const Point & point,
                                                Real & factor,
                                                SubdomainID neighbor_sub_id);
template void coordTransformFactor<ADPoint, ADReal>(const MooseMesh & mesh,
                                                    SubdomainID sub_id,
                                                    const ADPoint & point,
                                                    ADReal & factor,
                                                    SubdomainID neighbor_sub_id);

template <>
const MooseArray<MooseADWrapper<Point, false>> &
Assembly::genericQPoints<false>() const
{
  return qPoints();
}

template <>
const MooseArray<MooseADWrapper<Point, true>> &
Assembly::genericQPoints<true>() const
{
  return adQPoints();
}
