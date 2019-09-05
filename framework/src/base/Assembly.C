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

Assembly::Assembly(SystemBase & sys, THREAD_ID tid)
  : _sys(sys),
    _subproblem(_sys.subproblem()),
    _displaced(dynamic_cast<DisplacedSystem *>(&sys) ? true : false),
    _nonlocal_cm(_subproblem.nonlocalCouplingMatrix()),
    _computing_jacobian(_subproblem.currentlyComputingJacobian()),
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
    _qrule_msm(nullptr),

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

    _cached_residual_values(2), // The 2 is for TIME and NONTIME
    _cached_residual_rows(2),   // The 2 is for TIME and NONTIME

    _max_cached_residuals(0),
    _max_cached_jacobians(0),
    _block_diagonal_matrix(false),
    _calculate_xyz(false),
    _calculate_face_xyz(false),
    _calculate_curvatures(false)
{
  Order helper_order = _mesh.hasSecondOrderElements() ? SECOND : FIRST;
  // Build fe's for the helpers
  buildFE(FEType(helper_order, LAGRANGE));
  buildFaceFE(FEType(helper_order, LAGRANGE));
  buildNeighborFE(FEType(helper_order, LAGRANGE));
  buildFaceNeighborFE(FEType(helper_order, LAGRANGE));

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

  _fe_msm = FEGenericBase<Real>::build(_mesh_dimension - 1, FEType(helper_order, LAGRANGE));
  _JxW_msm = &_fe_msm->get_JxW();
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

  for (auto & it : _holder_qrule_volume)
    delete it.second;

  for (auto & it : _holder_qrule_arbitrary)
    delete it.second;

  for (auto & it : _holder_qrule_arbitrary_face)
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

  for (auto & it : _fe_shape_data_lower)
    delete it.second;

  for (auto & it : _ad_grad_phi_data)
    it.second.release();

  for (auto & it : _ad_vector_grad_phi_data)
    it.second.release();

  for (auto & it : _ad_grad_phi_data_face)
    it.second.release();

  for (auto & it : _ad_vector_grad_phi_data_face)
    it.second.release();

  delete _current_side_elem;
  delete _current_neighbor_side_elem;

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

void
Assembly::buildFE(FEType type) const
{
  if (!_fe_shape_data[type])
    _fe_shape_data[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe[dim][type])
      _const_fe[dim][type] = _fe[dim][type] = FEGenericBase<Real>::build(dim, type).release();

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
    _fe_shape_data_face[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe_face[dim][type])
      _const_fe_face[dim][type] = _fe_face[dim][type] =
          FEGenericBase<Real>::build(dim, type).release();

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
    _fe_shape_data_neighbor[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe_neighbor[dim][type])
      _const_fe_neighbor[dim][type] = _fe_neighbor[dim][type] =
          FEGenericBase<Real>::build(dim, type).release();

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
    _fe_shape_data_face_neighbor[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
  {
    if (!_fe_face_neighbor[dim][type])
      _const_fe_face_neighbor[dim][type] = _fe_face_neighbor[dim][type] =
          FEGenericBase<Real>::build(dim, type).release();

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
    _fe_shape_data_lower[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of
  // the current mesh minus one (because this is for lower-dimensional
  // elements!)
  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
  {
    if (!_fe_lower[dim][type])
      _const_fe_lower[dim][type] = _fe_lower[dim][type] =
          FEGenericBase<Real>::build(dim, type).release();

    _fe_lower[dim][type]->get_phi();
    _fe_lower[dim][type]->get_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _fe_lower[dim][type]->get_d2phi();
  }
}

void
Assembly::buildVectorLowerDFE(FEType type) const
{
  if (!_vector_fe_shape_data_lower[type])
    _vector_fe_shape_data_lower[type] = new VectorFEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of
  // the current mesh minus one (because this is for lower-dimensional
  // elements!)
  for (unsigned int dim = 0; dim <= _mesh_dimension - 1; dim++)
  {
    if (!_vector_fe_lower[dim][type])
      _const_vector_fe_lower[dim][type] = _vector_fe_lower[dim][type] =
          FEVectorBase::build(dim, type).release();

    _vector_fe_lower[dim][type]->get_phi();
    _vector_fe_lower[dim][type]->get_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _vector_fe_lower[dim][type]->get_d2phi();
  }
}

void
Assembly::buildVectorFE(FEType type) const
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
      _const_vector_fe[dim][type] = _vector_fe[dim][type] =
          FEGenericBase<VectorValue<Real>>::build(dim, type).release();

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
      _const_vector_fe_face[dim][type] = _vector_fe_face[dim][type] =
          FEGenericBase<VectorValue<Real>>::build(dim, type).release();

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
      _const_vector_fe_neighbor[dim][type] = _vector_fe_neighbor[dim][type] =
          FEGenericBase<VectorValue<Real>>::build(dim, type).release();

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
      _const_vector_fe_face_neighbor[dim][type] = _vector_fe_face_neighbor[dim][type] =
          FEGenericBase<VectorValue<Real>>::build(dim, type).release();

    _vector_fe_face_neighbor[dim][type]->get_phi();
    _vector_fe_face_neighbor[dim][type]->get_dphi();
    if (type.family == NEDELEC_ONE)
      _vector_fe_face_neighbor[dim][type]->get_curl_phi();
  }
}

void
Assembly::createQRules(QuadratureType type, Order order, Order volume_order, Order face_order)
{
  for (auto & it : _holder_qrule_volume)
    delete it.second;
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_volume[dim] = QBase::build(type, dim, volume_order).release();

  for (auto & it : _holder_qrule_face)
    delete it.second;
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_face[dim] = QBase::build(type, dim - 1, face_order).release();

  for (auto & it : _holder_qrule_neighbor)
    delete it.second;
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_neighbor[dim] = new ArbitraryQuadrature(dim - 1, face_order);

  for (auto & it : _holder_qrule_arbitrary)
    delete it.second;
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_arbitrary[dim] = new ArbitraryQuadrature(dim, order);

  for (auto & it : _holder_qrule_arbitrary_face)
    delete it.second;
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    _holder_qrule_arbitrary_face[dim] = new ArbitraryQuadrature(dim - 1, face_order);

  delete _qrule_msm;
  _const_qrule_msm = _qrule_msm = QBase::build(type, _mesh_dimension - 1, face_order).release();
  _fe_msm->attach_quadrature_rule(_qrule_msm);
}

void
Assembly::setVolumeQRule(QBase * qrule, unsigned int dim)
{
  _const_current_qrule = _current_qrule = qrule;

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
  _const_current_qrule_face = _current_qrule_face = qrule;

  for (auto & it : _fe_face[dim])
    it.second->attach_quadrature_rule(qrule);
  for (auto & it : _vector_fe_face[dim])
    it.second->attach_quadrature_rule(qrule);
}

void
Assembly::setNeighborQRule(QBase * qrule, unsigned int dim)
{
  _const_current_qrule_neighbor = _current_qrule_neighbor = qrule;

  for (auto & it : _fe_face_neighbor[dim])
    it.second->attach_quadrature_rule(qrule);
  for (auto & it : _vector_fe_face_neighbor[dim])
    it.second->attach_quadrature_rule(qrule);
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

  if (_computing_jacobian && _subproblem.haveADObjects())
  {
    auto n_qp = _current_qrule->n_points();
    resizeADMappingObjects(n_qp, dim);
    if (_displaced)
    {
      const auto & qw = _current_qrule->get_weights();
      if (elem->has_affine_map())
        computeAffineMapAD(elem, qw, n_qp, *_holder_fe_helper[dim]);
      else
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
      FEBase * fe = it.second;
      auto fe_type = it.first;
      auto num_shapes = fe->n_shape_functions();
      auto & grad_phi = _ad_grad_phi_data[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      if (_displaced)
        computeGradPhiAD(elem, n_qp, grad_phi, fe);
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
      FEVectorBase * fe = it.second;
      auto fe_type = it.first;
      auto num_shapes = fe->n_shape_functions();
      auto & grad_phi = _ad_vector_grad_phi_data[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      if (_displaced)
        computeGradPhiAD(elem, n_qp, grad_phi, fe);
      else
      {
        const auto & regular_grad_phi = _vector_fe_shape_data[fe_type]->_grad_phi;
        for (unsigned qp = 0; qp < n_qp; ++qp)
          for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
            grad_phi[i][qp] = regular_grad_phi[i][qp];
      }
    }
  }

  if (_xfem != nullptr)
    modifyWeightsDueToXFEM(elem);
}

template <typename OutputType>
void
Assembly::computeGradPhiAD(
    const Elem * elem,
    unsigned int n_qp,
    typename VariableTestGradientType<OutputType, ComputeStage::JACOBIAN>::type & grad_phi,
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
          grad_phi[i][qp](0) = dphidxi[i][qp] * _ad_dxidx_map[qp];
          grad_phi[i][qp](1) = dphidxi[i][qp] * _ad_dxidy_map[qp];
          grad_phi[i][qp](2) = dphidxi[i][qp] * _ad_dxidz_map[qp];
        }
      break;
    }

    case 2:
    {
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        for (unsigned qp = 0; qp < n_qp; ++qp)
        {
          grad_phi[i][qp](0) =
              dphidxi[i][qp] * _ad_dxidx_map[qp] + dphideta[i][qp] * _ad_detadx_map[qp];
          grad_phi[i][qp](1) =
              dphidxi[i][qp] * _ad_dxidy_map[qp] + dphideta[i][qp] * _ad_detady_map[qp];
          grad_phi[i][qp](2) =
              dphidxi[i][qp] * _ad_dxidz_map[qp] + dphideta[i][qp] * _ad_detadz_map[qp];
        }
      break;
    }

    case 3:
    {
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        for (unsigned qp = 0; qp < n_qp; ++qp)
        {
          grad_phi[i][qp](0) = dphidxi[i][qp] * _ad_dxidx_map[qp] +
                               dphideta[i][qp] * _ad_detadx_map[qp] +
                               dphidzeta[i][qp] * _ad_dzetadx_map[qp];
          grad_phi[i][qp](1) = dphidxi[i][qp] * _ad_dxidy_map[qp] +
                               dphideta[i][qp] * _ad_detady_map[qp] +
                               dphidzeta[i][qp] * _ad_dzetady_map[qp];
          grad_phi[i][qp](2) = dphidxi[i][qp] * _ad_dxidz_map[qp] +
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
Assembly::computeAffineMapAD(const Elem * elem,
                             const std::vector<Real> & qw,
                             unsigned int n_qp,
                             FEBase * fe)
{
  computeSinglePointMapAD(elem, qw, 0, fe);

  for (unsigned int p = 1; p < n_qp; p++)
  {
    // Compute xyz at all other quadrature points. Note that this map method call is only really
    // referring to the volumetric element...face quadrature points are calculated in computeFaceMap
    if (_calculate_xyz)
    {
      auto num_shapes = fe->n_shape_functions();
      const auto & elem_nodes = elem->get_nodes();
      const auto & phi_map = fe->get_fe_map().get_phi_map();
      _ad_q_points[p].zero();
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
      {
        mooseAssert(elem_nodes[i], "The node is null!");
        VectorValue<DualReal> elem_point = *elem_nodes[i];
        unsigned dimension = 0;
        for (const auto & disp_num : _displacements)
          Moose::derivInsert(elem_point(dimension++).derivatives(),
                             disp_num * _sys.getMaxVarNDofsPerElem() + i,
                             1.);

        _ad_q_points[p].add_scaled(elem_point, phi_map[i][p]);
      }
    }

    // Now copy over other map data for each extra quadrature point

    _ad_dxyzdxi_map[p] = _ad_dxyzdxi_map[0];
    _ad_dxidx_map[p] = _ad_dxidx_map[0];
    _ad_dxidy_map[p] = _ad_dxidy_map[0];
    _ad_dxidz_map[p] = _ad_dxidz_map[0];

    if (elem->dim() > 1)
    {
      _ad_dxyzdeta_map[p] = _ad_dxyzdeta_map[0];
      _ad_detadx_map[p] = _ad_detadx_map[0];
      _ad_detady_map[p] = _ad_detady_map[0];
      _ad_detadz_map[p] = _ad_detadz_map[0];

      if (elem->dim() > 2)
      {
        _ad_dxyzdzeta_map[p] = _ad_dxyzdzeta_map[0];
        _ad_dzetadx_map[p] = _ad_dzetadx_map[0];
        _ad_dzetady_map[p] = _ad_dzetady_map[0];
        _ad_dzetadz_map[p] = _ad_dzetadz_map[0];
      }
    }
    _ad_jac[p] = _ad_jac[0];
    _ad_JxW[p] = _ad_JxW[0] / qw[0] * qw[p];
  }
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
        libMesh::VectorValue<DualReal> elem_point = *elem_nodes[i];
        unsigned dimension = 0;
        for (const auto & disp_num : _displacements)
          Moose::derivInsert(elem_point(dimension++).derivatives(),
                             disp_num * _sys.getMaxVarNDofsPerElem() + i,
                             1.);

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
        libMesh::VectorValue<DualReal> elem_point = *elem_nodes[i];
        unsigned dimension = 0;
        for (const auto & disp_num : _displacements)
          Moose::derivInsert(elem_point(dimension++).derivatives(),
                             disp_num * _sys.getMaxVarNDofsPerElem() + i,
                             1.);

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
        libMesh::VectorValue<DualReal> elem_point = *elem_nodes[i];
        unsigned dimension = 0;
        for (const auto & disp_num : _displacements)
          Moose::derivInsert(elem_point(dimension++).derivatives(),
                             disp_num * _sys.getMaxVarNDofsPerElem() + i,
                             1.);

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

  _mapped_normals.resize(_current_normals.size(), Eigen::Map<RealDIMValue>(nullptr));
  for (unsigned int i = 0; i < _current_normals.size(); i++)
    // Note: this does NOT do any allocation.  It is "reconstructing" the object in place
    new (&_mapped_normals[i]) Eigen::Map<RealDIMValue>(const_cast<Real *>(&_current_normals[i](0)));

  if (_calculate_curvatures)
    _curvatures.shallowCopy(
        const_cast<std::vector<Real> &>((*_holder_fe_face_helper[dim])->get_curvatures()));

  computeADFace(elem, side);

  if (_xfem != nullptr)
    modifyFaceWeightsDueToXFEM(elem, side);
}

void
Assembly::computeFaceMap(unsigned dim, const std::vector<Real> & qw, const Elem * side)
{
  // Important quantities calculated by this method:
  //   - _ad_JxW_face
  //   - _ad_q_points_face
  //   - _ad_normals
  //   - _ad_curvatures

  const auto n_qp = qw.size();
  const Elem * elem = side->parent();
  auto side_number = elem->which_side_am_i(side);
  const auto & dpsidxi_map = (*_holder_fe_face_helper[dim])->get_fe_map().get_dpsidxi();
  const auto & dpsideta_map = (*_holder_fe_face_helper[dim])->get_fe_map().get_dpsideta();
  const auto & psi_map = (*_holder_fe_face_helper[dim])->get_fe_map().get_psi();
  std::vector<std::vector<Real>> const * d2psidxi2_map = nullptr;
  std::vector<std::vector<Real>> const * d2psidxideta_map = nullptr;
  std::vector<std::vector<Real>> const * d2psideta2_map = nullptr;
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

      if (side->node_id(0) == elem->node_id(0))
        _ad_normals[0] = Point(-1.);
      else
        _ad_normals[0] = Point(1.);

      VectorValue<DualReal> side_point;
      if (_calculate_face_xyz)
      {
        side_point = side->point(0);
        auto element_node_number = elem->which_node_am_i(side_number, 0);

        unsigned dimension = 0;
        for (const auto & disp_num : _displacements)
          Moose::derivInsert(side_point(dimension++).derivatives(),
                             disp_num * _sys.getMaxVarNDofsPerElem() + element_node_number,
                             1.);
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
          FE<2, LAGRANGE>::n_shape_functions(side->type(), side->default_order());

      for (unsigned int i = 0; i < n_mapping_shape_functions; i++)
      {
        VectorValue<DualReal> side_point = side->point(i);
        auto element_node_number = elem->which_node_am_i(side_number, i);

        unsigned dimension = 0;
        for (const auto & disp_num : _displacements)
          Moose::derivInsert(side_point(dimension++).derivatives(),
                             disp_num * _sys.getMaxVarNDofsPerElem() + element_node_number,
                             1.);

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
          FE<3, LAGRANGE>::n_shape_functions(side->type(), side->default_order());

      for (unsigned int i = 0; i < n_mapping_shape_functions; i++)
      {
        VectorValue<DualReal> side_point = side->point(i);
        auto element_node_number = elem->which_node_am_i(side_number, i);

        unsigned dimension = 0;
        for (const auto & disp_num : _displacements)
          Moose::derivInsert(side_point(dimension++).derivatives(),
                             disp_num * _sys.getMaxVarNDofsPerElem() + element_node_number,
                             1.);

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

    const std::vector<Real> & JxW = fe->get_JxW();
    MooseArray<Point> q_points;
    q_points.shallowCopy(const_cast<std::vector<Point> &>(fe->get_xyz()));

    setCoordinateTransformation<RESIDUAL>(
        qrule, q_points, _coord_neighbor, _current_neighbor_subdomain_id);

    _current_neighbor_volume = 0.;
    for (unsigned int qp = 0; qp < qrule->n_points(); qp++)
      _current_neighbor_volume += JxW[qp] * _coord_neighbor[qp];
  }
}

template <ComputeStage compute_stage>
void
Assembly::setCoordinateTransformation(const QBase * qrule,
                                      const ADPoint & q_points,
                                      MooseArray<ADReal> & coord,
                                      SubdomainID sub_id)
{
  mooseAssert(qrule, "The quadrature rule is null in Assembly::setCoordinateTransformation");
  auto n_points = qrule->n_points();
  mooseAssert(n_points == q_points.size(),
              "The number of points in the quadrature rule doesn't match the number of passed-in "
              "points in Assembly::setCoordinateTransformation");

  coord.resize(n_points);
  _coord_type = _subproblem.getCoordSystem(sub_id);
  unsigned int rz_radial_coord = _subproblem.getAxisymmetricRadialCoord();

  switch (_coord_type)
  {
    case Moose::COORD_XYZ:
      for (unsigned int qp = 0; qp < n_points; qp++)
        coord[qp] = 1.;
      break;

    case Moose::COORD_RZ:
      for (unsigned int qp = 0; qp < n_points; qp++)
        coord[qp] = 2 * M_PI * q_points[qp](rz_radial_coord);
      break;

    case Moose::COORD_RSPHERICAL:
      for (unsigned int qp = 0; qp < n_points; qp++)
        coord[qp] = 4 * M_PI * q_points[qp](0) * q_points[qp](0);
      break;

    default:
      mooseError("Unknown coordinate system");
      break;
  }
}

template void Assembly::setCoordinateTransformation<ComputeStage::RESIDUAL>(
    const QBase *, const MooseArray<Point> &, MooseArray<Real> &, SubdomainID);
template void Assembly::setCoordinateTransformation<ComputeStage::JACOBIAN>(
    const QBase *, const MooseArray<VectorValue<DualReal>> &, MooseArray<DualReal> &, SubdomainID);

void
Assembly::computeCurrentElemVolume()
{
  if (_current_elem_volume_computed)
    return;

  setCoordinateTransformation<ComputeStage::RESIDUAL>(
      _current_qrule, _current_q_points, _coord, _current_elem->subdomain_id());
  if (_computing_jacobian && _calculate_xyz)
    setCoordinateTransformation<ComputeStage::JACOBIAN>(
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

  setCoordinateTransformation<ComputeStage::RESIDUAL>(
      _current_qrule_face, _current_q_points_face, _coord, _current_elem->subdomain_id());
  if (_computing_jacobian && _calculate_face_xyz)
    setCoordinateTransformation<ComputeStage::JACOBIAN>(
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

  _current_qrule_volume = _holder_qrule_volume[elem_dimension];

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

  // Make sure the qrule is the right one
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

  _current_qrule_arbitrary_face = _holder_qrule_arbitrary_face[elem_dimension];

  // Make sure the qrule is the right one
  if (_current_qrule_face != _current_qrule_arbitrary_face)
    setFaceQRule(_current_qrule_arbitrary_face, elem_dimension);

  _current_qrule_arbitrary->setPoints(reference_points);

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
    ArbitraryQuadrature * face_rule = _holder_qrule_arbitrary_face[elem_dim];
    face_rule->setPoints(*pts);
    setFaceQRule(face_rule, elem_dim);
  }
  else
  {
    if (_current_qrule_face != _holder_qrule_face[elem_dim])
    {
      _current_qrule_face = _holder_qrule_face[elem_dim];
      setFaceQRule(_current_qrule_face, elem_dim);
    }
  }

  // reinit face
  for (const auto & it : _fe_face[elem_dim])
  {
    FEBase * fe_face = it.second;
    FEType fe_type = it.first;
    FEShapeData * fesd = _fe_shape_data_face[fe_type];

    fe_face->reinit(elem, elem_side, tolerance, pts, weights);

    _current_fe_face[fe_type] = fe_face;

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_face->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_face->get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_face->get_d2phi()));
  }
  for (const auto & it : _vector_fe_face[elem_dim])
  {
    FEVectorBase * fe_face = it.second;
    const FEType & fe_type = it.first;

    _current_vector_fe_face[fe_type] = fe_face;

    VectorFEShapeData * fesd = _vector_fe_shape_data_face[fe_type];

    fe_face->reinit(elem, elem_side, tolerance, pts, weights);

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

  computeADFace(elem, elem_side);
}

void
Assembly::computeADFace(const Elem * elem, unsigned int side)
{
  auto dim = elem->dim();

  if (_computing_jacobian && _subproblem.haveADObjects())
  {
    const std::unique_ptr<const Elem> side_elem(elem->build_side_ptr(side));

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
      computeFaceMap(dim, qw, side_elem.get());
      std::vector<Real> dummy_qw(n_qp, 1.);

      if (elem->has_affine_map())
        computeAffineMapAD(elem, dummy_qw, n_qp, *_holder_fe_face_helper[dim]);
      else
        for (unsigned int qp = 0; qp != n_qp; qp++)
          computeSinglePointMapAD(elem, dummy_qw, qp, *_holder_fe_face_helper[dim]);
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
      FEBase * fe = it.second;
      auto fe_type = it.first;
      auto num_shapes = fe->n_shape_functions();
      auto & grad_phi = _ad_grad_phi_data_face[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      const auto & regular_grad_phi = _fe_shape_data_face[fe_type]->_grad_phi;

      if (_displaced)
        computeGradPhiAD(elem, n_qp, grad_phi, fe);
      else
        for (unsigned qp = 0; qp < n_qp; ++qp)
          for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
            grad_phi[i][qp] = regular_grad_phi[i][qp];
    }
    for (const auto & it : _vector_fe_face[dim])
    {
      FEVectorBase * fe = it.second;
      auto fe_type = it.first;
      auto num_shapes = fe->n_shape_functions();
      auto & grad_phi = _ad_vector_grad_phi_data_face[fe_type];

      grad_phi.resize(num_shapes);
      for (decltype(num_shapes) i = 0; i < num_shapes; ++i)
        grad_phi[i].resize(n_qp);

      const auto & regular_grad_phi = _vector_fe_shape_data_face[fe_type]->_grad_phi;

      if (_displaced)
        computeGradPhiAD(elem, n_qp, grad_phi, fe);
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

  // _holder_qrule_neighbor really does contain pointers to ArbitraryQuadrature
  ArbitraryQuadrature * neighbor_rule = _holder_qrule_neighbor[neighbor_dim];
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
    FEBase * fe_face_neighbor = it.second;
    FEType fe_type = it.first;
    FEShapeData * fesd = _fe_shape_data_face_neighbor[fe_type];

    fe_face_neighbor->reinit(neighbor, neighbor_side, tolerance, pts, weights);

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

    fe_face_neighbor->reinit(neighbor, neighbor_side, tolerance, pts, weights);

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
  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points from it
  _current_q_points_face_neighbor.shallowCopy(const_cast<std::vector<Point> &>(
      (*_holder_fe_face_neighbor_helper[neighbor_dim])->get_xyz()));
}

void
Assembly::reinitLowerDElemRef(const Elem * elem,
                              const std::vector<Point> * const pts,
                              const std::vector<Real> * const weights)
{
  mooseAssert(pts->size(),
              "Currently reinitialization of lower d elements is only supported with custom "
              "quadrature points; there is no fall-back quadrature rule. Consequently make sure "
              "you never try to use JxW coming from a fe_lower object unless you are also passing "
              "a weights argument");

  _current_lower_d_elem = elem;

  unsigned int elem_dim = elem->dim();

  for (const auto & it : _fe_lower[elem_dim])
  {
    FEBase * fe_lower = it.second;
    FEType fe_type = it.first;
    FEShapeData * fesd = _fe_shape_data_lower[fe_type];

    fe_lower->reinit(elem, pts, weights);

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real>> &>(fe_lower->get_phi()));
    fesd->_grad_phi.shallowCopy(
        const_cast<std::vector<std::vector<RealGradient>> &>(fe_lower->get_dphi()));
    if (_need_second_derivative_neighbor.find(fe_type) != _need_second_derivative_neighbor.end())
      fesd->_second_phi.shallowCopy(
          const_cast<std::vector<std::vector<TensorValue<Real>>> &>(fe_lower->get_d2phi()));
  }

  MooseArray<Point> array_q_points;
  array_q_points.shallowCopy(const_cast<std::vector<Point> &>(*pts));
  setCoordinateTransformation<RESIDUAL>(
      _qrule_msm, array_q_points, _coord_msm, _current_lower_d_elem->subdomain_id());
}

void
Assembly::reinitMortarElem(const Elem * elem)
{
  mooseAssert(elem->dim() == _mesh_dimension - 1,
              "You should be calling reinitMortarElem on a lower dimensional element");

  _fe_msm->reinit(elem);
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

  auto num_vector_tags = _subproblem.numVectorTags();
  auto num_matrix_tags = _subproblem.numMatrixTags();

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
      }
      _jacobian_block_used[tag][i].resize(n_vars);
      _jacobian_block_neighbor_used[tag][i].resize(n_vars);
      _jacobian_block_lower_used[tag][i].resize(n_vars);
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
      jacobianBlock(vi, vj, tag)
          .resize(ivar.dofIndices().size() * ivar.count(), jvar.dofIndices().size() * jcount);
      _jacobian_block_used[tag][vi][vj] = 0;
    }
  }
}

void
Assembly::prepareResidual()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (MooseIndex(_sub_Re) tag = 0; tag < _sub_Re.size(); tag++)
      _sub_Re[tag][var->number()].resize(var->dofIndices().size() * var->count());
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
      jacobianBlockNonlocal(vi, vj, tag)
          .resize(ivar.dofIndices().size() * ivar.count(), jvar.allDofIndices().size() * jcount);
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

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    if (vi == var->number() || vj == var->number())
    {
      for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(vi, vj, tag)
            .resize(ivar.dofIndices().size() * ivar.count(), jvar.dofIndices().size() * jcount);
        _jacobian_block_used[tag][vi][vj] = 0;
      }
    }
  }

  for (MooseIndex(_sub_Re) tag = 0; tag < _sub_Re.size(); tag++)
    _sub_Re[tag][var->number()].resize(var->dofIndices().size() * var->count());
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
        jacobianBlockNonlocal(vi, vj, tag)
            .resize(ivar.dofIndices().size() * ivar.count(), jvar.allDofIndices().size() * jcount);
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

    unsigned int jcount = (vi == vj && _component_block_diagonal[vi]) ? 1 : jvar.count();

    for (MooseIndex(_jacobian_block_neighbor_used) tag = 0;
         tag < _jacobian_block_neighbor_used.size();
         tag++)
    {
      jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj, tag)
          .resize(ivar.dofIndices().size() * ivar.count(),
                  jvar.dofIndicesNeighbor().size() * jcount);

      jacobianBlockNeighbor(Moose::NeighborElement, vi, vj, tag)
          .resize(ivar.dofIndicesNeighbor().size() * ivar.count(),
                  jvar.dofIndices().size() * jcount);

      jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj, tag)
          .resize(ivar.dofIndicesNeighbor().size() * ivar.count(),
                  jvar.dofIndicesNeighbor().size() * jcount);

      _jacobian_block_neighbor_used[tag][vi][vj] = 0;
    }
  }

  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (MooseIndex(_sub_Rn) tag = 0; tag < _sub_Rn.size(); tag++)
      _sub_Rn[tag][var->number()].resize(var->dofIndicesNeighbor().size() * var->count());
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
      // Lower,Slave,Master. However, 4 cases will in general be covered by calls to prepare() and
      // prepareNeighbor(). These calls will cover SlaveSlave (ElementElement), SlaveMaster
      // (ElementNeighbor), MasterSlave (NeighborElement), and MasterMaster (NeighborNeighbor). With
      // these covered we only need to prepare the 5 remaining below

      // derivatives w.r.t. lower dimensional residuals
      jacobianBlockLower(Moose::LowerLower, vi, vj, tag)
          .resize(ivar.dofIndicesLower().size() * ivar.count(),
                  jvar.dofIndicesLower().size() * jcount);

      jacobianBlockLower(Moose::LowerSlave, vi, vj, tag)
          .resize(ivar.dofIndicesLower().size() * ivar.count(),
                  jvar.dofIndices().size() * jvar.count());

      jacobianBlockLower(Moose::LowerMaster, vi, vj, tag)
          .resize(ivar.dofIndicesLower().size() * ivar.count(),
                  jvar.dofIndicesNeighbor().size() * jvar.count());

      // derivatives w.r.t. interior slave residuals
      jacobianBlockLower(Moose::SlaveLower, vi, vj, tag)
          .resize(ivar.dofIndices().size() * ivar.count(),
                  jvar.dofIndicesLower().size() * jvar.count());

      // derivatives w.r.t. interior master residuals
      jacobianBlockLower(Moose::MasterLower, vi, vj, tag)
          .resize(ivar.dofIndicesNeighbor().size() * ivar.count(),
                  jvar.dofIndicesLower().size() * jvar.count());

      _jacobian_block_lower_used[tag][vi][vj] = 0;
    }
  }

  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (MooseIndex(_sub_Rl) tag = 0; tag < _sub_Rl.size(); tag++)
      _sub_Rl[tag][var->number()].resize(var->dofIndicesLower().size() * var->count());
}

