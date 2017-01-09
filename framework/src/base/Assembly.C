/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "Assembly.h"

// MOOSE includes
#include "SubProblem.h"
#include "ArbitraryQuadrature.h"
#include "SystemBase.h"
#include "MooseTypes.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "XFEMInterface.h"

// libMesh
#include "libmesh/quadrature_gauss.h"
#include "libmesh/fe_interface.h"
#include "libmesh/dof_map.h"
#include "libmesh/coupling_matrix.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/equation_systems.h"

Assembly::Assembly(SystemBase & sys, THREAD_ID tid) :
    _sys(sys),
    _nonlocal_cm(_sys.subproblem().nonlocalCouplingMatrix()),
    _dof_map(_sys.dofMap()),
    _tid(tid),
    _mesh(sys.mesh()),
    _mesh_dimension(_mesh.dimension()),
    _current_qrule(NULL),
    _current_qrule_volume(NULL),
    _current_qrule_arbitrary(NULL),
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

    _should_use_fe_cache(false),
    _currently_fe_caching(true),

    _cached_residual_values(2), // The 2 is for TIME and NONTIME
    _cached_residual_rows(2), // The 2 is for TIME and NONTIME

    _max_cached_residuals(0),
    _max_cached_jacobians(0),
    _block_diagonal_matrix(false)
{
  // Build fe's for the helpers
  buildFE(FEType(FIRST, LAGRANGE));
  buildFaceFE(FEType(FIRST, LAGRANGE));
  buildNeighborFE(FEType(FIRST, LAGRANGE));
  buildFaceNeighborFE(FEType(FIRST, LAGRANGE));

  // Build an FE helper object for this type for each dimension up to the dimension of the current mesh
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
      _fe[dim][type] = FEBase::build(dim, type).release();
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
      _fe_face[dim][type] = FEBase::build(dim, type).release();
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
      _fe_neighbor[dim][type] = FEBase::build(dim, type).release();
    _fe_neighbor[dim][type]->get_phi();
    _fe_neighbor[dim][type]->get_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
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
      _fe_face_neighbor[dim][type] = FEBase::build(dim, type).release();
    _fe_face_neighbor[dim][type]->get_phi();
    _fe_face_neighbor[dim][type]->get_dphi();
    if (_need_second_derivative.find(type) != _need_second_derivative.end())
      _fe_face_neighbor[dim][type]->get_d2phi();
  }
}

FEBase * &
Assembly::getFE(FEType type, unsigned int dim)
{
  buildFE(type);
  return _fe[dim][type];
}

FEBase * &
Assembly::getFEFace(FEType type, unsigned int dim)
{
  buildFaceFE(type);
  return _fe_face[dim][type];
}

FEBase * &
Assembly::getFEFaceNeighbor(FEType type, unsigned int dim)
{
  buildFaceNeighborFE(type);
  return _fe_neighbor[dim][type];
}

const VariablePhiValue &
Assembly::fePhi(FEType type)
{
  buildFE(type);
  return _fe_shape_data[type]->_phi;
}

const VariablePhiGradient &
Assembly::feGradPhi(FEType type)
{
  buildFE(type);
  return _fe_shape_data[type]->_grad_phi;
}

const VariablePhiSecond &
Assembly::feSecondPhi(FEType type)
{
  _need_second_derivative[type] = true;
  buildFE(type);
  return _fe_shape_data[type]->_second_phi;
}

const VariablePhiValue &
Assembly::fePhiFace(FEType type)
{
  buildFaceFE(type);
  return _fe_shape_data_face[type]->_phi;
}

const VariablePhiGradient &
Assembly::feGradPhiFace(FEType type)
{
  buildFaceFE(type);
  return _fe_shape_data_face[type]->_grad_phi;
}

const VariablePhiSecond &
Assembly::feSecondPhiFace(FEType type)
{
  _need_second_derivative[type] = true;
  buildFaceFE(type);
  return _fe_shape_data_face[type]->_second_phi;
}

const VariablePhiValue &
Assembly::fePhiNeighbor(FEType type)
{
  buildNeighborFE(type);
  return _fe_shape_data_neighbor[type]->_phi;
}

const VariablePhiGradient &
Assembly::feGradPhiNeighbor(FEType type)
{
  buildNeighborFE(type);
  return _fe_shape_data_neighbor[type]->_grad_phi;
}

const VariablePhiSecond &
Assembly::feSecondPhiNeighbor(FEType type)
{
  _need_second_derivative[type] = true;
  buildNeighborFE(type);
  return _fe_shape_data_neighbor[type]->_second_phi;
}

const VariablePhiValue &
Assembly::fePhiFaceNeighbor(FEType type)
{
  buildFaceNeighborFE(type);
  return _fe_shape_data_face_neighbor[type]->_phi;
}

const VariablePhiGradient &
Assembly::feGradPhiFaceNeighbor(FEType type)
{
  buildFaceNeighborFE(type);
  return _fe_shape_data_face_neighbor[type]->_grad_phi;
}

const VariablePhiSecond &
Assembly::feSecondPhiFaceNeighbor(FEType type)
{
  _need_second_derivative[type] = true;
  buildFaceNeighborFE(type);
  return _fe_shape_data_face_neighbor[type]->_second_phi;
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
  }
}

