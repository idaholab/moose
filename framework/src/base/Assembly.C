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

Assembly::Assembly(SystemBase & sys, THREAD_ID tid)
  : _sys(sys),
    _nonlocal_cm(_sys.subproblem().nonlocalCouplingMatrix()),
    _dof_map(_sys.dofMap()),
    _tid(tid),
    _mesh(sys.mesh()),
    _mesh_dimension(_mesh.dimension()),
    _current_qrule(NULL),
    _current_qrule_volume(NULL),
    _current_qrule_arbitrary(NULL),
    _coord_type(Moose::COORD_XYZ),
    _current_qrule_face(NULL),
    _current_qface_arbitrary(NULL),
    _current_qrule_neighbor(NULL),

    _current_elem(NULL),
    _current_elem_volume(0),
    _current_side(0),
    _current_side_elem(NULL),
    _current_side_volume(0),
    _current_neighbor_elem(NULL),
    _current_neighbor_side(0),
    _current_neighbor_side_elem(NULL),
    _need_neighbor_elem_volume(false),
    _current_neighbor_volume(0),
    _current_node(NULL),
    _current_neighbor_node(NULL),
    _current_elem_volume_computed(false),
    _current_side_volume_computed(false),

    _cached_residual_values(2), // The 2 is for TIME and NONTIME
    _cached_residual_rows(2),   // The 2 is for TIME and NONTIME

    _max_cached_residuals(0),
    _max_cached_jacobians(0),
    _block_diagonal_matrix(false)
{
  // Build fe's for the helpers
  buildFE(FEType(FIRST, LAGRANGE));
  buildFaceFE(FEType(FIRST, LAGRANGE));
  buildNeighborFE(FEType(FIRST, LAGRANGE));
  buildFaceNeighborFE(FEType(FIRST, LAGRANGE));

  // Build an FE helper object for this type for each dimension up to the dimension of the current
  // mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    _holder_fe_helper[dim] = &_fe[dim][FEType(FIRST, LAGRANGE)];
    (*_holder_fe_helper[dim])->get_phi();
    (*_holder_fe_helper[dim])->get_dphi();
    (*_holder_fe_helper[dim])->get_xyz();
    (*_holder_fe_helper[dim])->get_JxW();

    _holder_fe_face_helper[dim] = &_fe_face[dim][FEType(FIRST, LAGRANGE)];
    (*_holder_fe_face_helper[dim])->get_phi();
    (*_holder_fe_face_helper[dim])->get_dphi();
    (*_holder_fe_face_helper[dim])->get_xyz();
    (*_holder_fe_face_helper[dim])->get_JxW();
    (*_holder_fe_face_helper[dim])->get_normals();

    _holder_fe_face_neighbor_helper[dim] = &_fe_face_neighbor[dim][FEType(FIRST, LAGRANGE)];
    (*_holder_fe_face_neighbor_helper[dim])->get_xyz();
    (*_holder_fe_face_neighbor_helper[dim])->get_JxW();
    (*_holder_fe_face_neighbor_helper[dim])->get_normals();

    _holder_fe_neighbor_helper[dim] = &_fe_neighbor[dim][FEType(FIRST, LAGRANGE)];
    (*_holder_fe_neighbor_helper[dim])->get_xyz();
    (*_holder_fe_neighbor_helper[dim])->get_JxW();
  }
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

  for (auto & it : _holder_qrule_volume)
    delete it.second;

  for (auto & it : _holder_qrule_arbitrary)
    delete it.second;

  for (auto & it : _holder_qface_arbitrary)
    delete it.second;

  for (auto & it : _holder_qrule_face)
    delete it.second;

  for (auto & it : _holder_qrule_neighbor)
    delete it.second;

  for (auto & it : _fe_shape_data)
    delete it.second;

  for (auto & it : _fe_shape_data_face)
    delete it.second;

  for (auto & it : _fe_shape_data_neighbor)
    delete it.second;

  for (auto & it : _fe_shape_data_face_neighbor)
    delete it.second;

  for (auto & it : _vector_fe_shape_data)
    delete it.second;

  for (auto & it : _vector_fe_shape_data_face)
    delete it.second;

  for (auto & it : _vector_fe_shape_data_neighbor)
    delete it.second;

  for (auto & it : _vector_fe_shape_data_face_neighbor)
    delete it.second;

  delete _current_side_elem;
  delete _current_neighbor_side_elem;

  _current_physical_points.release();

  _coord.release();
  _coord_neighbor.release();
}

void
Assembly::buildFE(FEType type)
{
  if (!_fe_shape_data[type])
    _fe_shape_data[type] = new FEShapeData;

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
Assembly::buildFaceFE(FEType type)
{
  if (!_fe_shape_data_face[type])
    _fe_shape_data_face[type] = new FEShapeData;

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
Assembly::buildNeighborFE(FEType type)
{
  if (!_fe_shape_data_neighbor[type])
    _fe_shape_data_neighbor[type] = new FEShapeData;

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
Assembly::buildFaceNeighborFE(FEType type)
{
  if (!_fe_shape_data_face_neighbor[type])
    _fe_shape_data_face_neighbor[type] = new FEShapeData;

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
Assembly::buildVectorFE(FEType type)
{
  if (!_vector_fe_shape_data[type])
    _vector_fe_shape_data[type] = new VectorFEShapeData;

  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  // Note that NEDELEC_ONE elements can only be built for dimension > 2. The for loop logic should
  // be modified for LAGRANGE_VEC
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe[dim][type])
      _vector_fe[dim][type] = FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe[dim][type]->get_phi();
    _vector_fe[dim][type]->get_dphi();
    _vector_fe[dim][type]->get_curl_phi();
    // Pre-request xyz.  We have always computed xyz, but due to
    // recent optimizations in libmesh, we now need to explicity
    // request it, since apps (Yak) may rely on it being computed.
    _vector_fe[dim][type]->get_xyz();
  }
}

void
Assembly::buildVectorFaceFE(FEType type)
{
  if (!_vector_fe_shape_data_face[type])
    _vector_fe_shape_data_face[type] = new VectorFEShapeData;

  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an VectorFE object for this type for each dimension up to the dimension of the current
  // mesh
  // Note that NEDELEC_ONE elements can only be built for dimension > 2. The for loop logic should
  // be modified for LAGRANGE_VEC
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe_face[dim][type])
      _vector_fe_face[dim][type] = FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe_face[dim][type]->get_phi();
    _vector_fe_face[dim][type]->get_dphi();
    _vector_fe_face[dim][type]->get_curl_phi();
  }
}

void
Assembly::buildVectorNeighborFE(FEType type)
{
  if (!_vector_fe_shape_data_neighbor[type])
    _vector_fe_shape_data_neighbor[type] = new VectorFEShapeData;

  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an VectorFE object for this type for each dimension up to the dimension of the current
  // mesh
  // Note that NEDELEC_ONE elements can only be built for dimension > 2. The for loop logic should
  // be modified for LAGRANGE_VEC
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe_neighbor[dim][type])
      _vector_fe_neighbor[dim][type] = FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe_neighbor[dim][type]->get_phi();
    _vector_fe_neighbor[dim][type]->get_dphi();
    _vector_fe_neighbor[dim][type]->get_curl_phi();
  }
}