void
Assembly::prepareBlock(unsigned int ivar,
                       unsigned int jvar,
                       const std::vector<dof_id_type> & dof_indices)
{
  unsigned int icount = _sys.getVariable(_tid, ivar).count();
  unsigned int jcount = _sys.getVariable(_tid, jvar).count();
  if (ivar == jvar && _component_block_diagonal[ivar])
    jcount = 1;

  for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
  {
    jacobianBlock(ivar, jvar, tag).resize(dof_indices.size() * icount, dof_indices.size() * jcount);
    _jacobian_block_used[tag][ivar][jvar] = 0;
  }

  for (MooseIndex(_sub_Re) tag = 0; tag < _sub_Re.size(); tag++)
    _sub_Re[tag][ivar].resize(dof_indices.size() * icount);
}

void
Assembly::prepareBlockNonlocal(unsigned int ivar,
                               unsigned int jvar,
                               const std::vector<dof_id_type> & idof_indices,
                               const std::vector<dof_id_type> & jdof_indices)
{
  unsigned int icount = _sys.getVariable(_tid, ivar).count();
  unsigned int jcount = _sys.getVariable(_tid, jvar).count();
  if (ivar == jvar && _component_block_diagonal[ivar])
    jcount = 1;

  for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
       tag < _jacobian_block_nonlocal_used.size();
       tag++)
  {
    jacobianBlockNonlocal(ivar, jvar, tag)
        .resize(idof_indices.size() * icount, jdof_indices.size() * jcount);
    _jacobian_block_nonlocal_used[tag][ivar][jvar] = 0;
  }
}