void
Assembly::setFaceQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_face = qrule;

  for (auto & it : _fe_face[dim])
    it.second->attach_quadrature_rule(_current_qrule_face);
}

void
Assembly::setNeighborQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_neighbor = qrule;

  for (auto & it : _fe_face_neighbor[dim])
    it.second->attach_quadrature_rule(_current_qrule_neighbor);
}

void
Assembly::invalidateCache()
{
  for (auto & it : _element_fe_shape_data_cache)
    it.second->_invalidated = true;
}

void
Assembly::reinitFE(const Elem * elem)
{
  unsigned int dim = elem->dim();
  ElementFEShapeData * efesd = NULL;

  // Whether or not we're going to do FE caching this time through
  bool do_caching = _should_use_fe_cache && _currently_fe_caching;

  if (do_caching)
  {
    efesd = _element_fe_shape_data_cache[elem->id()];

    if (!efesd)
    {
      efesd = new ElementFEShapeData;
      _element_fe_shape_data_cache[elem->id()] = efesd;
      efesd->_invalidated = true;
    }
  }

  for (const auto & it : _fe[dim])
  {
    FEBase * fe = it.second;
    const FEType & fe_type = it.first;

    _current_fe[fe_type] = fe;

    FEShapeData * fesd = _fe_shape_data[fe_type];

    FEShapeData * cached_fesd = NULL;
    if (do_caching)
      cached_fesd = efesd->_shape_data[fe_type];

    if (!cached_fesd || efesd->_invalidated)
    {
      fe->reinit(elem);

      fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real> > &>(fe->get_phi()));
      fesd->_grad_phi.shallowCopy(const_cast<std::vector<std::vector<RealGradient> > &>(fe->get_dphi()));
      if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
        fesd->_second_phi.shallowCopy(const_cast<std::vector<std::vector<RealTensor> > &>(fe->get_d2phi()));

      if (do_caching)
      {
        if (!cached_fesd)
        {
          cached_fesd = new FEShapeData;
          efesd->_shape_data[fe_type] = cached_fesd;
        }

        *cached_fesd = *fesd;
      }
    }
    else // This means we have valid cached shape function values for this element / fe_type combo
    {
      fesd->_phi.shallowCopy(cached_fesd->_phi);
      fesd->_grad_phi.shallowCopy(cached_fesd->_grad_phi);
      if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
        fesd->_second_phi.shallowCopy(cached_fesd->_second_phi);
    }
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  if (!do_caching || efesd->_invalidated)
  {
    _current_q_points.shallowCopy(const_cast<std::vector<Point> &>((*_holder_fe_helper[dim])->get_xyz()));
    _current_JxW.shallowCopy(const_cast<std::vector<Real> &>((*_holder_fe_helper[dim])->get_JxW()));

    if (do_caching)
    {
      efesd->_q_points = _current_q_points;
      efesd->_JxW = _current_JxW;
    }
  }
  else // Use cached values
  {
    _current_q_points.shallowCopy(efesd->_q_points);
    _current_JxW.shallowCopy(efesd->_JxW);
  }

  if (do_caching)
    efesd->_invalidated = false;

  if (_xfem != NULL)
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

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real> > &>(fe_face->get_phi()));
    fesd->_grad_phi.shallowCopy(const_cast<std::vector<std::vector<RealGradient> > &>(fe_face->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(const_cast<std::vector<std::vector<RealTensor> > &>(fe_face->get_d2phi()));
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points_face.shallowCopy(const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_xyz()));
  _current_JxW_face.shallowCopy(const_cast<std::vector<Real> &>((*_holder_fe_face_helper[dim])->get_JxW()));
  _current_normals.shallowCopy(const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_normals()));
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

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real> > &>(fe_face_neighbor->get_phi()));
    fesd->_grad_phi.shallowCopy(const_cast<std::vector<std::vector<RealGradient> > &>(fe_face_neighbor->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(const_cast<std::vector<std::vector<RealTensor> > &>(fe_face_neighbor->get_d2phi()));
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

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real> > &>(fe_neighbor->get_phi()));
    fesd->_grad_phi.shallowCopy(const_cast<std::vector<std::vector<RealGradient> > &>(fe_neighbor->get_dphi()));
    if (_need_second_derivative.find(fe_type) != _need_second_derivative.end())
      fesd->_second_phi.shallowCopy(const_cast<std::vector<std::vector<RealTensor> > &>(fe_neighbor->get_d2phi()));
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
    Moose::CoordinateSystemType coord_type = _sys.subproblem().getCoordSystem(_current_neighbor_elem->subdomain_id());
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
  _current_neighbor_elem = NULL;
  _current_elem_volume_computed = false;

  unsigned int elem_dimension = elem->dim();

  _current_qrule_volume = _holder_qrule_volume[elem_dimension];

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_volume)
    setVolumeQRule(_current_qrule_volume, elem_dimension);

  _currently_fe_caching = true;

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
  _current_neighbor_elem = NULL;
  _current_elem_volume_computed = false;

  FEInterface::inverse_map(elem->dim(),
                           (*_holder_fe_helper[elem->dim()])->get_fe_type(),
                           elem,
                           physical_points,
                           _temp_reference_points);

  _currently_fe_caching = false;

  reinit(elem, _temp_reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

void
Assembly::reinit(const Elem * elem, const std::vector<Point> & reference_points)
{
  _current_elem = elem;
  _current_neighbor_elem = NULL;
  _current_elem_volume_computed = false;

  unsigned int elem_dimension = _current_elem->dim();

  _current_qrule_arbitrary = _holder_qrule_arbitrary[elem_dimension];

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_arbitrary)
    setVolumeQRule(_current_qrule_arbitrary, elem_dimension);

  _current_qrule_arbitrary->setPoints(reference_points);

  _currently_fe_caching = false;

  reinitFE(elem);

  computeCurrentElemVolume();
}