void
Assembly::buildVectorFaceNeighborFE(FEType type)
{
  if (!_vector_fe_shape_data_face_neighbor[type])
    _vector_fe_shape_data_face_neighbor[type] = new VectorFEShapeData;

  unsigned int min_dim;
  if (type.family == LAGRANGE_VEC)
    min_dim = 0;
  else
    min_dim = 2;

  // Build an VectorFE object for this type for each dimension up to the dimension of the current
  // mesh
  // Note that NEDELEC_ONE elements can only be built for dimension > 2. The for loop logic should
  // be modified for LAGRANGE_VEC
  for (unsigned int dim = min_dim; dim <= _mesh_dimension; dim++)
  {
    if (!_vector_fe_face_neighbor[dim][type])
      _vector_fe_face_neighbor[dim][type] =
          FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe_face_neighbor[dim][type]->get_phi();
    _vector_fe_face_neighbor[dim][type]->get_dphi();
    _vector_fe_face_neighbor[dim][type]->get_curl_phi();
  }
}

const Real &
Assembly::neighborVolume()
{
  _need_neighbor_elem_volume = true;
  return _current_neighbor_volume;
}

void
Assembly::createQRules(QuadratureType type, Order order, Order volume_order, Order face_order)
{
  _holder_qrule_volume.clear();
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_volume[dim] = QBase::build(type, dim, volume_order).release();

  _holder_qrule_face.clear();
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_face[dim] = QBase::build(type, dim - 1, face_order).release();

  _holder_qrule_neighbor.clear();
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_neighbor[dim] = new ArbitraryQuadrature(dim, face_order);

  _holder_qrule_arbitrary.clear();
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_arbitrary[dim] = new ArbitraryQuadrature(dim, order);
}

void
Assembly::setVolumeQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule = qrule;

  if (qrule) // Don't set a NULL qrule
  {
    for (auto & it : _fe[dim])
      it.second->attach_quadrature_rule(_current_qrule);
    for (auto & it : _vector_fe[dim])
      it.second->attach_quadrature_rule(_current_qrule);
  }
}

void
Assembly::setFaceQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_face = qrule;

  for (auto & it : _fe_face[dim])
    it.second->attach_quadrature_rule(_current_qrule_face);
  for (auto & it : _vector_fe_face[dim])
    it.second->attach_quadrature_rule(_current_qrule_face);
}

void
Assembly::setNeighborQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_neighbor = qrule;

  for (auto & it : _fe_face_neighbor[dim])
    it.second->attach_quadrature_rule(_current_qrule_neighbor);
  for (auto & it : _vector_fe_face_neighbor[dim])
    it.second->attach_quadrature_rule(_current_qrule_neighbor);
}

void
Assembly::reinitFE(const Elem * elem)
{
  unsigned int dim = elem->dim();

  for (const auto & it : _fe[dim])
  {
    FEBase * fe = it.second;
    const FEType & fe_type = it.first;

    _current_fe[fe_type] = fe;

    FEShapeData * fesd = _fe_shape_data[fe_type];

    fe->reinit(elem);

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe->get_d2phi()));
  }
  for (const auto & it : _vector_fe[dim])
  {
    FEVectorBase * fe = it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe[fe_type] = fe;

    VectorFEShapeData * fesd = _vector_fe_shape_data[fe_type];

    fe->reinit(elem);

    fesd->_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(fe->get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd->_curl_phi.shallowCopy(
          const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe->get_curl_phi()));
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_helper[dim])->get_xyz()));
  _current_JxW.shallowCopy(const_cast<std::vector<Real> &>((*_holder_fe_helper[dim])->get_JxW()));

  if (_xfem != nullptr)
    modifyWeightsDueToXFEM(elem);
}

void
Assembly::reinitFEFace(const Elem * elem, unsigned int side)
{
  unsigned int dim = elem->dim();

  for (const auto & it : _fe_face[dim])
  {
    FEBase * fe_face = it.second;
    const FEType & fe_type = it.first;
    FEShapeData * fesd = _fe_shape_data_face[fe_type];
    fe_face->reinit(elem, side);
    _current_fe_face[fe_type] = fe_face;

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_face->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face->get_d2phi()));
  }
  for (const auto & it : _vector_fe_face[dim])
  {
    FEVectorBase * fe_face = it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_face[fe_type] = fe_face;

    VectorFEShapeData * fesd = _vector_fe_shape_data_face[fe_type];

    fe_face->reinit(elem, side);

    fesd->_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(fe_face->get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd->_curl_phi.shallowCopy(
          const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face->get_curl_phi()));
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points_face.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_xyz()));
  _current_JxW_face.shallowCopy(
      const_cast<std::vector<Real> &>((*_holder_fe_face_helper[dim])->get_JxW()));
  _current_normals.shallowCopy(
      const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_normals()));

  if (_xfem != nullptr)
    modifyFaceWeightsDueToXFEM(elem, side);
}

void
Assembly::reinitFEFaceNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points)
{
  unsigned int neighbor_dim = neighbor->dim();

  // reinit neighbor face
  for (const auto & it : _fe_face_neighbor[neighbor_dim])
  {
    FEBase * fe_face_neighbor = it.second;
    FEType fe_type = it.first;
    FEShapeData * fesd = _fe_shape_data_face_neighbor[fe_type];

    fe_face_neighbor->reinit(neighbor, &reference_points);

    _current_fe_face_neighbor[fe_type] = fe_face_neighbor;

    fesd->_phi.shallowCopy(
        const_cast<std::vector<std::vector<Real>> &>(fe_face_neighbor->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_face_neighbor->get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face_neighbor->get_d2phi()));
  }
  for (const auto & it : _vector_fe_face_neighbor[neighbor_dim])
  {
    FEVectorBase * fe_face_neighbor = it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_face_neighbor[fe_type] = fe_face_neighbor;

    VectorFEShapeData * fesd = _vector_fe_shape_data_face_neighbor[fe_type];

    fe_face_neighbor->reinit(neighbor, &reference_points);

    fesd->_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_face_neighbor->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face_neighbor->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(
          fe_face_neighbor->get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd->_curl_phi.shallowCopy(const_cast<std::vector<std::vector<VectorValue<Real>>> &>(
          fe_face_neighbor->get_curl_phi()));
  }
}

void
Assembly::reinitFENeighbor(const Elem * neighbor, const std::vector<Point> & reference_points)
{
  unsigned int neighbor_dim = neighbor->dim();

  // reinit neighbor face
  for (const auto & it : _fe_neighbor[neighbor_dim])
  {
    FEBase * fe_neighbor = it.second;
    FEType fe_type = it.first;
    FEShapeData * fesd = _fe_shape_data_neighbor[fe_type];

    fe_neighbor->reinit(neighbor, &reference_points);

    _current_fe_neighbor[fe_type] = fe_neighbor;

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_neighbor->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_neighbor->get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_neighbor->get_d2phi()));
  }
  for (const auto & it : _vector_fe_neighbor[neighbor_dim])
  {
    FEVectorBase * fe_neighbor = it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_neighbor[fe_type] = fe_neighbor;

    VectorFEShapeData * fesd = _vector_fe_shape_data_neighbor[fe_type];

    fe_neighbor->reinit(neighbor, &reference_points);

    fesd->_phi.shallowCopy(
        const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_neighbor->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_neighbor->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TypeNTensor<3, Real>>> &>(fe_neighbor->get_d2phi()));
    if (_need_curl.find(fe_type) != _need_curl.end())
      fesd->_curl_phi.shallowCopy(
          const_cast<std::vector<std::vector<VectorValue<Real>>> &>(fe_neighbor->get_curl_phi()));
  }
}