void
Assembly::prepareScalar()
{
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (const auto & ivar : vars)
  {
    auto idofs = ivar->dofIndices().size();

    for (MooseIndex(_sub_Re) tag = 0; tag < _sub_Re.size(); tag++)
      _sub_Re[tag][ivar->number()].resize(idofs);

    for (const auto & jvar : vars)
    {
      auto jdofs = jvar->dofIndices().size();

      for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(ivar->number(), jvar->number(), tag).resize(idofs, jdofs);
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
    auto idofs = ivar->dofIndices().size();

    for (const auto & jvar : vars)
    {
      auto jdofs = jvar->dofIndices().size() * jvar->count();
      for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
      {
        jacobianBlock(ivar->number(), jvar->number(), tag).resize(idofs, jdofs);
        _jacobian_block_used[tag][ivar->number()][jvar->number()] = 0;

        jacobianBlock(jvar->number(), ivar->number(), tag).resize(jdofs, idofs);
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
  MooseVariableFEBase & v = _sys.getVariable(_tid, var);
  if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    MooseVariable & v = _sys.getFieldVariable<Real>(_tid, var);
    copyShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    ArrayMooseVariable & v = _sys.getFieldVariable<RealEigenVector>(_tid, var);
    copyShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_VECTOR)
  {
    VectorMooseVariable & v = _sys.getFieldVariable<RealVectorValue>(_tid, var);
    copyShapes(v);
    if (v.computingCurl())
      curlPhi(v).shallowCopy(v.curlPhi());
  }
  else
    mooseError("Unsupported variable field type!");
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
  MooseVariableFEBase & v = _sys.getVariable(_tid, var);
  if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    MooseVariable & v = _sys.getFieldVariable<Real>(_tid, var);
    copyFaceShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    ArrayMooseVariable & v = _sys.getFieldVariable<RealEigenVector>(_tid, var);
    copyFaceShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_VECTOR)
  {
    VectorMooseVariable & v = _sys.getFieldVariable<RealVectorValue>(_tid, var);
    copyFaceShapes(v);
    if (v.computingCurl())
      _vector_curl_phi_face.shallowCopy(v.curlPhi());
  }
  else
    mooseError("Unsupported variable field type!");
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
  MooseVariableFEBase & v = _sys.getVariable(_tid, var);
  if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_STANDARD)
  {
    MooseVariable & v = _sys.getFieldVariable<Real>(_tid, var);
    copyNeighborShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_ARRAY)
  {
    ArrayMooseVariable & v = _sys.getFieldVariable<RealEigenVector>(_tid, var);
    copyNeighborShapes(v);
  }
  else if (v.fieldType() == Moose::VarFieldType::VAR_FIELD_VECTOR)
  {
    VectorMooseVariable & v = _sys.getFieldVariable<RealVectorValue>(_tid, var);
    copyNeighborShapes(v);
  }
  else
    mooseError("Unsupported variable field type!");
}

DenseMatrix<Number> &
Assembly::jacobianBlockNeighbor(Moose::DGJacobianType type,
                                unsigned int ivar,
                                unsigned int jvar,
                                TagID tag)
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

DenseMatrix<Number> &
Assembly::jacobianBlockLower(Moose::ConstraintJacobianType type,
                             unsigned int ivar,
                             unsigned int jvar,
                             TagID tag)
{
  _jacobian_block_lower_used[tag][ivar][jvar] = 1;
  if (_block_diagonal_matrix)
  {
    switch (type)
    {
      default:
      case Moose::LowerLower:
        return _sub_Kll[tag][ivar][0];
      case Moose::LowerSlave:
        return _sub_Kle[tag][ivar][0];
      case Moose::LowerMaster:
        return _sub_Kln[tag][ivar][0];
      case Moose::SlaveLower:
        return _sub_Kel[tag][ivar][0];
      case Moose::SlaveSlave:
        return _sub_Kee[tag][ivar][0];
      case Moose::SlaveMaster:
        return _sub_Ken[tag][ivar][0];
      case Moose::MasterLower:
        return _sub_Knl[tag][ivar][0];
      case Moose::MasterSlave:
        return _sub_Kne[tag][ivar][0];
      case Moose::MasterMaster:
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
      case Moose::LowerSlave:
        return _sub_Kle[tag][ivar][jvar];
      case Moose::LowerMaster:
        return _sub_Kln[tag][ivar][jvar];
      case Moose::SlaveLower:
        return _sub_Kel[tag][ivar][jvar];
      case Moose::SlaveSlave:
        return _sub_Kee[tag][ivar][jvar];
      case Moose::SlaveMaster:
        return _sub_Ken[tag][ivar][jvar];
      case Moose::MasterLower:
        return _sub_Knl[tag][ivar][jvar];
      case Moose::MasterSlave:
        return _sub_Kne[tag][ivar][jvar];
      case Moose::MasterMaster:
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
Assembly::addResidual(NumericVector<Number> & residual, TagID tag_id)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual,
                     _sub_Re[tag_id][var->number()],
                     var->dofIndices(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

void
Assembly::addResidual(const std::map<TagName, TagID> & tags)
{
  for (auto & tag : tags)
    if (_sys.hasVector(tag.second))
      addResidual(_sys.getVector(tag.second), tag.second);
}

void
Assembly::addResidualNeighbor(NumericVector<Number> & residual, TagID tag_id)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual,
                     _sub_Rn[tag_id][var->number()],
                     var->dofIndicesNeighbor(),
                     var->arrayScalingFactor(),
                     var->isNodal());
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
                       var->arrayScalingFactor(),
                       false);
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
    for (MooseIndex(_cached_residual_values) tag = 0; tag < _cached_residual_values.size(); tag++)
      if (_sys.hasVector(tag))
        cacheResidualBlock(_cached_residual_values[tag],
                           _cached_residual_rows[tag],
                           _sub_Re[tag][var->number()],
                           var->dofIndices(),
                           var->arrayScalingFactor(),
                           var->isNodal());
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
Assembly::cacheResidualNodes(const DenseVector<Number> & res,
                             const std::vector<dof_id_type> & dof_index,
                             TagID tag)
{
  // Add the residual value and dof_index to cached_residual_values and cached_residual_rows
  // respectively.
  // This is used by NodalConstraint.C to cache the residual calculated for master and slave node.
  for (MooseIndex(dof_index) i = 0; i < dof_index.size(); ++i)
  {
    _cached_residual_values[tag].push_back(res(i));
    _cached_residual_rows[tag].push_back(dof_index[i]);
  }
}

void
Assembly::cacheResidualNeighbor()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
  {
    for (MooseIndex(_cached_residual_values) tag = 0; tag < _cached_residual_values.size(); tag++)
    {
      if (_sys.hasVector(tag))
        cacheResidualBlock(_cached_residual_values[tag],
                           _cached_residual_rows[tag],
                           _sub_Rn[tag][var->number()],
                           var->dofIndicesNeighbor(),
                           var->arrayScalingFactor(),
                           var->isNodal());
    }
  }
}

void
Assembly::cacheResidualLower()
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
  {
    for (MooseIndex(_cached_residual_values) tag = 0; tag < _cached_residual_values.size(); tag++)
    {
      if (_sys.hasVector(tag))
        cacheResidualBlock(_cached_residual_values[tag],
                           _cached_residual_rows[tag],
                           _sub_Rl[tag][var->number()],
                           var->dofIndicesLower(),
                           var->arrayScalingFactor(),
                           var->isNodal());
    }
  }
}

void
Assembly::addCachedResiduals()
{
  for (MooseIndex(_cached_residual_values) tag = 0; tag < _cached_residual_values.size(); tag++)
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
    if (_subproblem.vectorTagExists(tag_id))
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
Assembly::setResidual(NumericVector<Number> & residual, TagID tag_id)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(residual,
                     _sub_Re[tag_id][var->number()],
                     var->dofIndices(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

void
Assembly::setResidualNeighbor(NumericVector<Number> & residual, TagID tag_id)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(residual,
                     _sub_Rn[tag_id][var->number()],
                     var->dofIndicesNeighbor(),
                     var->arrayScalingFactor(),
                     var->isNodal());
}

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

      // If we're computing the initial jacobian for automatically scaling variables we do not want
      // to constrain the element matrix because it introduces 1s on the diagonal for the
      // constrained dofs
      if (!_sys.computingInitialJacobian())
        _dof_map.constrain_element_matrix(sub, di, dj, false);

      jacobian.add_matrix(sub, di, dj);
    }
  }
}

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

      // If we're computing the initial jacobian for automatically scaling variables we do not want
      // to constrain the element matrix because it introduces 1s on the diagonal for the
      // constrained dofs
      if (!_sys.computingInitialJacobian())
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
                             TagID tag)
{
  // Only cache data when the matrix exists
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0) && jac_block.n() && jac_block.m() &&
      _sys.hasMatrix(tag))
  {
    std::vector<dof_id_type> di(idof_indices);
    std::vector<dof_id_type> dj(jdof_indices);

    // If we're computing the initial jacobian for automatically scaling variables we do not want to
    // constrain the element matrix because it introduces 1s on the diagonal for the constrained
    // dofs
    if (!_sys.computingInitialJacobian())
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

void
Assembly::addCachedJacobian(SparseMatrix<Number> & /*jacobian*/)
{
  mooseDeprecated(" Please use addCachedJacobian() ");

  addCachedJacobian();
}

void
Assembly::addCachedJacobian()
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
    if (_jacobian_block_used[tag][i][j] && _sys.hasMatrix(tag))
      addJacobianBlock(_sys.getMatrix(tag),
                       jacobianBlock(i, j, tag),
                       ivar,
                       jvar,
                       ivar.dofIndices(),
                       jvar.dofIndices());
}