void
Assembly::reinit(const Elem * elem, unsigned int side)
{
  _current_elem = elem;
  _current_side = side;
  _current_neighbor_elem = NULL;
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
  _current_side_elem = elem->build_side(side).release();

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
Assembly::reinitNodeNeighbor(const Node * node)
{
  _current_neighbor_node = node;
}

void
Assembly::reinitElemAndNeighbor(const Elem * elem, unsigned int side, const Elem * neighbor, unsigned int neighbor_side)
{
  _current_neighbor_side = neighbor_side;

  reinit(elem, side);

  unsigned int neighbor_dim = neighbor->dim();

  std::vector<Point> reference_points;
  FEInterface::inverse_map(neighbor_dim, FEType(), neighbor, _current_q_points_face.stdVector(), reference_points);

  reinitFEFaceNeighbor(neighbor, reference_points);
  reinitNeighbor(neighbor, reference_points);
}

void
Assembly::reinitNeighborAtPhysical(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points)
{
  delete _current_neighbor_side_elem;
  _current_neighbor_side_elem = neighbor->build_side(neighbor_side).release();

  std::vector<Point> reference_points;

  unsigned int neighbor_dim = neighbor->dim();
  FEInterface::inverse_map(neighbor_dim, FEType(), neighbor, physical_points, reference_points);

  // first do the side element
  reinitFEFaceNeighbor(_current_neighbor_side_elem, reference_points);
  reinitNeighbor(_current_neighbor_side_elem, reference_points);
  // compute JxW on the neighbor's face
  unsigned int neighbor_side_dim = _current_neighbor_side_elem->dim();
  _current_JxW_neighbor.shallowCopy(const_cast<std::vector<Real> &>((*_holder_fe_face_neighbor_helper[neighbor_side_dim])->get_JxW()));

  reinitFEFaceNeighbor(neighbor, reference_points);
  reinitNeighbor(neighbor, reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

void
Assembly::reinitNeighborAtPhysical(const Elem * neighbor, const std::vector<Point> & physical_points)
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
Assembly::jacobianBlock(unsigned int ivar, unsigned int jvar)
{
  _jacobian_block_used[ivar][jvar] = 1;
  return _sub_Kee[ivar][_block_diagonal_matrix ? 0 : jvar];
}

DenseMatrix<Number> &
Assembly::jacobianBlockNonlocal(unsigned int ivar, unsigned int jvar)
{
  _jacobian_block_nonlocal_used[ivar][jvar] = 1;
  return _sub_Keg[ivar][_block_diagonal_matrix ? 0 : jvar];
}

DenseMatrix<Number> &
Assembly::jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar)
{
  _jacobian_block_neighbor_used[ivar][jvar] = 1;
  if (_block_diagonal_matrix)
  {
    switch (type)
    {
    default:
    case Moose::ElementElement: return _sub_Kee[ivar][0];
    case Moose::ElementNeighbor: return _sub_Ken[ivar][0];
    case Moose::NeighborElement: return _sub_Kne[ivar][0];
    case Moose::NeighborNeighbor: return _sub_Knn[ivar][0];
    }
  }
  else
  {
    switch (type)
    {
    default:
    case Moose::ElementElement: return _sub_Kee[ivar][jvar];
    case Moose::ElementNeighbor: return _sub_Ken[ivar][jvar];
    case Moose::NeighborElement: return _sub_Kne[ivar][jvar];
    case Moose::NeighborNeighbor: return _sub_Knn[ivar][jvar];
    }
  }
}

void
Assembly::init(const CouplingMatrix * cm)
{
  _cm = cm;

  unsigned int n_vars = _sys.nVariables();

  // I want the blocks to go by columns first to reduce copying of shape function in assembling "full" Jacobian
  _cm_entry.clear();
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & jvar : vars)
  {
    unsigned int j = jvar->number();
    for (const auto & ivar : vars)
    {
      unsigned int i = ivar->number();
      if ((*_cm)(i, j) != 0)
        _cm_entry.push_back(std::make_pair(ivar, jvar));
    }
  }

  unsigned int max_rows_per_column = 0;  // If this is 1 then we are using a block-diagonal preconditioner and we can save a bunch of memory.
  for (unsigned int i = 0; i < n_vars; i++)
  {
    unsigned int max_rows_per_this_column = 0;
    for (unsigned int j = 0; j < n_vars; j++)
    {
      if ((*_cm)(i, j) != 0)
        max_rows_per_this_column++;
    }
    max_rows_per_column = std::max(max_rows_per_column, max_rows_per_this_column);
  }

  if (max_rows_per_column == 1 && _sys.getScalarVariables(_tid).size() == 0)
    _block_diagonal_matrix = true;

  // two vectors: one for time residual contributions and one for non-time residual contributions
  _sub_Re.resize(2);
  _sub_Rn.resize(2);
  for (unsigned int i = 0; i < _sub_Re.size(); i++)
  {
    _sub_Re[i].resize(n_vars);
    _sub_Rn[i].resize(n_vars);
  }

  _sub_Kee.resize(n_vars);
  _sub_Keg.resize(n_vars);
  _sub_Ken.resize(n_vars);
  _sub_Kne.resize(n_vars);
  _sub_Knn.resize(n_vars);
  _jacobian_block_used.resize(n_vars);
  _jacobian_block_nonlocal_used.resize(n_vars);
  _jacobian_block_neighbor_used.resize(n_vars);

  for (unsigned int i = 0; i < n_vars; ++i)
  {
    if (!_block_diagonal_matrix)
    {
      _sub_Kee[i].resize(n_vars);
      _sub_Keg[i].resize(n_vars);
      _sub_Ken[i].resize(n_vars);
      _sub_Kne[i].resize(n_vars);
      _sub_Knn[i].resize(n_vars);
    }
    else
    {
      _sub_Kee[i].resize(1);
      _sub_Keg[i].resize(1);
      _sub_Ken[i].resize(1);
      _sub_Kne[i].resize(1);
      _sub_Knn[i].resize(1);
    }
    _jacobian_block_used[i].resize(n_vars);
    _jacobian_block_nonlocal_used[i].resize(n_vars);
    _jacobian_block_neighbor_used[i].resize(n_vars);
  }
}

void
Assembly::initNonlocalCoupling()
{
  _cm_nonlocal_entry.clear();
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & jvar : vars)
  {
    unsigned int j = jvar->number();
    for (const auto & ivar : vars)
    {
      unsigned int i = ivar->number();
      if (_nonlocal_cm(i, j) != 0)
        _cm_nonlocal_entry.push_back(std::make_pair(ivar, jvar));
    }
  }
}