void
Assembly::reinitNeighbor(const Elem * neighbor, const std::vector<Point> & reference_points)
{
  unsigned int neighbor_dim = neighbor->dim();

  ArbitraryQuadrature * neighbor_rule = _holder_qrule_neighbor[neighbor_dim];
  neighbor_rule->setPoints(reference_points);
  setNeighborQRule(neighbor_rule, neighbor_dim);

  _current_neighbor_elem = neighbor;
  mooseAssert(_current_neighbor_subdomain_id == _current_neighbor_elem->subdomain_id(),
              "current neighbor subdomain has been set incorrectly");

  // Calculate the volume of the neighbor
  if (_need_neighbor_elem_volume)
  {
    unsigned int dim = neighbor->dim();
    FEBase * fe = *_holder_fe_neighbor_helper[dim];
    QBase * qrule = _holder_qrule_volume[dim];

    fe->attach_quadrature_rule(qrule);
    fe->reinit(neighbor);

    // set the coord transformation
    _coord_neighbor.resize(qrule->n_points());
    Moose::CoordinateSystemType coord_type =
        _sys.subproblem().getCoordSystem(_current_neighbor_subdomain_id);
    unsigned int rz_radial_coord = _sys.subproblem().getAxisymmetricRadialCoord();
    const std::vector<Real> & JxW = fe->get_JxW();
    const std::vector<Point> & q_points = fe->get_xyz();

    switch (coord_type)
    {
      case Moose::COORD_XYZ:
        for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
          _coord_neighbor[qp] = 1.;
        break;

      case Moose::COORD_RZ:
        for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
          _coord_neighbor[qp] = 2 * M_PI * q_points[qp](rz_radial_coord);
        break;

      case Moose::COORD_RSPHERICAL:
        for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
          _coord_neighbor[qp] = 4 * M_PI * q_points[qp](0) * q_points[qp](0);
        break;

      default:
        mooseError("Unknown coordinate system");
        break;
    }

    _current_neighbor_volume = 0.;
    for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
      _current_neighbor_volume += JxW[qp] * _coord_neighbor[qp];
  }
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

  _current_qrule_volume = _holder_qrule_volume[elem_dimension];

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_volume)
    setVolumeQRule(_current_qrule_volume, elem_dimension);

  reinitFE(elem);

  computeCurrentElemVolume();
}

void
Assembly::setCoordinateTransformation(const QBase * qrule, const MooseArray<Point> & q_points)
{
  _coord.resize(qrule->n_points());
  _coord_type = _sys.subproblem().getCoordSystem(_current_elem->subdomain_id());
  unsigned int rz_radial_coord = _sys.subproblem().getAxisymmetricRadialCoord();

  switch (_coord_type)
  {
    case Moose::COORD_XYZ:
      for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
        _coord[qp] = 1.;
      break;

    case Moose::COORD_RZ:
      for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
        _coord[qp] = 2 * M_PI * q_points[qp](rz_radial_coord);
      break;

    case Moose::COORD_RSPHERICAL:
      for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
        _coord[qp] = 4 * M_PI * q_points[qp](0) * q_points[qp](0);
      break;

    default:
      mooseError("Unknown coordinate system");
      break;
  }
}

void
Assembly::computeCurrentElemVolume()
{
  if (_current_elem_volume_computed)
    return;

  setCoordinateTransformation(_current_qrule, _current_q_points);

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

  setCoordinateTransformation(_current_qrule_face, _current_q_points_face);

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
Assembly::reinit(const Elem * elem, const std::vector<Point> & reference_points)
{
  _current_elem = elem;
  _current_neighbor_elem = nullptr;
  mooseAssert(_current_subdomain_id == _current_elem->subdomain_id(),
              "current subdomain has been set incorrectly");
  _current_elem_volume_computed = false;

  unsigned int elem_dimension = _current_elem->dim();

  _current_qrule_arbitrary = _holder_qrule_arbitrary[elem_dimension];

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_arbitrary)
    setVolumeQRule(_current_qrule_arbitrary, elem_dimension);

  _current_qrule_arbitrary->setPoints(reference_points);

  reinitFE(elem);

  computeCurrentElemVolume();
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

  unsigned int elem_dimension = _current_elem->dim();

  if (_current_qrule_face != _holder_qrule_face[elem_dimension])
  {
    _current_qrule_face = _holder_qrule_face[elem_dimension];
    setFaceQRule(_current_qrule_face, elem_dimension);
  }

  if (_current_side_elem)
    delete _current_side_elem;
  _current_side_elem = elem->build_side_ptr(side).release();

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
                                unsigned int neighbor_side)
{
  _current_neighbor_side = neighbor_side;

  reinit(elem, side);

  unsigned int neighbor_dim = neighbor->dim();

  std::vector<Point> reference_points;
  FEInterface::inverse_map(
      neighbor_dim, FEType(), neighbor, _current_q_points_face.stdVector(), reference_points);

  reinitFEFaceNeighbor(neighbor, reference_points);
  reinitNeighbor(neighbor, reference_points);
}