void
Assembly::addJacobian()
{
  for (const auto & it : _cm_ff_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_sf_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_fs_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);
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
    for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
      if (_jacobian_block_nonlocal_used[tag][i][j] && _sys.hasMatrix(tag))
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNonlocal(i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->allDofIndices());
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
    for (MooseIndex(_jacobian_block_neighbor_used) tag = 0;
         tag < _jacobian_block_neighbor_used.size();
         tag++)
      if (_jacobian_block_neighbor_used[tag][i][j] && _sys.hasMatrix(tag))
      {
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::ElementNeighbor, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->dofIndicesNeighbor());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborElement, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndices());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockNeighbor(Moose::NeighborNeighbor, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndicesNeighbor());
      }
  }
}

void
Assembly::addJacobianLower()
{
  for (const auto & it : _cm_ff_entry)
  {
    auto ivar = it.first;
    auto jvar = it.second;
    auto i = ivar->number();
    auto j = jvar->number();
    for (MooseIndex(_jacobian_block_lower_used) tag = 0; tag < _jacobian_block_lower_used.size();
         tag++)
      if (_jacobian_block_lower_used[tag][i][j] && _sys.hasMatrix(tag))
      {
        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockLower(Moose::LowerLower, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndicesLower());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockLower(Moose::LowerSlave, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndices());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockLower(Moose::LowerMaster, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesLower(),
                         jvar->dofIndicesNeighbor());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockLower(Moose::SlaveLower, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndices(),
                         jvar->dofIndicesLower());

        addJacobianBlock(_sys.getMatrix(tag),
                         jacobianBlockLower(Moose::MasterLower, i, j, tag),
                         *ivar,
                         *jvar,
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndicesLower());
      }
  }
}