void
Assembly::prepare()
{
  for (const auto & it : _cm_entry)
  {
    MooseVariable & ivar = *(it.first);
    MooseVariable & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    jacobianBlock(vi, vj).resize(ivar.dofIndices().size(), jvar.dofIndices().size());
    jacobianBlock(vi, vj).zero();
    _jacobian_block_used[vi][vj] = 0;
  }

  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (unsigned int i = 0; i < _sub_Re.size(); i++)
    {
      _sub_Re[i][var->number()].resize(var->dofIndices().size());
      _sub_Re[i][var->number()].zero();
    }
}

void
Assembly::prepareNonlocal()
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    MooseVariable & ivar = *(it.first);
    MooseVariable & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    jacobianBlockNonlocal(vi,vj).resize(ivar.dofIndices().size(), jvar.allDofIndices().size());
    jacobianBlockNonlocal(vi,vj).zero();
    _jacobian_block_nonlocal_used[vi][vj] = 0;
  }
}

void
Assembly::prepareVariable(MooseVariable * var)
{
  for (const auto & it : _cm_entry)
  {
    MooseVariable & ivar = *(it.first);
    MooseVariable & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    if (vi == var->number() || vj == var->number())
      jacobianBlock(vi,vj).resize(ivar.dofIndices().size(), jvar.dofIndices().size());
  }

  for (unsigned int i = 0; i < _sub_Re.size(); i++)
  {
    _sub_Re[i][var->number()].resize(var->dofIndices().size());
    _sub_Re[i][var->number()].zero();
  }
}

void
Assembly::prepareVariableNonlocal(MooseVariable * var)
{
  for (const auto & it : _cm_nonlocal_entry)
  {
    MooseVariable & ivar = *(it.first);
    MooseVariable & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    if (vi == var->number() || vj == var->number())
      jacobianBlockNonlocal(vi,vj).resize(ivar.dofIndices().size(), jvar.allDofIndices().size());
  }
}

void
Assembly::prepareNeighbor()
{
  for (const auto & it : _cm_entry)
  {
    MooseVariable & ivar = *(it.first);
    MooseVariable & jvar = *(it.second);

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj).resize(ivar.dofIndices().size(), jvar.dofIndicesNeighbor().size());
    jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj).zero();

    jacobianBlockNeighbor(Moose::NeighborElement, vi, vj).resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndices().size());
    jacobianBlockNeighbor(Moose::NeighborElement, vi, vj).zero();

    jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj).resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndicesNeighbor().size());
    jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj).zero();

    _jacobian_block_neighbor_used[vi][vj] = 0;
  }

  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (unsigned int i = 0; i < _sub_Rn.size(); i++)
    {
      _sub_Rn[i][var->number()].resize(var->dofIndicesNeighbor().size());
      _sub_Rn[i][var->number()].zero();
    }
}