void
Assembly::reinitNeighborAtPhysical(const Elem * neighbor,
                                   unsigned int neighbor_side,
                                   const std::vector<Point> & physical_points)
{
  delete _current_neighbor_side_elem;
  _current_neighbor_side_elem = neighbor->build_side_ptr(neighbor_side).release();

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

DenseMatrix<Number> &
Assembly::jacobianBlock(unsigned int ivar, unsigned int jvar, TagID tag /* = 0 */)
{
  _jacobian_block_used[tag][ivar][jvar] = 1;
  return _sub_Kee[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
}

DenseMatrix<Number> &
Assembly::jacobianBlockNonlocal(unsigned int ivar, unsigned int jvar, TagID tag /* = 0*/)
{
  _jacobian_block_nonlocal_used[tag][ivar][jvar] = 1;
  return _sub_Keg[tag][ivar][_block_diagonal_matrix ? 0 : jvar];
}

DenseMatrix<Number> &
Assembly::jacobianBlockNeighbor(Moose::DGJacobianType type,
                                unsigned int ivar,
                                unsigned int jvar,
                                TagID tag /*=0*/)
{
  _jacobian_block_neighbor_used[tag][ivar][jvar] = 1;
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
    for (const auto & j : ConstCouplingRow(i, *_cm))
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
        _cm_ff_entry.push_back(std::make_pair(ivar, &jvar));
        if (i != j)
          _block_diagonal_matrix = false;
      }
    }
  }

  auto & scalar_vars = _sys.getScalarVariables(_tid);

  for (auto & ivar : scalar_vars)
  {
    auto i = ivar->number();
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

  auto num_vector_tags = _sys.subproblem().numVectorTags();
  auto num_matrix_tags = _sys.subproblem().numMatrixTags();

  _sub_Re.resize(num_vector_tags);
  _sub_Rn.resize(num_vector_tags);
  for (auto i = beginIndex(_sub_Re); i < _sub_Re.size(); i++)
  {
    _sub_Re[i].resize(n_vars);
    _sub_Rn[i].resize(n_vars);
  }

  _cached_residual_values.resize(num_vector_tags);
  _cached_residual_rows.resize(num_vector_tags);

  _cached_jacobian_values.resize(num_matrix_tags);
  _cached_jacobian_rows.resize(num_matrix_tags);
  _cached_jacobian_cols.resize(num_matrix_tags);

  // Element matrices
  _sub_Kee.resize(num_matrix_tags);
  _sub_Keg.resize(num_matrix_tags);
  _sub_Ken.resize(num_matrix_tags);
  _sub_Kne.resize(num_matrix_tags);
  _sub_Knn.resize(num_matrix_tags);

  _jacobian_block_used.resize(num_matrix_tags);
  _jacobian_block_neighbor_used.resize(num_matrix_tags);
  _jacobian_block_nonlocal_used.resize(num_matrix_tags);

  for (unsigned int tag = 0; tag < num_matrix_tags; tag++)
  {
    _sub_Keg[tag].resize(n_vars);
    _sub_Ken[tag].resize(n_vars);
    _sub_Kne[tag].resize(n_vars);
    _sub_Knn[tag].resize(n_vars);
    _sub_Kee[tag].resize(n_vars);

    _jacobian_block_used[tag].resize(n_vars);
    _jacobian_block_neighbor_used[tag].resize(n_vars);
    _jacobian_block_nonlocal_used[tag].resize(n_vars);
    for (unsigned int i = 0; i < n_vars; ++i)
    {
      if (!_block_diagonal_matrix)
      {
        _sub_Kee[tag][i].resize(n_vars);
        _sub_Keg[tag][i].resize(n_vars);
        _sub_Ken[tag][i].resize(n_vars);
        _sub_Kne[tag][i].resize(n_vars);
        _sub_Knn[tag][i].resize(n_vars);
      }
      else
      {
        _sub_Kee[tag][i].resize(1);
        _sub_Keg[tag][i].resize(1);
        _sub_Ken[tag][i].resize(1);
        _sub_Kne[tag][i].resize(1);
        _sub_Knn[tag][i].resize(1);
      }
      _jacobian_block_used[tag][i].resize(n_vars);
      _jacobian_block_neighbor_used[tag][i].resize(n_vars);
      _jacobian_block_nonlocal_used[tag][i].resize(n_vars);
    }
  }

  // Cached Jacobian contributions
  _cached_jacobian_contribution_vals.resize(num_matrix_tags);
  _cached_jacobian_contribution_rows.resize(num_matrix_tags);
  _cached_jacobian_contribution_cols.resize(num_matrix_tags);
}

void
Assembly::initNonlocalCoupling()
{
  _cm_nonlocal_entry.clear();

  auto & vars = _sys.getVariables(_tid);

  for (auto & ivar : vars)
  {
    auto i = ivar->number();
    for (const auto & j : ConstCouplingRow(i, _nonlocal_cm))
      if (!_sys.isScalarVariable(j))
      {
        auto & jvar = _sys.getVariable(_tid, j);
        _cm_nonlocal_entry.push_back(std::make_pair(ivar, &jvar));
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

    for (auto tag = beginIndex(_jacobian_block_used); tag < _jacobian_block_used.size(); tag++)
    {
      jacobianBlock(vi, vj, tag).resize(ivar.dofIndices().size(), jvar.dofIndices().size());
      jacobianBlock(vi, vj, tag).zero();
      _jacobian_block_used[tag][vi][vj] = 0;
    }
  }
}

void
Assembly::prepareResidual()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (auto tag = beginIndex(_sub_Re); tag < _sub_Re.size(); tag++)
    {
      _sub_Re[tag][var->number()].resize(var->dofIndices().size());
      _sub_Re[tag][var->number()].zero();
    }
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

    for (auto tag = beginIndex(_jacobian_block_nonlocal_used);
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
    {
      jacobianBlockNonlocal(vi, vj, tag)
          .resize(ivar.dofIndices().size(), jvar.allDofIndices().size());
      jacobianBlockNonlocal(vi, vj, tag).zero();
      _jacobian_block_nonlocal_used[tag][vi][vj] = 0;
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

    if (vi == var->number() || vj == var->number())
    {
      for (auto tag = beginIndex(_jacobian_block_used); tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(vi, vj, tag).resize(ivar.dofIndices().size(), jvar.dofIndices().size());
        jacobianBlock(vi, vj, tag).zero();
        _jacobian_block_used[tag][vi][vj] = 0;
      }
    }
  }

  for (auto tag = beginIndex(_sub_Re); tag < _sub_Re.size(); tag++)
  {
    _sub_Re[tag][var->number()].resize(var->dofIndices().size());
    _sub_Re[tag][var->number()].zero();
  }
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

    if (vi == var->number() || vj == var->number())
    {
      for (auto tag = beginIndex(_jacobian_block_nonlocal_used);
           tag < _jacobian_block_nonlocal_used.size();
           tag++)
      {
        jacobianBlockNonlocal(vi, vj, tag)
            .resize(ivar.dofIndices().size(), jvar.allDofIndices().size());
        jacobianBlockNonlocal(vi, vj, tag).zero();
        _jacobian_block_nonlocal_used[tag][vi][vj] = 0;
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

    for (auto tag = beginIndex(_jacobian_block_neighbor_used);
         tag < _jacobian_block_neighbor_used.size();
         tag++)
    {
      jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj, tag)
          .resize(ivar.dofIndices().size(), jvar.dofIndicesNeighbor().size());
      jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj, tag).zero();

      jacobianBlockNeighbor(Moose::NeighborElement, vi, vj, tag)
          .resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndices().size());
      jacobianBlockNeighbor(Moose::NeighborElement, vi, vj, tag).zero();

      jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj, tag)
          .resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndicesNeighbor().size());
      jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj, tag).zero();

      _jacobian_block_neighbor_used[tag][vi][vj] = 0;
    }
  }

  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (auto tag = beginIndex(_sub_Rn); tag < _sub_Rn.size(); tag++)
    {
      _sub_Rn[tag][var->number()].resize(var->dofIndicesNeighbor().size());
      _sub_Rn[tag][var->number()].zero();
    }
}

void
Assembly::prepareBlock(unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices)
{
  for (auto tag = beginIndex(_jacobian_block_used); tag < _jacobian_block_used.size(); tag++)
  {
    jacobianBlock(ivar, jvar, tag).resize(dof_indices.size(), dof_indices.size());
    jacobianBlock(ivar, jvar, tag).zero();
    _jacobian_block_used[tag][ivar][jvar] = 0;
  }

  for (auto tag = beginIndex(_sub_Re); tag < _sub_Re.size(); tag++)
  {
    _sub_Re[tag][ivar].resize(dof_indices.size());
    _sub_Re[tag][ivar].zero();
  }
}