void
Assembly::cacheJacobian()
{
  for (const auto & it : _cm_ff_entry)
    cacheJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_fs_entry)
    cacheJacobianCoupledVarPair(*it.first, *it.second);

  for (const auto & it : _cm_sf_entry)
    cacheJacobianCoupledVarPair(*it.first, *it.second);
}

inline void
Assembly::cacheJacobianCoupledVarPair(const MooseVariableBase & ivar,
                                      const MooseVariableBase & jvar)
{
  auto i = ivar.number();
  auto j = jvar.number();
  for (MooseIndex(_jacobian_block_used) tag = 0; tag < _jacobian_block_used.size(); tag++)
    if (_jacobian_block_used[tag][i][j] && _sys.hasMatrix(tag))
      cacheJacobianBlock(
          jacobianBlock(i, j, tag), ivar, jvar, ivar.dofIndices(), jvar.dofIndices(), tag);
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
    for (MooseIndex(_jacobian_block_nonlocal_used) tag = 0;
         tag < _jacobian_block_nonlocal_used.size();
         tag++)
      if (_jacobian_block_nonlocal_used[tag][i][j] && _sys.hasMatrix(tag))
        cacheJacobianBlockNonzero(jacobianBlockNonlocal(i, j, tag),
                                  *ivar,
                                  *jvar,
                                  ivar->dofIndices(),
                                  jvar->allDofIndices(),
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

    for (MooseIndex(_jacobian_block_neighbor_used) tag = 0;
         tag < _jacobian_block_neighbor_used.size();
         tag++)
      if (_jacobian_block_neighbor_used[tag][i][j] && _sys.hasMatrix(tag))
      {
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::ElementNeighbor, i, j, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndices(),
                           jvar->dofIndicesNeighbor(),
                           tag);
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborElement, i, j, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndices(),
                           tag);
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborNeighbor, i, j, tag),
                           *ivar,
                           *jvar,
                           ivar->dofIndicesNeighbor(),
                           jvar->dofIndicesNeighbor(),
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
  if (dof_indices.size() == 0)
    return;
  if (!(*_cm)(ivar, jvar))
    return;

  DenseMatrix<Number> & ke = jacobianBlock(ivar, jvar);
  auto & iv = _sys.getVariable(_tid, ivar);
  auto & jv = _sys.getVariable(_tid, jvar);
  auto & scaling_factor = iv.arrayScalingFactor();

  // It is guaranteed by design iv.number <= ivar since iv is obtained
  // through SystemBase::getVariable with ivar.
  // Most of times ivar will just be equal to iv.number except for array variables,
  // where ivar could be a number for a component of an array variable but calling
  // getVariable will return the array variable that has the number of the 0th component.
  // It is the same for jvar.
  unsigned int i = ivar - iv.number();
  unsigned int j = jvar - jv.number();

  // DoF indices are independently given
  auto di = dof_indices;
  auto dj = dof_indices;

  auto indof = di.size();
  auto jndof = dj.size();

  unsigned int jj = j;
  if (ivar == jvar && _component_block_diagonal[iv.number()])
    jj = 0;

  auto sub = ke.sub_matrix(i * indof, indof, jj * jndof, jndof);
  // If we're computing the initial jacobian for automatically scaling variables we do not want to
  // constrain the element matrix because it introduces 1s on the diagonal for the constrained
  // dofs
  if (!_sys.computingInitialJacobian())
    dof_map.constrain_element_matrix(sub, di, dj, false);

  if (scaling_factor[i] != 1.0)
    sub *= scaling_factor[i];

  jacobian.add_matrix(sub, di, dj);
}