void
Assembly::prepareBlock(unsigned int ivar, unsigned int jvar, const std::vector<dof_id_type> & dof_indices)
{
  jacobianBlock(ivar,jvar).resize(dof_indices.size(), dof_indices.size());
  jacobianBlock(ivar,jvar).zero();
  _jacobian_block_used[ivar][jvar] = 0;

  for (unsigned int i = 0; i < _sub_Re.size(); i++)
  {
    _sub_Re[i][ivar].resize(dof_indices.size());
    _sub_Re[i][ivar].zero();
  }
}

void
Assembly::prepareBlockNonlocal(unsigned int ivar, unsigned int jvar, const std::vector<dof_id_type> & idof_indices, const std::vector<dof_id_type> & jdof_indices)
{
  jacobianBlockNonlocal(ivar,jvar).resize(idof_indices.size(), jdof_indices.size());
  jacobianBlockNonlocal(ivar,jvar).zero();
  _jacobian_block_nonlocal_used[ivar][jvar] = 0;
}

void
Assembly::prepareScalar()
{
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (const auto & ivar : vars)
  {
    unsigned int idofs = ivar->dofIndices().size();

    for (unsigned int i = 0; i < _sub_Re.size(); i++)
    {
      _sub_Re[i][ivar->number()].resize(idofs);
      _sub_Re[i][ivar->number()].zero();
    }

    for (const auto & jvar : vars)
    {
      unsigned int jdofs = jvar->dofIndices().size();

      jacobianBlock(ivar->number(), jvar->number()).resize(idofs, jdofs);
      jacobianBlock(ivar->number(), jvar->number()).zero();
      _jacobian_block_used[ivar->number()][jvar->number()] = 0;
    }
  }
}

void
Assembly::prepareOffDiagScalar()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);

  for (const auto & ivar : scalar_vars)
  {
    unsigned int idofs = ivar->dofIndices().size();

    for (const auto & jvar : vars)
    {
      unsigned int jdofs = jvar->dofIndices().size();

      jacobianBlock(ivar->number(), jvar->number()).resize(idofs, jdofs);
      jacobianBlock(ivar->number(), jvar->number()).zero();
      _jacobian_block_used[ivar->number()][jvar->number()] = 0;

      jacobianBlock(jvar->number(), ivar->number()).resize(jdofs, idofs);
      jacobianBlock(jvar->number(), ivar->number()).zero();
      _jacobian_block_used[jvar->number()][ivar->number()] = 0;
    }
  }
}

void
Assembly::copyShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  _phi.shallowCopy(v.phi());
  _grad_phi.shallowCopy(v.gradPhi());

  if (v.computingSecond())
    _second_phi.shallowCopy(v.secondPhi());
}

void
Assembly::copyFaceShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  _phi_face.shallowCopy(v.phiFace());
  _grad_phi_face.shallowCopy(v.gradPhiFace());

  if (v.computingSecond())
    _second_phi_face.shallowCopy(v.secondPhiFace());
}

void
Assembly::copyNeighborShapes(unsigned int var)
{
  MooseVariable & v = _sys.getVariable(_tid, var);

  if (v.usesPhi())
    _phi_face_neighbor.shallowCopy(v.phiFaceNeighbor());
  if (v.usesGradPhi())
    _grad_phi_face_neighbor.shallowCopy(v.gradPhiFaceNeighbor());
  if (v.usesSecondPhi())
    _second_phi_face_neighbor.shallowCopy(v.secondPhiFaceNeighbor());

  if (v.usesPhi())
    _phi_neighbor.shallowCopy(v.phiNeighbor());
  if (v.usesGradPhi())
    _grad_phi_neighbor.shallowCopy(v.gradPhiNeighbor());
  if (v.usesSecondPhi())
    _second_phi_neighbor.shallowCopy(v.secondPhiNeighbor());
}

void
Assembly::addResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, const std::vector<dof_id_type> & dof_indices, Real scaling_factor)
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

      for (unsigned int i=0; i<_tmp_Re.size(); i++)
      {
        cached_residual_values.push_back(_tmp_Re(i));
        cached_residual_rows.push_back(_temp_dof_indices[i]);
      }
    }
    else
    {
      for (unsigned int i=0; i<res_block.size(); i++)
      {
        cached_residual_values.push_back(res_block(i));
        cached_residual_rows.push_back(_temp_dof_indices[i]);
      }
    }
  }

  res_block.zero();
}

void
Assembly::addResidual(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual, _sub_Re[type][var->number()], var->dofIndices(), var->scalingFactor());
}

void
Assembly::addResidualNeighbor(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual, _sub_Rn[type][var->number()], var->dofIndicesNeighbor(), var->scalingFactor());
}

void
Assembly::addResidualScalar(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  // add the scalar variables residuals
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (const auto & var : vars)
    addResidualBlock(residual, _sub_Re[type][var->number()], var->dofIndices(), var->scalingFactor());
}


void
Assembly::cacheResidual()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (unsigned int i = 0; i < _sub_Re.size(); i++)
      cacheResidualBlock(_cached_residual_values[i], _cached_residual_rows[i], _sub_Re[i][var->number()], var->dofIndices(), var->scalingFactor());
}

void
Assembly::cacheResidualContribution(dof_id_type dof, Real value, Moose::KernelType type)
{
  _cached_residual_values[type].push_back(value);
  _cached_residual_rows[type].push_back(dof);
}