void
Assembly::prepareBlockNonlocal(unsigned int ivar,
                               unsigned int jvar,
                               const std::vector<dof_id_type> & idof_indices,
                               const std::vector<dof_id_type> & jdof_indices)
{
  for (auto tag = beginIndex(_jacobian_block_nonlocal_used);
       tag < _jacobian_block_nonlocal_used.size();
       tag++)
  {
    jacobianBlockNonlocal(ivar, jvar, tag).resize(idof_indices.size(), jdof_indices.size());
    jacobianBlockNonlocal(ivar, jvar, tag).zero();
    _jacobian_block_nonlocal_used[tag][ivar][jvar] = 0;
  }
}

void
Assembly::prepareScalar()
{
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (const auto & ivar : vars)
  {
    unsigned int idofs = ivar->dofIndices().size();

    for (auto tag = beginIndex(_sub_Re); tag < _sub_Re.size(); tag++)
    {
      _sub_Re[tag][ivar->number()].resize(idofs);
      _sub_Re[tag][ivar->number()].zero();
    }

    for (const auto & jvar : vars)
    {
      unsigned int jdofs = jvar->dofIndices().size();

      for (auto tag = beginIndex(_jacobian_block_used); tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(ivar->number(), jvar->number(), tag).resize(idofs, jdofs);
        jacobianBlock(ivar->number(), jvar->number(), tag).zero();
        _jacobian_block_used[tag][ivar->number()][jvar->number()] = 0;
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
    unsigned int idofs = ivar->dofIndices().size();

    for (const auto & jvar : vars)
    {
      unsigned int jdofs = jvar->dofIndices().size();
      for (auto tag = beginIndex(_jacobian_block_used); tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(ivar->number(), jvar->number(), tag).resize(idofs, jdofs);
        jacobianBlock(ivar->number(), jvar->number(), tag).zero();
        _jacobian_block_used[tag][ivar->number()][jvar->number()] = 0;

        jacobianBlock(jvar->number(), ivar->number(), tag).resize(jdofs, idofs);
        jacobianBlock(jvar->number(), ivar->number(), tag).zero();
        _jacobian_block_used[tag][jvar->number()][ivar->number()] = 0;
      }
    }
  }
}

template <typename T>
void
Assembly::copyShapes(MooseVariableFE<T> & v)
{
  phi(v).shallowCopy(v.phi());
  gradPhi(v).shallowCopy(v.gradPhi());
  if (v.computingSecond())
    secondPhi(v).shallowCopy(v.secondPhi());
}

void
Assembly::copyShapes(unsigned int var)
{
  try
  {
    MooseVariable & v = _sys.getFieldVariable<Real>(_tid, var);
    copyShapes(v);
  }
  catch (std::out_of_range & e)
  {
    VectorMooseVariable & v = _sys.getFieldVariable<RealVectorValue>(_tid, var);
    copyShapes(v);
    if (v.computingCurl())
      curlPhi(v).shallowCopy(v.curlPhi());
  }
}

template <typename T>
void
Assembly::copyFaceShapes(MooseVariableFE<T> & v)
{
  phiFace(v).shallowCopy(v.phiFace());
  gradPhiFace(v).shallowCopy(v.gradPhiFace());
  if (v.computingSecond())
    secondPhiFace(v).shallowCopy(v.secondPhiFace());
}

void
Assembly::copyFaceShapes(unsigned int var)
{
  try
  {
    MooseVariable & v = _sys.getFieldVariable<Real>(_tid, var);
    copyFaceShapes(v);
  }
  catch (std::out_of_range & e)
  {
    VectorMooseVariable & v = _sys.getFieldVariable<RealVectorValue>(_tid, var);
    copyFaceShapes(v);
    if (v.computingCurl())
      _vector_curl_phi_face.shallowCopy(v.curlPhi());
  }
}

template <typename T>
void
Assembly::copyNeighborShapes(MooseVariableFE<T> & v)
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
  try
  {
    MooseVariable & v = _sys.getFieldVariable<Real>(_tid, var);
    copyNeighborShapes(v);
  }
  catch (std::out_of_range & e)
  {
    VectorMooseVariable & v = _sys.getFieldVariable<RealVectorValue>(_tid, var);
    copyNeighborShapes(v);
  }
}

void
Assembly::addResidualBlock(NumericVector<Number> & residual,
                           DenseVector<Number> & res_block,
                           const std::vector<dof_id_type> & dof_indices,
                           Real scaling_factor)
{
  if (dof_indices.size() > 0 && res_block.size())
  {
    _temp_dof_indices = dof_indices;
    _dof_map.constrain_element_vector(res_block, _temp_dof_indices, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Re = res_block;
      _tmp_Re *= scaling_factor;
      residual.add_vector(_tmp_Re, _temp_dof_indices);
    }
    else
    {
      residual.add_vector(res_block, _temp_dof_indices);
    }
  }
}

void
Assembly::cacheResidualBlock(std::vector<Real> & cached_residual_values,
                             std::vector<dof_id_type> & cached_residual_rows,
                             DenseVector<Number> & res_block,
                             std::vector<dof_id_type> & dof_indices,
                             Real scaling_factor)
{
  if (dof_indices.size() > 0 && res_block.size())
  {
    _temp_dof_indices = dof_indices;
    _dof_map.constrain_element_vector(res_block, _temp_dof_indices, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Re = res_block;
      _tmp_Re *= scaling_factor;

      for (unsigned int i = 0; i < _tmp_Re.size(); i++)
      {
        cached_residual_values.push_back(_tmp_Re(i));
        cached_residual_rows.push_back(_temp_dof_indices[i]);
      }
    }
    else
    {
      for (unsigned int i = 0; i < res_block.size(); i++)
      {
        cached_residual_values.push_back(res_block(i));
        cached_residual_rows.push_back(_temp_dof_indices[i]);
      }
    }
  }

  res_block.zero();
}

void
Assembly::addResidual(NumericVector<Number> & residual, TagID tag_id /* = 0 */)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(
        residual, _sub_Re[tag_id][var->number()], var->dofIndices(), var->scalingFactor());
}

void
Assembly::addResidual(const std::map<TagName, TagID> & tags)
{
  for (auto & tag : tags)
    if (_sys.hasVector(tag.second))
      addResidual(_sys.getVector(tag.second), tag.second);
}

void
Assembly::addResidualNeighbor(NumericVector<Number> & residual, TagID tag_id /* = 0 */)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(
        residual, _sub_Rn[tag_id][var->number()], var->dofIndicesNeighbor(), var->scalingFactor());
}

void
Assembly::addResidualNeighbor(const std::map<TagName, TagID> & tags)
{
  for (auto & tag : tags)
    if (_sys.hasVector(tag.second))
      addResidualNeighbor(_sys.getVector(tag.second), tag.second);
}

void
Assembly::addResidualScalar(TagID tag_id)
{
  // add the scalar variables residuals
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (const auto & var : vars)
    if (_sys.hasVector(tag_id))
      addResidualBlock(_sys.getVector(tag_id),
                       _sub_Re[tag_id][var->number()],
                       var->dofIndices(),
                       var->scalingFactor());
}