void
Assembly::addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DofMap & dof_map,
                                   const std::vector<dof_id_type> & idof_indices,
                                   const std::vector<dof_id_type> & jdof_indices)
{
  if (idof_indices.size() == 0 || jdof_indices.size() == 0)
    return;
  if (jacobian.n() == 0 || jacobian.m() == 0)
    return;
  if (!(*_cm)(ivar, jvar))
    return;

  DenseMatrix<Number> & keg = jacobianBlockNonlocal(ivar, jvar);
  auto & iv = _sys.getVariable(_tid, ivar);
  auto & jv = _sys.getVariable(_tid, jvar);
  auto & scaling_factor = iv.arrayScalingFactor();

  // It is guaranteed by design iv.number <= ivar since iv is obtained
  // through SystemBase::getVariable with ivar.
  // Most of times ivar will just be equal to iv.number except for array variables,
  // where ivar could be a number for a component of an array variable but calling
  // getVariable will return the array variable that has the number of the 0th component.
  // It is the same for jvar.
  unsigned int i = ivar - iv.number();
  unsigned int j = jvar - jv.number();

  // DoF indices are independently given
  auto di = idof_indices;
  auto dj = jdof_indices;

  auto indof = di.size();
  auto jndof = dj.size();

  unsigned int jj = j;
  if (ivar == jvar && _component_block_diagonal[iv.number()])
    jj = 0;

  auto sub = keg.sub_matrix(i * indof, indof, jj * jndof, jndof);
  // If we're computing the initial jacobian for automatically scaling variables we do not want to
  // constrain the element matrix because it introduces 1s on the diagonal for the constrained
  // dofs
  if (!_sys.computingInitialJacobian())
    dof_map.constrain_element_matrix(sub, di, dj, false);

  if (scaling_factor[i] != 1.0)
    sub *= scaling_factor[i];

  jacobian.add_matrix(sub, di, dj);
}