void
Assembly::cacheResidualNeighbor()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    for (unsigned int i = 0; i < _sub_Re.size(); i++)
      cacheResidualBlock(_cached_residual_values[i], _cached_residual_rows[i], _sub_Rn[i][var->number()], var->dofIndicesNeighbor(), var->scalingFactor());
}

void
Assembly::cacheResidualNodes(DenseVector<Number> & res, std::vector<dof_id_type> & dof_index)
{
  // Add the residual value and dof_index to cached_residual_values and cached_residual_rows respectively.
  // This is used by NodalConstraint.C to cache the residual calculated for master and slave node.
  Moose::KernelType type = Moose::KT_NONTIME;
  for (unsigned int i = 0; i < dof_index.size(); ++i)
  {
    _cached_residual_values[type].push_back(res(i));
    _cached_residual_rows[type].push_back(dof_index[i]);
  }
}

void
Assembly::addCachedResidual(NumericVector<Number> & residual, Moose::KernelType type)
{
  std::vector<Real> & cached_residual_values = _cached_residual_values[type];
  std::vector<dof_id_type> & cached_residual_rows = _cached_residual_rows[type];

  mooseAssert(cached_residual_values.size() == cached_residual_rows.size(), "Number of cached residuals and number of rows must match!");

  residual.add_vector(cached_residual_values, cached_residual_rows);

  if (_max_cached_residuals < cached_residual_values.size())
    _max_cached_residuals = cached_residual_values.size();

  // Try to be more efficient from now on
  // The 2 is just a fudge factor to keep us from having to grow the vector during assembly
  cached_residual_values.clear();
  cached_residual_values.reserve(_max_cached_residuals*2);

  cached_residual_rows.clear();
  cached_residual_rows.reserve(_max_cached_residuals*2);
}


void
Assembly::setResidualBlock(NumericVector<Number> & residual, DenseVector<Number> & res_block, std::vector<dof_id_type> & dof_indices, Real scaling_factor)
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
Assembly::setResidual(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(residual, _sub_Re[type][var->number()], var->dofIndices(), var->scalingFactor());
}

void
Assembly::setResidualNeighbor(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & var : vars)
    setResidualBlock(residual, _sub_Rn[type][var->number()], var->dofIndicesNeighbor(), var->scalingFactor());
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
Assembly::cacheJacobianBlock(DenseMatrix<Number> & jac_block, std::vector<dof_id_type> & idof_indices, std::vector<dof_id_type> & jdof_indices, Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0) && jac_block.n() && jac_block.m())
  {
    std::vector<dof_id_type> di(idof_indices);
    std::vector<dof_id_type> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
      jac_block *= scaling_factor;

    for (unsigned int i=0; i<di.size(); i++)
      for (unsigned int j=0; j<dj.size(); j++)
      {
        _cached_jacobian_values.push_back(jac_block(i, j));
        _cached_jacobian_rows.push_back(di[i]);
        _cached_jacobian_cols.push_back(dj[j]);
      }
  }
  jac_block.zero();
}

void
Assembly::cacheJacobianBlockNonlocal(DenseMatrix<Number> & jac_block, const std::vector<dof_id_type> & idof_indices, const std::vector<dof_id_type> & jdof_indices, Real scaling_factor)
{
  if ((idof_indices.size() > 0) && (jdof_indices.size() > 0) && jac_block.n() && jac_block.m())
  {
    std::vector<dof_id_type> di(idof_indices);
    std::vector<dof_id_type> dj(jdof_indices);
    _dof_map.constrain_element_matrix(jac_block, di, dj, false);

    if (scaling_factor != 1.0)
      jac_block *= scaling_factor;

    for (unsigned int i=0; i<di.size(); i++)
      for (unsigned int j=0; j<dj.size(); j++)
        if (jac_block(i, j) != 0.0) // no storage allocated for unimplemented jacobian terms, maintaining maximum sparsity possible
        {
          _cached_jacobian_values.push_back(jac_block(i, j));
          _cached_jacobian_rows.push_back(di[i]);
          _cached_jacobian_cols.push_back(dj[j]);
        }
  }
  jac_block.zero();
}

void
Assembly::addCachedJacobian(SparseMatrix<Number> & jacobian)
{
  if (!_sys.subproblem().checkNonlocalCouplingRequirement())
    mooseAssert(_cached_jacobian_rows.size() == _cached_jacobian_cols.size(),
                "Error: Cached data sizes MUST be the same!");

  for (unsigned int i=0; i<_cached_jacobian_rows.size(); i++)
    jacobian.add(_cached_jacobian_rows[i], _cached_jacobian_cols[i], _cached_jacobian_values[i]);

  if (_max_cached_jacobians < _cached_jacobian_values.size())
    _max_cached_jacobians = _cached_jacobian_values.size();

  // Try to be more efficient from now on
  // The 2 is just a fudge factor to keep us from having to grow the vector during assembly
  _cached_jacobian_values.clear();
  _cached_jacobian_values.reserve(_max_cached_jacobians*2);

  _cached_jacobian_rows.clear();
  _cached_jacobian_rows.reserve(_max_cached_jacobians*2);

  _cached_jacobian_cols.clear();
  _cached_jacobian_cols.reserve(_max_cached_jacobians*2);
}