void
Assembly::addResidualScalar(const std::map<TagName, TagID> & tags)
{
  for (auto & tag : tags)
    addResidualScalar(tag.second);
}

void
Assembly::cacheResidual()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
  {
    for (auto tag = beginIndex(_cached_residual_values); tag < _cached_residual_values.size();
         tag++)
      if (_sys.hasVector(tag))
        cacheResidualBlock(_cached_residual_values[tag],
                           _cached_residual_rows[tag],
                           _sub_Re[tag][var->number()],
                           var->dofIndices(),
                           var->scalingFactor());
  }
}

void
Assembly::cacheResidualContribution(dof_id_type dof, Real value, TagID tag_id)
{
  _cached_residual_values[tag_id].push_back(value);
  _cached_residual_rows[tag_id].push_back(dof);
}

void
Assembly::cacheResidualContribution(dof_id_type dof, Real value, const std::set<TagID> & tags)
{
  for (auto & tag : tags)
    cacheResidualContribution(dof, value, tag);
}

void
Assembly::cacheResidualNeighbor()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
  {
    for (auto tag = beginIndex(_cached_residual_values); tag < _cached_residual_values.size();
         tag++)
    {
      if (_sys.hasVector(tag))
        cacheResidualBlock(_cached_residual_values[tag],
                           _cached_residual_rows[tag],
                           _sub_Rn[tag][var->number()],
                           var->dofIndicesNeighbor(),
                           var->scalingFactor());
    }
  }
}

void
Assembly::cacheResidualNodes(const DenseVector<Number> & res,
                             std::vector<dof_id_type> & dof_index,
                             TagID tag /* = 0*/)
{
  // Add the residual value and dof_index to cached_residual_values and cached_residual_rows
  // respectively.
  // This is used by NodalConstraint.C to cache the residual calculated for master and slave node.
  for (unsigned int i = 0; i < dof_index.size(); ++i)
  {
    _cached_residual_values[tag].push_back(res(i));
    _cached_residual_rows[tag].push_back(dof_index[i]);
  }
}

void
Assembly::addCachedResiduals()
{
  for (auto tag = beginIndex(_cached_residual_values); tag < _cached_residual_values.size(); tag++)
  {
    if (!_sys.hasVector(tag))
    {
      _cached_residual_values[tag].clear();
      _cached_residual_rows[tag].clear();
      continue;
    }
    addCachedResidual(_sys.getVector(tag), tag);
  }
}

void
Assembly::addCachedResidual(NumericVector<Number> & residual, TagID tag_id)
{
  if (!_sys.hasVector(tag_id))
  {
    // Only clean up things when tag exists
    if (_sys.subproblem().vectorTagExists(tag_id))
    {
      _cached_residual_values[tag_id].clear();
      _cached_residual_rows[tag_id].clear();
    }
    return;
  }

  std::vector<Real> & cached_residual_values = _cached_residual_values[tag_id];
  std::vector<dof_id_type> & cached_residual_rows = _cached_residual_rows[tag_id];

  mooseAssert(cached_residual_values.size() == cached_residual_rows.size(),
              "Number of cached residuals and number of rows must match!");
  if (cached_residual_values.size())
    residual.add_vector(cached_residual_values, cached_residual_rows);

  if (_max_cached_residuals < cached_residual_values.size())
    _max_cached_residuals = cached_residual_values.size();

  // Try to be more efficient from now on
  // The 2 is just a fudge factor to keep us from having to grow the vector during assembly
  cached_residual_values.clear();
  cached_residual_values.reserve(_max_cached_residuals * 2);

  cached_residual_rows.clear();
  cached_residual_rows.reserve(_max_cached_residuals * 2);
}

void
Assembly::setResidualBlock(NumericVector<Number> & residual,
                           DenseVector<Number> & res_block,
                           std::vector<dof_id_type> & dof_indices,
                           Real scaling_factor)
{
  if (dof_indices.size() > 0)
  {
    std::vector<dof_id_type> di(dof_indices);
    _dof_map.constrain_element_vector(res_block, di, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Re = res_block;
      _tmp_Re *= scaling_factor;
      residual.insert(_tmp_Re, di);
    }
    else
      residual.insert(res_block, di);
  }
}

void
Assembly::setResidual(NumericVector<Number> & residual, TagID tag_id /* = 0 */)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(
        residual, _sub_Re[tag_id][var->number()], var->dofIndices(), var->scalingFactor());
}

void
Assembly::setResidualNeighbor(NumericVector<Number> & residual, TagID tag_id /* = 0 */)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(
        residual, _sub_Rn[tag_id][var->number()], var->dofIndicesNeighbor(), var->scalingFactor());
}

void
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian,
                           DenseMatrix<Number> & jac_block,
                           const std::vector<dof_id_type> & idof_indices,
                           const std::vector<dof_id_type> & jdof_indices,
                           Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0) && jac_block.n() && jac_block.m())
  {
    std::vector<dof_id_type> di(idof_indices);
    std::vector<dof_id_type> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
    {
      _tmp_Ke = jac_block;
      _tmp_Ke *= scaling_factor;
      jacobian.add_matrix(_tmp_Ke, di, dj);
    }
    else
      jacobian.add_matrix(jac_block, di, dj);
  }
}

void
Assembly::cacheJacobianBlock(DenseMatrix<Number> & jac_block,
                             std::vector<dof_id_type> & idof_indices,
                             std::vector<dof_id_type> & jdof_indices,
                             Real scaling_factor,
                             TagID tag /*=0*/)
{
  // Only cache data when the matrix exists
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0) && jac_block.n() && jac_block.m() &&
      _sys.hasMatrix(tag))
  {
    std::vector<dof_id_type> di(idof_indices);
    std::vector<dof_id_type> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
      jac_block *= scaling_factor;

    for (unsigned int i = 0; i < di.size(); i++)
      for (unsigned int j = 0; j < dj.size(); j++)
      {
        _cached_jacobian_values[tag].push_back(jac_block(i, j));
        _cached_jacobian_rows[tag].push_back(di[i]);
        _cached_jacobian_cols[tag].push_back(dj[j]);
      }
  }
  jac_block.zero();
}

void
Assembly::cacheJacobianBlockNonlocal(DenseMatrix<Number> & jac_block,
                                     const std::vector<dof_id_type> & idof_indices,
                                     const std::vector<dof_id_type> & jdof_indices,
                                     Real scaling_factor,
                                     TagID tag /*= 0*/)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0) && jac_block.n() && jac_block.m() &&
      _sys.hasMatrix(tag))
  {
    std::vector<dof_id_type> di(idof_indices);
    std::vector<dof_id_type> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
      jac_block *= scaling_factor;

    for (unsigned int i = 0; i < di.size(); i++)
      for (unsigned int j = 0; j < dj.size(); j++)
        if (jac_block(i, j) != 0.0) // no storage allocated for unimplemented jacobian terms,
                                    // maintaining maximum sparsity possible
        {
          _cached_jacobian_values[tag].push_back(jac_block(i, j));
          _cached_jacobian_rows[tag].push_back(di[i]);
          _cached_jacobian_cols[tag].push_back(dj[j]);
        }
  }
  jac_block.zero();
}