void
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian,
                              unsigned int ivar,
                              unsigned int jvar,
                              const DofMap & dof_map,
                              std::vector<dof_id_type> & dof_indices,
                              std::vector<dof_id_type> & neighbor_dof_indices)
{
  if (dof_indices.size() == 0 && neighbor_dof_indices.size() == 0)
    return;
  if (!(*_cm)(ivar, jvar))
    return;

  DenseMatrix<Number> & ken = jacobianBlockNeighbor(Moose::ElementNeighbor, ivar, jvar);
  DenseMatrix<Number> & kne = jacobianBlockNeighbor(Moose::NeighborElement, ivar, jvar);
  DenseMatrix<Number> & knn = jacobianBlockNeighbor(Moose::NeighborNeighbor, ivar, jvar);

  auto & iv = _sys.getVariable(_tid, ivar);
  auto & jv = _sys.getVariable(_tid, jvar);
  auto & scaling_factor = iv.arrayScalingFactor();

  // It is guaranteed by design iv.number <= ivar since iv is obtained
  // through SystemBase::getVariable with ivar.
  // Most of times ivar will just be equal to iv.number except for array variables,
  // where ivar could be a number for a component of an array variable but calling
  // getVariable will return the array variable that has the number of the 0th component.
  // It is the same for jvar.
  unsigned int i = ivar - iv.number();
  unsigned int j = jvar - jv.number();

  // DoF indices are independently given
  auto dc = dof_indices;
  auto dn = neighbor_dof_indices;
  auto cndof = dc.size();
  auto nndof = dn.size();

  unsigned int jj = j;
  if (ivar == jvar && _component_block_diagonal[iv.number()])
    jj = 0;

  auto suben = ken.sub_matrix(i * cndof, cndof, jj * nndof, nndof);
  auto subne = kne.sub_matrix(i * nndof, nndof, jj * cndof, cndof);
  auto subnn = knn.sub_matrix(i * nndof, nndof, jj * nndof, nndof);

  // If we're computing the initial jacobian for automatically scaling variables we do not want to
  // constrain the element matrix because it introduces 1s on the diagonal for the constrained
  // dofs
  if (!_sys.computingInitialJacobian())
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
Assembly::addJacobianScalar()
{
  for (const auto & it : _cm_ss_entry)
    addJacobianCoupledVarPair(*it.first, *it.second);
}

void
Assembly::addJacobianOffDiagScalar(unsigned int ivar)
{
  const std::vector<MooseVariableFEBase *> & vars = _sys.getVariables(_tid);
  MooseVariableScalar & var_i = _sys.getScalarVariable(_tid, ivar);
  for (const auto & var_j : vars)
    addJacobianCoupledVarPair(var_i, *var_j);
}

void
Assembly::cacheJacobianContribution(numeric_index_type i,
                                    numeric_index_type j,
                                    Real value,
                                    TagID tag)
{
  _cached_jacobian_contribution_rows[tag].push_back(i);
  _cached_jacobian_contribution_cols[tag].push_back(j);
  _cached_jacobian_contribution_vals[tag].push_back(value);
}

void
Assembly::cacheJacobianContribution(numeric_index_type i,
                                    numeric_index_type j,
                                    Real value,
                                    const std::set<TagID> & tags)
{
  for (auto tag : tags)
    if (_sys.hasMatrix(tag))
      cacheJacobianContribution(i, j, value, tag);
}

void
Assembly::setCachedJacobianContributions()
{
  for (MooseIndex(_cached_jacobian_contribution_rows) tag = 0;
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
    if (_sys.hasMatrix(tag))
    {
      // First zero the rows (including the diagonals) to prepare for
      // setting the cached values.
      _sys.getMatrix(tag).zero_rows(_cached_jacobian_contribution_rows[tag], 0.0);

      // TODO: Use SparseMatrix::set_values() for efficiency
      for (MooseIndex(_cached_jacobian_contribution_vals) i = 0;
           i < _cached_jacobian_contribution_vals[tag].size();
           ++i)
        _sys.getMatrix(tag).set(_cached_jacobian_contribution_rows[tag][i],
                                _cached_jacobian_contribution_cols[tag][i],
                                _cached_jacobian_contribution_vals[tag][i]);
    }

  clearCachedJacobianContributions();
}

void
Assembly::zeroCachedJacobianContributions()
{
  for (MooseIndex(_cached_jacobian_contribution_rows) tag = 0;
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
    if (_sys.hasMatrix(tag))
      _sys.getMatrix(tag).zero_rows(_cached_jacobian_contribution_rows[tag], 0.0);

  clearCachedJacobianContributions();
}

void
Assembly::addCachedJacobianContributions()
{
  for (MooseIndex(_cached_jacobian_contribution_rows) tag = 0;
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
    if (_sys.hasMatrix(tag))
    {
      // TODO: Use SparseMatrix::set_values() for efficiency
      for (MooseIndex(_cached_jacobian_contribution_vals[tag]) i = 0;
           i < _cached_jacobian_contribution_vals[tag].size();
           ++i)
        _sys.getMatrix(tag).add(_cached_jacobian_contribution_rows[tag][i],
                                _cached_jacobian_contribution_cols[tag][i],
                                _cached_jacobian_contribution_vals[tag][i]);
    }

  clearCachedJacobianContributions();
}

void
Assembly::clearCachedJacobianContributions()
{
  for (MooseIndex(_cached_jacobian_contribution_rows) tag = 0;
       tag < _cached_jacobian_contribution_rows.size();
       tag++)
  {
    auto orig_size = _cached_jacobian_contribution_rows[tag].size();

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
const typename OutputTools<VectorValue<Real>>::VariablePhiGradient &
Assembly::feGradPhiLower<VectorValue<Real>>(FEType type) const
{
  buildVectorFE(type);
  return _vector_fe_shape_data_lower[type]->_grad_phi;
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

template <>
const typename VariableTestGradientType<Real, ComputeStage::JACOBIAN>::type &
Assembly::adGradPhi<Real, ComputeStage::JACOBIAN>(const MooseVariableFE<Real> & v) const
{
  return _ad_grad_phi_data.at(v.feType());
}

template <>
const typename VariableTestGradientType<RealVectorValue, ComputeStage::JACOBIAN>::type &
Assembly::adGradPhi<RealVectorValue, ComputeStage::JACOBIAN>(
    const MooseVariableFE<RealVectorValue> & v) const
{
  return _ad_vector_grad_phi_data.at(v.feType());
}

template <>
const MooseArray<typename Moose::RealType<RESIDUAL>::type> &
Assembly::adJxW<RESIDUAL>() const
{
  return _current_JxW;
}

template <ComputeStage compute_stage>
const MooseArray<ADReal> &
Assembly::adCurvatures() const
{
  _calculate_curvatures = true;
  for (unsigned int dim = 0; dim <= _mesh_dimension; dim++)
    (*_holder_fe_face_helper.at(dim))->get_curvatures();
  return _curvatures;
}

template const MooseArray<typename Moose::RealType<RESIDUAL>::type> &
Assembly::adCurvatures<RESIDUAL>() const;

template void Assembly::computeGradPhiAD<Real>(
    const Elem * elem,
    unsigned int n_qp,
    typename VariableTestGradientType<Real, ComputeStage::JACOBIAN>::type & grad_phi,
    FEGenericBase<Real> * fe);