void
Assembly::addJacobian(SparseMatrix<Number> & jacobian)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & ivar : vars)
    for (const auto & jvar : vars)
      if ((*_cm)(ivar->number(), jvar->number()) != 0 && _jacobian_block_used[ivar->number()][jvar->number()])
        addJacobianBlock(jacobian, jacobianBlock(ivar->number(), jvar->number()), ivar->dofIndices(), jvar->dofIndices(), ivar->scalingFactor());

  // Possibly add jacobian contributions from off-diagonal blocks coming from the scalar variables
  if (_sys.getScalarVariables(_tid).size() > 0)
  {
    const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
    const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
    for (const auto & ivar : scalar_vars)
    {
      for (const auto & jvar : vars)
      {
        // We only add jacobian blocks for d(nl-var)/d(scalar-var) now (these we generated by kernels and BCs)
        // jacobian blocks d(scalar-var)/d(nl-var) are added later with addJacobianScalar
        if ((*_cm)(jvar->number(), ivar->number()) != 0 && _jacobian_block_used[jvar->number()][ivar->number()])
          addJacobianBlock(jacobian, jacobianBlock(jvar->number(), ivar->number()), jvar->dofIndices(), ivar->dofIndices(), jvar->scalingFactor());
        if ((*_cm)(ivar->number(), jvar->number()) != 0 && _jacobian_block_used[ivar->number()][jvar->number()])
          addJacobianBlock(jacobian, jacobianBlock(ivar->number(), jvar->number()), ivar->dofIndices(), jvar->dofIndices(), ivar->scalingFactor());
      }
    }
  }
}

void
Assembly::addJacobianNonlocal(SparseMatrix<Number> & jacobian)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & ivar : vars)
    for (const auto & jvar : vars)
      if (_nonlocal_cm(ivar->number(), jvar->number()) != 0 && _jacobian_block_nonlocal_used[ivar->number()][jvar->number()])
        addJacobianBlock(jacobian, jacobianBlockNonlocal(ivar->number(), jvar->number()), ivar->dofIndices(), jvar->allDofIndices(), ivar->scalingFactor());
}

void
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & ivar : vars)
    for (const auto & jvar : vars)
      if ((*_cm)(ivar->number(), jvar->number()) != 0 && _jacobian_block_neighbor_used[ivar->number()][jvar->number()])
      {
        addJacobianBlock(jacobian,
                         jacobianBlockNeighbor(Moose::ElementNeighbor, ivar->number(), jvar->number()),
                         ivar->dofIndices(),
                         jvar->dofIndicesNeighbor(),
                         ivar->scalingFactor());

        addJacobianBlock(jacobian,
                         jacobianBlockNeighbor(Moose::NeighborElement, ivar->number(), jvar->number()),
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndices(),
                         ivar->scalingFactor());

        addJacobianBlock(jacobian,
                         jacobianBlockNeighbor(Moose::NeighborNeighbor, ivar->number(), jvar->number()),
                         ivar->dofIndicesNeighbor(),
                         jvar->dofIndicesNeighbor(),
                         ivar->scalingFactor());
      }
}

void
Assembly::cacheJacobian()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & ivar : vars)
    for (const auto & jvar : vars)
      if ((*_cm)(ivar->number(), jvar->number()) != 0 && _jacobian_block_used[ivar->number()][jvar->number()])
        cacheJacobianBlock(jacobianBlock(ivar->number(), jvar->number()), ivar->dofIndices(), jvar->dofIndices(), ivar->scalingFactor());

  // Possibly add jacobian contributions from off-diagonal blocks coming from the scalar variables
  if (_sys.getScalarVariables(_tid).size() > 0)
  {
    const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
    const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
    for (std::vector<MooseVariableScalar *>::const_iterator it = scalar_vars.begin(); it != scalar_vars.end(); ++it)
    {
      MooseVariableScalar & ivar = *(*it);
      for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
      {
        MooseVariable & jvar = *(*jt);
        // We only add jacobian blocks for d(nl-var)/d(scalar-var) now (these we generated by kernels and BCs)
        // jacobian blocks d(scalar-var)/d(nl-var) are added later with addJacobianScalar
        if ((*_cm)(jvar.number(), ivar.number()) != 0 && _jacobian_block_used[jvar.number()][ivar.number()])
          cacheJacobianBlock(jacobianBlock(jvar.number(), ivar.number()), jvar.dofIndices(), ivar.dofIndices(), jvar.scalingFactor());
        if ((*_cm)(ivar.number(), jvar.number()) != 0 && _jacobian_block_used[ivar.number()][jvar.number()])
          cacheJacobianBlock(jacobianBlock(ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
      }
    }
  }
}

void
Assembly::cacheJacobianNonlocal()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & ivar : vars)
    for (const auto & jvar : vars)
      if (_nonlocal_cm(ivar->number(), jvar->number()) != 0 && _jacobian_block_nonlocal_used[ivar->number()][jvar->number()])
        cacheJacobianBlockNonlocal(jacobianBlockNonlocal(ivar->number(), jvar->number()), ivar->dofIndices(), jvar->allDofIndices(), ivar->scalingFactor());
}