void
Assembly::addCachedJacobian(SparseMatrix<Number> & /*jacobian*/)
{
  mooseDeprecated(" Please use addCachedJacobian() ");

  addCachedJacobian();
}

void
Assembly::addCachedJacobian()
{
  if (!_sys.subproblem().checkNonlocalCouplingRequirement())
  {
    mooseAssert(_cached_jacobian_rows.size() == _cached_jacobian_cols.size(),
                "Error: Cached data sizes MUST be the same!");
    for (unsigned int i = 0; i < _cached_jacobian_rows.size(); i++)
      mooseAssert(_cached_jacobian_rows[i].size() == _cached_jacobian_cols[i].size(),
                  "Error: Cached data sizes MUST be the same for a given tag!");
  }

  for (unsigned int i = 0; i < _cached_jacobian_rows.size(); i++)
    if (_sys.hasMatrix(i))
      for (unsigned int j = 0; j < _cached_jacobian_rows[i].size(); j++)
        _sys.getMatrix(i).add(_cached_jacobian_rows[i][j],
                              _cached_jacobian_cols[i][j],
                              _cached_jacobian_values[i][j]);

  for (unsigned int i = 0; i < _cached_jacobian_rows.size(); i++)
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
Assembly::addJacobianCoupledVarPair(MooseVariableBase * ivar, MooseVariableBase * jvar)
{
  auto i = ivar->number();
  auto j = jvar->number();
  for (unsigned int tag = beginIndex(_jacobian_block_used); tag < _jacobian_block_used.size();
       tag++)
    if (_jacobian_block_used[tag][i][j] && _sys.hasMatrix(tag))
      addJacobianBlock(_sys.getMatrix(tag),
                       jacobianBlock(i, j, tag),
                       ivar->dofIndices(),
                       jvar->dofIndices(),
                       ivar->scalingFactor());
}

void
Assembly::addJacobian()
{
  for (const auto & it : _cm_ff_entry)
    addJacobianCoupledVarPair(it.first, it.second);

  for (const auto & it : _cm_sf_entry)
    addJacobianCoupledVarPair(it.first, it.second);

  for (const auto & it : _cm_fs_entry)
    addJacobianCoupledVarPair(it.first, it.second);
}

void
Assembly::addJacobianNonlocal()
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (auto tag = beginIndex(_jacobian_block_nonlocal_used);
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
      if (_jacobian_block_nonlocal_used[tag][i][j] && _sys.hasMatrix(tag))
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNonlocal(i, j, tag),
                         ivar->dofIndices(),
                         jvar->allDofIndices(),
                         ivar->scalingFactor());
  }
}

void
Assembly::addJacobianNeighbor()
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (auto tag = beginIndex(_jacobian_block_neighbor_used);
         tag < _jacobian_block_neighbor_used.size();
         tag++)
      if (_jacobian_block_neighbor_used[tag][i][j] && _sys.hasMatrix(tag))
      {
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::ElementNeighbor, i, j, tag),
                         ivar->dofIndices(),
                         jvar->dofIndicesNeighbor(),
                         ivar->scalingFactor());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborElement, i, j, tag),
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndices(),
                         ivar->scalingFactor());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborNeighbor, i, j, tag),
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndicesNeighbor(),
                         ivar->scalingFactor());
      }
  }
}

inline void
Assembly::cacheJacobianCoupledVarPair(MooseVariableBase * ivar, MooseVariableBase * jvar)
{
  auto i = ivar->number();
  auto j = jvar->number();
  for (unsigned int tag = beginIndex(_jacobian_block_used); tag < _jacobian_block_used.size();
       tag++)
    if (_jacobian_block_used[tag][i][j] && _sys.hasMatrix(tag))
      cacheJacobianBlock(jacobianBlock(i, j, tag),
                         ivar->dofIndices(),
                         jvar->dofIndices(),
                         ivar->scalingFactor(),
                         tag);
}

void
Assembly::cacheJacobian()
{
  for (const auto & it : _cm_ff_entry)
    cacheJacobianCoupledVarPair(it.first, it.second);

  for (const auto & it : _cm_fs_entry)
    cacheJacobianCoupledVarPair(it.first, it.second);

  for (const auto & it : _cm_sf_entry)
    cacheJacobianCoupledVarPair(it.first, it.second);
}

void
Assembly::cacheJacobianNonlocal()
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (auto tag = beginIndex(_jacobian_block_nonlocal_used);
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
      if (_jacobian_block_nonlocal_used[tag][i][j] && _sys.hasMatrix(tag))
        cacheJacobianBlockNonlocal(jacobianBlockNonlocal(i, j, tag),
                                   ivar->dofIndices(),
                                   jvar->allDofIndices(),
                                   ivar->scalingFactor(),
                                   tag);
  }
}

void
Assembly::cacheJacobianNeighbor()
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();

    for (auto tag = beginIndex(_jacobian_block_neighbor_used);
         tag < _jacobian_block_neighbor_used.size();
         tag++)
      if (_jacobian_block_neighbor_used[tag][i][j] && _sys.hasMatrix(tag))
      {
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::ElementNeighbor, i, j, tag),
                           ivar->dofIndices(),
                           jvar->dofIndicesNeighbor(),
                           ivar->scalingFactor(),
                           tag);
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborElement, i, j, tag),
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndices(),
                           ivar->scalingFactor(),
                           tag);
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborNeighbor, i, j, tag),
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndicesNeighbor(),
                           ivar->scalingFactor(),
                           tag);
      }
  }
}

void
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian,
                           unsigned int ivar,
                           unsigned int jvar,
                           const DofMap & dof_map,
                           std::vector<dof_id_type> & dof_indices)
{
  DenseMatrix<Number> & ke = jacobianBlock(ivar, jvar);

  // stick it into the matrix
  std::vector<dof_id_type> di(dof_indices);
  dof_map.constrain_element_matrix(ke, di, false);

  Real scaling_factor = _sys.getVariable(_tid, ivar).scalingFactor();
  if (scaling_factor != 1.0)
  {
    _tmp_Ke = ke;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, di);
  }
  else
    jacobian.add_matrix(ke, di);
}

void
Assembly::addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   const std::vector<dof_id_type> & idof_indices,
                                   const std::vector<dof_id_type> & jdof_indices)
{
  DenseMatrix<Number> & keg = jacobianBlockNonlocal(ivar, jvar);

  std::vector<dof_id_type> di(idof_indices);
  std::vector<dof_id_type> dg(jdof_indices);
  dof_map.constrain_element_matrix(keg, di, dg, false);

  Real scaling_factor = _sys.getVariable(_tid, ivar).scalingFactor();
  if (scaling_factor != 1.0)
  {
    _tmp_Ke = keg;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, di, dg);
  }
  else
    jacobian.add_matrix(keg, di, dg);
}