void
Assembly::cacheJacobianNeighbor()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (const auto & ivar : vars)
    for (const auto & jvar : vars)
      if ((*_cm)(ivar->number(), jvar->number()) != 0 && _jacobian_block_neighbor_used[ivar->number()][jvar->number()])
      {
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::ElementNeighbor, ivar->number(), jvar->number()), ivar->dofIndices(), jvar->dofIndicesNeighbor(), ivar->scalingFactor());
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborElement, ivar->number(), jvar->number()), ivar->dofIndicesNeighbor(), jvar->dofIndices(), ivar->scalingFactor());
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborNeighbor, ivar->number(), jvar->number()), ivar->dofIndicesNeighbor(), jvar->dofIndicesNeighbor(), ivar->scalingFactor());
      }
}

void
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices)
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
Assembly::addJacobianBlockNonlocal(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, const std::vector<dof_id_type> & idof_indices, const std::vector<dof_id_type> & jdof_indices)
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
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<dof_id_type> & dof_indices, std::vector<dof_id_type> & neighbor_dof_indices)
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
Assembly::addJacobianScalar(SparseMatrix<Number> & jacobian)
{
  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);
  for (const auto & ivar : scalar_vars)
    for (const auto & jvar : scalar_vars)
      if ((*_cm)(ivar->number(), jvar->number()) != 0 && _jacobian_block_used[ivar->number()][jvar->number()])
        addJacobianBlock(jacobian, jacobianBlock(ivar->number(), jvar->number()), ivar->dofIndices(), jvar->dofIndices(), ivar->scalingFactor());
}

void
Assembly::addJacobianOffDiagScalar(SparseMatrix<Number> & jacobian, unsigned int ivar)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  MooseVariableScalar & var_i = _sys.getScalarVariable(_tid, ivar);
  for (const auto & var_j : vars)
    if ((*_cm)(var_i.number(), var_j->number()) != 0 && _jacobian_block_used[var_i.number()][var_j->number()])
      addJacobianBlock(jacobian, jacobianBlock(var_i.number(), var_j->number()), var_i.dofIndices(), var_j->dofIndices(), var_i.scalingFactor());
}

void
Assembly::cacheJacobianContribution(numeric_index_type i, numeric_index_type j, Real value)
{
  _cached_jacobian_contribution_rows.push_back(i);
  _cached_jacobian_contribution_cols.push_back(j);
  _cached_jacobian_contribution_vals.push_back(value);
}

void
Assembly::setCachedJacobianContributions(SparseMatrix<Number> & jacobian)
{
  // First zero the rows (including the diagonals) to prepare for
  // setting the cached values.
  jacobian.zero_rows(_cached_jacobian_contribution_rows, 0.0);

  // TODO: Use SparseMatrix::set_values() for efficiency
  for (unsigned int i = 0; i < _cached_jacobian_contribution_vals.size(); ++i)
    jacobian.set(_cached_jacobian_contribution_rows[i],
                 _cached_jacobian_contribution_cols[i],
                 _cached_jacobian_contribution_vals[i]);

  clearCachedJacobianContributions();
}

void
Assembly::addCachedJacobianContributions(SparseMatrix<Number> & jacobian)
{
  // TODO: Use SparseMatrix::add_values() for efficiency
  for (unsigned int i = 0; i < _cached_jacobian_contribution_vals.size(); ++i)
    jacobian.add(_cached_jacobian_contribution_rows[i],
                 _cached_jacobian_contribution_cols[i],
                 _cached_jacobian_contribution_vals[i]);

  clearCachedJacobianContributions();
}

void
Assembly::clearCachedJacobianContributions()
{
  unsigned int orig_size = _cached_jacobian_contribution_rows.size();

  _cached_jacobian_contribution_rows.clear();
  _cached_jacobian_contribution_cols.clear();
  _cached_jacobian_contribution_vals.clear();

  // It's possible (though massively unlikely) that clear() will
  // change the capacity of the vectors, so let's be paranoid and
  // explicitly reserve() the same amount of memory to avoid multiple
  // push_back() induced allocations.  We reserve 20% more than the
  // original size that was cached to account for variations in the
  // number of BCs assigned to each thread (for when the Jacobian
  // contributions are computed threaded).
  _cached_jacobian_contribution_rows.reserve(1.2*orig_size);
  _cached_jacobian_contribution_cols.reserve(1.2*orig_size);
  _cached_jacobian_contribution_vals.reserve(1.2*orig_size);
}

void
Assembly::modifyWeightsDueToXFEM(const Elem *elem)
{
  mooseAssert(_xfem != NULL, "This function should not be called if xfem is inactive");

  if (_current_qrule == _current_qrule_arbitrary)
    return;

  MooseArray<Real> xfem_weight_multipliers;
  if (_xfem->getXFEMWeights(xfem_weight_multipliers, elem, _current_qrule,_current_q_points))
  {
    mooseAssert(xfem_weight_multipliers.size() == _current_JxW.size(),"Size of weight multipliers in xfem doesn't match number of quadrature points");
    for (unsigned i = 0; i < xfem_weight_multipliers.size(); i++)
    {
      _current_JxW[i] = _current_JxW[i] * xfem_weight_multipliers[i];
    }
    xfem_weight_multipliers.release();
  }
}