void
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                              unsigned int ivar,
                              unsigned int jvar,
                              const DofMap & dof_map,
                              std::vector<dof_id_type> & dof_indices,
                              std::vector<dof_id_type> & neighbor_dof_indices)
{
  DenseMatrix<Number> & kee = jacobianBlock(ivar, jvar);
  DenseMatrix<Number> & ken = jacobianBlockNeighbor(Moose::ElementNeighbor, ivar, jvar);
  DenseMatrix<Number> & kne = jacobianBlockNeighbor(Moose::NeighborElement, ivar, jvar);
  DenseMatrix<Number> & knn = jacobianBlockNeighbor(Moose::NeighborNeighbor, ivar, jvar);

  std::vector<dof_id_type> di(dof_indices);
  std::vector<dof_id_type> dn(neighbor_dof_indices);
  // stick it into the matrix
  dof_map.constrain_element_matrix(kee, di, false);
  dof_map.constrain_element_matrix(ken, di, dn, false);
  dof_map.constrain_element_matrix(kne, dn, di, false);
  dof_map.constrain_element_matrix(knn, dn, false);

  Real scaling_factor = _sys.getVariable(_tid, ivar).scalingFactor();
  if (scaling_factor != 1.0)
  {
    _tmp_Ke = ken;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, di, dn);

    _tmp_Ke = kne;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, dn, di);

    _tmp_Ke = knn;
    _tmp_Ke *= scaling_factor;
    jacobian.add_matrix(_tmp_Ke, dn);
  }
  else
  {
    jacobian.add_matrix(ken, di, dn);
    jacobian.add_matrix(kne, dn, di);
    jacobian.add_matrix(knn, dn);
  }
}

void
Assembly::addJacobianScalar()
{
  for (const auto & it : _cm_ss_entry)
    addJacobianCoupledVarPair(it.first, it.second);
}

void
Assembly::addJacobianOffDiagScalar(unsigned int ivar)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  MooseVariableScalar & var_i = _sys.getScalarVariable(_tid, ivar);
  for (const auto & var_j : vars)
    addJacobianCoupledVarPair(&var_i, var_j);
}

void
Assembly::cacheJacobianContribution(numeric_index_type i,
                                    numeric_index_type j,
                                    Real value,
                                    TagID tag /* = 0*/)
{
  _cached_jacobian_contribution_rows[tag].push_back(i);
  _cached_jacobian_contribution_cols[tag].push_back(j);
  _cached_jacobian_contribution_vals[tag].push_back(value);
}

void
Assembly::setCachedJacobianContributions()
{
  for (auto tag = beginIndex(_cached_jacobian_contribution_rows);
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
    if (_sys.hasMatrix(tag))
    {
      // First zero the rows (including the diagonals) to prepare for
      // setting the cached values.
      _sys.getMatrix(tag).zero_rows(_cached_jacobian_contribution_rows[tag], 0.0);

      // TODO: Use SparseMatrix::set_values() for efficiency
      for (unsigned int i = 0; i < _cached_jacobian_contribution_vals[tag].size(); ++i)
        _sys.getMatrix(tag).set(_cached_jacobian_contribution_rows[tag][i],
                                _cached_jacobian_contribution_cols[tag][i],
                                _cached_jacobian_contribution_vals[tag][i]);
    }

  clearCachedJacobianContributions();
}

void
Assembly::zeroCachedJacobianContributions()
{
  for (auto tag = beginIndex(_cached_jacobian_contribution_rows);
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
    if (_sys.hasMatrix(tag))
      _sys.getMatrix(tag).zero_rows(_cached_jacobian_contribution_rows[tag], 0.0);

  clearCachedJacobianContributions();
}

void
Assembly::addCachedJacobianContributions()
{
  for (auto tag = beginIndex(_cached_jacobian_contribution_rows);
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
    if (_sys.hasMatrix(tag))
    {
      // TODO: Use SparseMatrix::set_values() for efficiency
      for (unsigned int i = 0; i < _cached_jacobian_contribution_vals[tag].size(); ++i)
        _sys.getMatrix(tag).add(_cached_jacobian_contribution_rows[tag][i],
                                _cached_jacobian_contribution_cols[tag][i],
                                _cached_jacobian_contribution_vals[tag][i]);
    }

  clearCachedJacobianContributions();
}

void
Assembly::clearCachedJacobianContributions()
{
  for (auto tag = beginIndex(_cached_jacobian_contribution_rows);
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
  {
    unsigned int orig_size = _cached_jacobian_contribution_rows[tag].size();

    _cached_jacobian_contribution_rows[tag].clear();
    _cached_jacobian_contribution_cols[tag].clear();
    _cached_jacobian_contribution_vals[tag].clear();

    // It's possible (though massively unlikely) that clear() will
    // change the capacity of the vectors, so let's be paranoid and
    // explicitly reserve() the same amount of memory to avoid multiple
    // push_back() induced allocations.  We reserve 20% more than the
    // original size that was cached to account for variations in the
    // number of BCs assigned to each thread (for when the Jacobian
    // contributions are computed threaded).
    _cached_jacobian_contribution_rows[tag].reserve(1.2 * orig_size);
    _cached_jacobian_contribution_cols[tag].reserve(1.2 * orig_size);
    _cached_jacobian_contribution_vals[tag].reserve(1.2 * orig_size);
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

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhi<VectorValue<Real>>(FEType type)
{
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhi<VectorValue<Real>>(FEType type)
{
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhi<VectorValue<Real>>(FEType type)
{
  _need_second_derivative[type] = true;
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFace<VectorValue<Real>>(FEType type)
{
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFace<VectorValue<Real>>(FEType type)
{
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFace<VectorValue<Real>>(FEType type)
{
  _need_second_derivative[type] = true;
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiNeighbor<VectorValue<Real>>(FEType type)
{
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiNeighbor<VectorValue<Real>>(FEType type)
{
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiNeighbor<VectorValue<Real>>(FEType type)
{
  _need_second_derivative_neighbor[type] = true;
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiValue &
Assembly::fePhiFaceNeighbor<VectorValue<Real>>(FEType type)
{
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiFaceNeighbor<VectorValue<Real>>(FEType type)
{
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_grad_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiSecond &
Assembly::feSecondPhiFaceNeighbor<VectorValue<Real>>(FEType type)
{
  _need_second_derivative_neighbor[type] = true;
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_second_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhi<VectorValue<Real>>(FEType type)
{
  _need_curl[type] = true;
  buildVectorFE(type);
  return _vector_fe_shape_data[type]->_curl_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFace<VectorValue<Real>>(FEType type)
{
  _need_curl[type] = true;
  buildVectorFaceFE(type);
  return _vector_fe_shape_data_face[type]->_curl_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiNeighbor<VectorValue<Real>>(FEType type)
{
  _need_curl[type] = true;
  buildVectorNeighborFE(type);
  return _vector_fe_shape_data_neighbor[type]->_curl_phi;
}

template <>
const typename OutputTools<VectorValue<Real>>::VariablePhiCurl &
Assembly::feCurlPhiFaceNeighbor<VectorValue<Real>>(FEType type)
{
  _need_curl[type] = true;
  buildVectorFaceNeighborFE(type);
  return _vector_fe_shape_data_face_neighbor[type]->_curl_phi;
}
