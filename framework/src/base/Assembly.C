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

// libMesh
#include "libmesh/quadrature_gauss.h"
#include "libmesh/fe_interface.h"


Assembly::Assembly(SystemBase & sys, CouplingMatrix * & cm, THREAD_ID tid) :
    _sys(sys),
    _cm(cm),
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
    _current_side(0),
    _current_side_elem(NULL),
    _current_neighbor_elem(NULL),
    _current_neighbor_side(0),
    _current_neighbor_side_elem(NULL),
    _current_node(NULL),
    _current_neighbor_node(NULL),

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
  buildFaceNeighborFE(FEType(FIRST, LAGRANGE));

  // Build an FE helper object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
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

    _holder_fe_neighbor_helper[dim] = &_fe_neighbor[dim][FEType(FIRST, LAGRANGE)];
    (*_holder_fe_neighbor_helper[dim])->get_xyz();
    (*_holder_fe_neighbor_helper[dim])->get_JxW();
    (*_holder_fe_neighbor_helper[dim])->get_normals();
  }
}

Assembly::~Assembly()
{
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
    for (std::map<FEType, FEBase *>::iterator it = _fe[dim].begin(); it != _fe[dim].end(); ++it)
      delete it->second;
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
    for (std::map<FEType, FEBase *>::iterator it = _fe_face[dim].begin(); it != _fe_face[dim].end(); ++it)
      delete it->second;
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
    for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor[dim].begin(); it != _fe_neighbor[dim].end(); ++it)
      delete it->second;

  for (std::map<unsigned int, QBase *>::iterator it = _holder_qrule_volume.begin(); it != _holder_qrule_volume.end(); ++it)
    delete it->second;
  for (std::map<unsigned int, ArbitraryQuadrature *>::iterator it = _holder_qrule_arbitrary.begin(); it != _holder_qrule_arbitrary.end(); ++it)
    delete it->second;
  for (std::map<unsigned int, ArbitraryQuadrature *>::iterator it = _holder_qface_arbitrary.begin(); it != _holder_qface_arbitrary.end(); ++it)
    delete it->second;
  for (std::map<unsigned int, QBase *>::iterator it = _holder_qrule_face.begin(); it != _holder_qrule_face.end(); ++it)
    delete it->second;

  for (std::map<unsigned int, ArbitraryQuadrature *>::iterator it = _holder_qrule_neighbor.begin(); it != _holder_qrule_neighbor.end(); ++it)
    delete it->second;

  for (std::map<FEType, FEShapeData * >::iterator it = _fe_shape_data.begin(); it != _fe_shape_data.end(); ++it)
    delete it->second;
  for (std::map<FEType, FEShapeData * >::iterator it = _fe_shape_data_face.begin(); it != _fe_shape_data_face.end(); ++it)
    delete it->second;
  for (std::map<FEType, FEShapeData * >::iterator it = _fe_shape_data_face_neighbor.begin(); it != _fe_shape_data_face_neighbor.end(); ++it)
    delete it->second;

  delete _current_side_elem;
  delete _current_neighbor_side_elem;

  _current_physical_points.release();

  _coord.release();
}

void
Assembly::buildFE(FEType type)
{
  if (!_fe_shape_data[type])
    _fe_shape_data[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
  {
    if (!_fe[dim][type])
      _fe[dim][type] = FEBase::build(dim, type).release();
  }
}

void
Assembly::buildFaceFE(FEType type)
{
  if (!_fe_shape_data_face[type])
    _fe_shape_data_face[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
  {
    if (!_fe_face[dim][type])
      _fe_face[dim][type] = FEBase::build(dim, type).release();
  }
}

void
Assembly::buildFaceNeighborFE(FEType type)
{
  if (!_fe_shape_data_face_neighbor[type])
    _fe_shape_data_face_neighbor[type] = new FEShapeData;

  // Build an FE object for this type for each dimension up to the dimension of the current mesh
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
  {
    if (!_fe_neighbor[dim][type])
      _fe_neighbor[dim][type] = FEBase::build(dim, type).release();
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

void
Assembly::createQRules(QuadratureType type, Order order, Order volume_order, Order face_order)
{
  _holder_qrule_volume.clear();
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
    _holder_qrule_volume[dim] = QBase::build(type, dim, volume_order).release();

  _holder_qrule_face.clear();
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
    _holder_qrule_face[dim] = QBase::build(type, dim - 1, face_order).release();

  _holder_qrule_neighbor.clear();
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
    _holder_qrule_neighbor[dim] = new ArbitraryQuadrature(dim, face_order);

  _holder_qrule_arbitrary.clear();
  for (unsigned int dim=1; dim<=_mesh_dimension; dim++)
    _holder_qrule_arbitrary[dim] = new ArbitraryQuadrature(dim, order);

//  setVolumeQRule(_qrule_volume);
//  setFaceQRule(_qrule_face);
}

void
Assembly::setVolumeQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule = qrule;

  if (qrule) // Don't set a NULL qrule
  {
    for (std::map<FEType, FEBase *>::iterator it = _fe[dim].begin(); it != _fe[dim].end(); ++it)
      it->second->attach_quadrature_rule(_current_qrule);
  }
}

void
Assembly::setFaceQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_face = qrule;

  for (std::map<FEType, FEBase *>::iterator it = _fe_face[dim].begin(); it != _fe_face[dim].end(); ++it)
    it->second->attach_quadrature_rule(_current_qrule_face);
}

void
Assembly::setNeighborQRule(QBase * qrule, unsigned int dim)
{
  _current_qrule_neighbor = qrule;

  for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor[dim].begin(); it != _fe_neighbor[dim].end(); ++it)
    it->second->attach_quadrature_rule(_current_qrule_neighbor);
}

void
Assembly::invalidateCache()
{
  std::map<unsigned int, ElementFEShapeData * >::iterator it = _element_fe_shape_data_cache.begin();
  std::map<unsigned int, ElementFEShapeData * >::iterator end = _element_fe_shape_data_cache.end();

  for (; it!=end; ++it)
    it->second->_invalidated = true;
}

void
Assembly::reinitFE(const Elem * elem)
{
  unsigned int dim = elem->dim();
  std::map<FEType, FEBase *>::iterator it = _fe[dim].begin();
  std::map<FEType, FEBase *>::iterator end = _fe[dim].end();

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

  for (; it != end; ++it)
  {
    FEBase * fe = it->second;
    const FEType & fe_type = it->first;

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
      if (_need_second_derivative[fe_type])
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
      if (_need_second_derivative[fe_type])
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
}

void
Assembly::reinitFEFace(const Elem * elem, unsigned int side)
{
  unsigned int dim = elem->dim();

  for (std::map<FEType, FEBase *>::iterator it = _fe_face[dim].begin(); it != _fe_face[dim].end(); ++it)
  {
    FEBase * fe_face = it->second;
    const FEType & fe_type = it->first;
    FEShapeData * fesd = _fe_shape_data_face[fe_type];
    fe_face->reinit(elem, side);
    _current_fe_face[it->first] = fe_face;

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real> > &>(fe_face->get_phi()));
    fesd->_grad_phi.shallowCopy(const_cast<std::vector<std::vector<RealGradient> > &>(fe_face->get_dphi()));
    if (_need_second_derivative[fe_type])
      fesd->_second_phi.shallowCopy(const_cast<std::vector<std::vector<RealTensor> > &>(fe_face->get_d2phi()));
  }

  // During that last loop the helper objects will have been reinitialized as well
  // We need to dig out the q_points and JxW from it.
  _current_q_points_face.shallowCopy(const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_xyz()));
  _current_JxW_face.shallowCopy(const_cast<std::vector<Real> &>((*_holder_fe_face_helper[dim])->get_JxW()));
  _current_normals.shallowCopy(const_cast<std::vector<Point> &>((*_holder_fe_face_helper[dim])->get_normals()));
}

void
Assembly::reinit(const Elem * elem)
{
  _current_elem = elem;
  _current_neighbor_elem = NULL;

  unsigned int elem_dimension = elem->dim();

  _current_qrule_volume = _holder_qrule_volume[elem_dimension];

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_volume)
    setVolumeQRule(_current_qrule_volume, elem_dimension);

  _currently_fe_caching = true;

  reinitFE(elem);

  // set the coord transformation
  _coord.resize(_current_qrule->n_points());
  _coord_type = _sys.subproblem().getCoordSystem(elem->subdomain_id());
  switch (_coord_type)
  {
  case Moose::COORD_XYZ:
    for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
      _coord[qp] = 1.;
    break;

  case Moose::COORD_RZ:
    for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
      _coord[qp] = 2 * M_PI * _current_q_points[qp](0);
    break;

  case Moose::COORD_RSPHERICAL:
    for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
      _coord[qp] = 4 * M_PI * _current_q_points[qp](0) * _current_q_points[qp](0);
    break;

  default:
    mooseError("Unknown coordinate system");
    break;
  }

  //Compute the area of the element
  _current_elem_volume = 0.;
  for (unsigned int qp = 0; qp < _current_qrule->n_points(); qp++)
    _current_elem_volume += _current_JxW[qp] * _coord[qp];
}

void
Assembly::reinitAtPhysical(const Elem * elem, const std::vector<Point> & physical_points)
{
  _current_elem = elem;
  _current_neighbor_elem = NULL;

  std::vector<Point> reference_points;

  FEInterface::inverse_map(elem->dim(), FEType(), elem, physical_points, reference_points);

  _currently_fe_caching = false;

  reinit(elem, reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

void
Assembly::reinit(const Elem * elem, const std::vector<Point> & reference_points)
{
  _current_elem = elem;
  _current_neighbor_elem = NULL;

  unsigned int elem_dimension = _current_elem->dim();

  _current_qrule_arbitrary = _holder_qrule_arbitrary[elem_dimension];

  // Make sure the qrule is the right one
  if (_current_qrule != _current_qrule_arbitrary)
    setVolumeQRule(_current_qrule_arbitrary, elem_dimension);

  _current_qrule_arbitrary->setPoints(reference_points);

  _currently_fe_caching = false;

  reinitFE(elem);
}

void
Assembly::reinit(const Elem * elem, unsigned int side)
{
  _current_elem = elem;
  _current_side = side;
  _current_neighbor_elem = NULL;

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

  // set the coord transformation
  _coord.resize(_current_qrule_face->n_points());
  _coord_type = _sys.subproblem().getCoordSystem(elem->subdomain_id());
  switch (_coord_type)
  {
  case Moose::COORD_XYZ:
    for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
      _coord[qp] = 1.;
    break;

  case Moose::COORD_RZ:
    for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
      _coord[qp] = 2 * M_PI * _current_q_points_face[qp](0);
    break;

  case Moose::COORD_RSPHERICAL:
    for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
      _coord[qp] = 4 * M_PI * _current_q_points_face[qp](0) * _current_q_points_face[qp](0);
    break;

  default:
    mooseError("Unknown coordinate system");
    break;
  }

  //Compute the area of the element
  _current_side_volume = 0.;
  for (unsigned int qp = 0; qp < _current_qrule_face->n_points(); qp++)
    _current_side_volume += _current_JxW_face[qp] * _coord[qp];
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

  reinitNeighborAtReference(neighbor, reference_points);
}

void
Assembly::reinitNeighborAtReference(const Elem * neighbor, const std::vector<Point> & reference_points)
{
  unsigned int neighbor_dim = neighbor->dim();

  // reinit neighbor element
  for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor[neighbor_dim].begin(); it != _fe_neighbor[neighbor_dim].end(); ++it)
  {
    FEBase * fe_neighbor = it->second;
    FEType fe_type = it->first;
    FEShapeData * fesd = _fe_shape_data_face_neighbor[fe_type];

    it->second->reinit(neighbor, &reference_points);

    _current_fe_neighbor[it->first] = it->second;

    fesd->_phi.shallowCopy(const_cast<std::vector<std::vector<Real> > &>(fe_neighbor->get_phi()));
    fesd->_grad_phi.shallowCopy(const_cast<std::vector<std::vector<RealGradient> > &>(fe_neighbor->get_dphi()));
    if (_need_second_derivative[fe_type])
      fesd->_second_phi.shallowCopy(const_cast<std::vector<std::vector<RealTensor> > &>(fe_neighbor->get_d2phi()));
  }

  ArbitraryQuadrature * neighbor_rule = _holder_qrule_neighbor[neighbor_dim];
  neighbor_rule->setPoints(reference_points);
  setNeighborQRule(neighbor_rule, neighbor_dim);

  _current_neighbor_elem = neighbor;

  // Calculate the volume of the neighbor

  FEType fe_type (neighbor->default_order() , LAGRANGE);
  AutoPtr<FEBase> fe (FEBase::build(neighbor->dim(), fe_type));

  const std::vector<Real> & JxW = fe->get_JxW();
  const std::vector<Point> & q_points = fe->get_xyz();

  // The default quadrature rule should integrate the mass matrix,
  // thus it should be plenty to compute the area
  QGauss qrule (neighbor->dim(), fe_type.default_quadrature_order());
  fe->attach_quadrature_rule(&qrule);
  fe->reinit(neighbor);

  // set the coord transformation
  MooseArray<Real> coord;
  coord.resize(qrule.n_points());
  Moose::CoordinateSystemType coord_type = _sys.subproblem().getCoordSystem(neighbor->subdomain_id());
  switch (coord_type) // coord type should be the same for the neighbor
  {
  case Moose::COORD_XYZ:
    for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
      coord[qp] = 1.;
    break;

  case Moose::COORD_RZ:
    for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
      coord[qp] = 2 * M_PI * q_points[qp](0);
    break;

  case Moose::COORD_RSPHERICAL:
    for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
      coord[qp] = 4 * M_PI * q_points[qp](0) * q_points[qp](0);
    break;

  default:
    mooseError("Unknown coordinate system");
    break;
  }

  _current_neighbor_volume = 0.;
  for (unsigned int qp = 0; qp < qrule.n_points(); qp++)
    _current_neighbor_volume += JxW[qp] * coord[qp];

  coord.release();
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
  reinitNeighborAtReference(_current_neighbor_side_elem, reference_points);
  // compute JxW on the neighbor's face
  unsigned int neighbor_side_dim = _current_neighbor_side_elem->dim();
  _current_JxW_neighbor.shallowCopy(const_cast<std::vector<Real> &>((*_holder_fe_neighbor_helper[neighbor_side_dim])->get_JxW()));

  reinitNeighborAtReference(neighbor, reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

DenseMatrix<Number> &
Assembly::jacobianBlock(unsigned int ivar, unsigned int jvar)
{
  if (_block_diagonal_matrix)
    return _sub_Kee[ivar][0];
  else
    return _sub_Kee[ivar][jvar];
}

DenseMatrix<Number> &
Assembly::jacobianBlockNeighbor(Moose::DGJacobianType type, unsigned int ivar, unsigned int jvar)
{
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
Assembly::init()
{
  unsigned int n_vars = _sys.nVariables();

  // I want the blocks to go by columns first to reduce copying of shape function in assembling "full" Jacobian
  _cm_entry.clear();
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
  {
    unsigned int j = (*jt)->number();
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
      unsigned int i = (*it)->number();
      if ((*_cm)(i, j) != 0)
        _cm_entry.push_back(std::pair<MooseVariable *, MooseVariable *>(*it, *jt));
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
  _sub_Ken.resize(n_vars);
  _sub_Kne.resize(n_vars);
  _sub_Knn.resize(n_vars);

  for (unsigned int i = 0; i < n_vars; ++i)
  {
    if (!_block_diagonal_matrix)
    {
      _sub_Kee[i].resize(n_vars);
      _sub_Ken[i].resize(n_vars);
      _sub_Kne[i].resize(n_vars);
      _sub_Knn[i].resize(n_vars);
    }
    else
    {
      _sub_Kee[i].resize(1);
      _sub_Ken[i].resize(1);
      _sub_Kne[i].resize(1);
      _sub_Knn[i].resize(1);
    }
  }
}

void
Assembly::prepare()
{
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = _cm_entry.begin(); it != _cm_entry.end(); ++it)
  {
    MooseVariable & ivar = *(*it).first;
    MooseVariable & jvar = *(*it).second;

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    jacobianBlock(vi, vj).resize(ivar.dofIndices().size(), jvar.dofIndices().size());
    jacobianBlock(vi, vj).zero();
  }

  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & ivar = *(*it);

    for (unsigned int i = 0; i < _sub_Re.size(); i++)
    {
      _sub_Re[i][ivar.number()].resize(ivar.dofIndices().size());
      _sub_Re[i][ivar.number()].zero();
    }
  }
}

void
Assembly::prepareVariable(MooseVariable * var)
{
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = _cm_entry.begin(); it != _cm_entry.end(); ++it)
  {
    MooseVariable & ivar = *(*it).first;
    MooseVariable & jvar = *(*it).second;

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    if (vi == var->number() || vj == var->number())
    {
      jacobianBlock(vi,vj).resize(ivar.dofIndices().size(), jvar.dofIndices().size());
    }
  }

  for (unsigned int i = 0; i < _sub_Re.size(); i++)
  {
    _sub_Re[i][var->number()].resize(var->dofIndices().size());
    _sub_Re[i][var->number()].zero();
  }
}

void
Assembly::prepareNeighbor()
{
  for (std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator it = _cm_entry.begin(); it != _cm_entry.end(); ++it)
  {
    MooseVariable & ivar = *(*it).first;
    MooseVariable & jvar = *(*it).second;

    unsigned int vi = ivar.number();
    unsigned int vj = jvar.number();

    jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj).resize(ivar.dofIndices().size(), jvar.dofIndicesNeighbor().size());
    jacobianBlockNeighbor(Moose::ElementNeighbor, vi, vj).zero();

    jacobianBlockNeighbor(Moose::NeighborElement, vi, vj).resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndices().size());
    jacobianBlockNeighbor(Moose::NeighborElement, vi, vj).zero();

    jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj).resize(ivar.dofIndicesNeighbor().size(), jvar.dofIndicesNeighbor().size());
    jacobianBlockNeighbor(Moose::NeighborNeighbor, vi, vj).zero();
  }

  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & ivar = *(*it);
    for (unsigned int i = 0; i < _sub_Rn.size(); i++)
    {
      _sub_Rn[i][ivar.number()].resize(ivar.dofIndicesNeighbor().size());
      _sub_Rn[i][ivar.number()].zero();
    }
  }
}

void
Assembly::prepareBlock(unsigned int ivar, unsigned jvar, const std::vector<dof_id_type> & dof_indices)
{
  jacobianBlock(ivar,jvar).resize(dof_indices.size(), dof_indices.size());
  jacobianBlock(ivar,jvar).zero();

  for (unsigned int i = 0; i < _sub_Re.size(); i++)
  {
    _sub_Re[i][ivar].resize(dof_indices.size());
    _sub_Re[i][ivar].zero();
  }
}

void
Assembly::prepareScalar()
{
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (std::vector<MooseVariableScalar *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariableScalar & ivar = *(*it);
    unsigned int idofs = ivar.dofIndices().size();

    for (unsigned int i = 0; i < _sub_Re.size(); i++)
    {
      _sub_Re[i][ivar.number()].resize(idofs);
      _sub_Re[i][ivar.number()].zero();
    }

    for (std::vector<MooseVariableScalar *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
    {
      MooseVariableScalar & jvar = *(*jt);
      unsigned int jdofs = jvar.dofIndices().size();

      jacobianBlock(ivar.number(), jvar.number()).resize(idofs, jdofs);
      jacobianBlock(ivar.number(), jvar.number()).zero();
    }
  }
}

void
Assembly::prepareOffDiagScalar()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  const std::vector<MooseVariableScalar *> & scalar_vars = _sys.getScalarVariables(_tid);

  for (std::vector<MooseVariableScalar *>::const_iterator it = scalar_vars.begin(); it != scalar_vars.end(); ++it)
  {
    MooseVariableScalar & ivar = *(*it);
    unsigned int idofs = ivar.dofIndices().size();

    for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
    {
      MooseVariable & jvar = *(*jt);
      unsigned int jdofs = jvar.dofIndices().size();

      jacobianBlock(ivar.number(), jvar.number()).resize(idofs, jdofs);
      jacobianBlock(ivar.number(), jvar.number()).zero();

      jacobianBlock(jvar.number(), ivar.number()).resize(jdofs, idofs);
      jacobianBlock(jvar.number(), ivar.number()).zero();
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
Assembly::cacheResidualBlock(std::vector<Real> & cached_residual_values, std::vector<unsigned int> & cached_residual_rows, DenseVector<Number> & res_block, std::vector<dof_id_type> & dof_indices, Real scaling_factor)
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
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & var = *(*it);
    addResidualBlock(residual, _sub_Re[type][var.number()], var.dofIndices(), var.scalingFactor());
  }
}

void
Assembly::addResidualNeighbor(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & var = *(*it);
    addResidualBlock(residual, _sub_Rn[type][var.number()], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}

void
Assembly::addResidualScalar(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  // add the scalar variables residuals
  const std::vector<MooseVariableScalar *> & vars = _sys.getScalarVariables(_tid);
  for (std::vector<MooseVariableScalar *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariableScalar & var = *(*it);
    addResidualBlock(residual, _sub_Re[type][var.number()], var.dofIndices(), var.scalingFactor());
  }
}


void
Assembly::cacheResidual()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & var = *(*it);

    for (unsigned int i = 0; i < _sub_Re.size(); i++)
      cacheResidualBlock(_cached_residual_values[i], _cached_residual_rows[i], _sub_Re[i][var.number()], var.dofIndices(), var.scalingFactor());
  }
}

void
Assembly::cacheResidualNeighbor()
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & var = *(*it);

    for (unsigned int i = 0; i < _sub_Re.size(); i++)
      cacheResidualBlock(_cached_residual_values[i], _cached_residual_rows[i], _sub_Rn[i][var.number()], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}


void
Assembly::addCachedResidual(NumericVector<Number> & residual, Moose::KernelType type)
{
  std::vector<Real> & cached_residual_values = _cached_residual_values[type];
  std::vector<unsigned int> & cached_residual_rows = _cached_residual_rows[type];

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
      for (unsigned int i=0; i<di.size(); i++)
        residual.set(di[i], _tmp_Re(i));
    }
    else
      for (unsigned int i=0; i<di.size(); i++)
        residual.set(di[i], res_block(i));
  }
}

void
Assembly::setResidual(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  const std::vector<MooseVariable *> vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & var = *(*it);
    setResidualBlock(residual, _sub_Re[type][var.number()], var.dofIndices(), var.scalingFactor());
  }
}

void
Assembly::setResidualNeighbor(NumericVector<Number> & residual, Moose::KernelType type/* = Moose::KT_NONTIME*/)
{
  const std::vector<MooseVariable *> vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & var = *(*it);
    setResidualBlock(residual, _sub_Rn[type][var.number()], var.dofIndicesNeighbor(), var.scalingFactor());
  }
}


void
Assembly::addJacobianBlock(SparseMatrix<Number> & jacobian, DenseMatrix<Number> & jac_block, const std::vector<dof_id_type> & idof_indices, const std::vector<dof_id_type> & jdof_indices, Real scaling_factor)
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
    {
      _tmp_Ke = jac_block;
      _tmp_Ke *= scaling_factor;

      for (unsigned int i=0; i<di.size(); i++)
        for (unsigned int j=0; j<dj.size(); j++)
        {
          _cached_jacobian_values.push_back(_tmp_Ke(i, j));
          _cached_jacobian_rows.push_back(di[i]);
          _cached_jacobian_cols.push_back(dj[j]);
        }
    }
    else
    {
      for (unsigned int i=0; i<di.size(); i++)
        for (unsigned int j=0; j<dj.size(); j++)
        {
          _cached_jacobian_values.push_back(jac_block(i, j));
          _cached_jacobian_rows.push_back(di[i]);
          _cached_jacobian_cols.push_back(dj[j]);
        }
    }
  }

  jac_block.zero();
}


void
Assembly::addCachedJacobian(SparseMatrix<Number> & jacobian)
{
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
  const std::vector<MooseVariable *> vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & ivar = *(*it);
    for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
    {
      MooseVariable & jvar = *(*jt);
      if ((*_cm)(ivar.number(), jvar.number()) != 0)
        addJacobianBlock(jacobian, jacobianBlock(ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
    }
  }

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
        if ((*_cm)(ivar.number(), jvar.number()) != 0)
        {
          addJacobianBlock(jacobian, jacobianBlock(jvar.number(), ivar.number()), jvar.dofIndices(), ivar.dofIndices(), jvar.scalingFactor());
          addJacobianBlock(jacobian, jacobianBlock(ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
        }
      }
    }
  }
}

void
Assembly::addJacobianNeighbor(SparseMatrix<Number> & jacobian)
{
  const std::vector<MooseVariable *> vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & ivar = *(*it);
    for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
    {
      MooseVariable & jvar = *(*jt);
      if ((*_cm)(ivar.number(), jvar.number()) != 0)
      {
        addJacobianBlock(jacobian, jacobianBlockNeighbor(Moose::ElementNeighbor, ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
        addJacobianBlock(jacobian, jacobianBlockNeighbor(Moose::NeighborElement, ivar.number(), jvar.number()), ivar.dofIndicesNeighbor(), jvar.dofIndices(), ivar.scalingFactor());
        addJacobianBlock(jacobian, jacobianBlockNeighbor(Moose::NeighborNeighbor, ivar.number(), jvar.number()), ivar.dofIndicesNeighbor(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
      }
    }
  }
}

void
Assembly::cacheJacobian()
{
  const std::vector<MooseVariable *> vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & ivar = *(*it);
    for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
    {
      MooseVariable & jvar = *(*jt);
      if ((*_cm)(ivar.number(), jvar.number()) != 0)
        cacheJacobianBlock(jacobianBlock(ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
    }
  }

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
        if ((*_cm)(ivar.number(), jvar.number()) != 0)
        {
          cacheJacobianBlock(jacobianBlock(jvar.number(), ivar.number()), jvar.dofIndices(), ivar.dofIndices(), jvar.scalingFactor());
          cacheJacobianBlock(jacobianBlock(ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
        }
      }
    }
  }
}

void
Assembly::cacheJacobianNeighbor()
{
  const std::vector<MooseVariable *> vars = _sys.getVariables(_tid);
  for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
  {
    MooseVariable & ivar = *(*it);
    for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
    {
      MooseVariable & jvar = *(*jt);
      if ((*_cm)(ivar.number(), jvar.number()) != 0)
      {
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::ElementNeighbor, ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborElement, ivar.number(), jvar.number()), ivar.dofIndicesNeighbor(), jvar.dofIndices(), ivar.scalingFactor());
        cacheJacobianBlock(jacobianBlockNeighbor(Moose::NeighborNeighbor, ivar.number(), jvar.number()), ivar.dofIndicesNeighbor(), jvar.dofIndicesNeighbor(), ivar.scalingFactor());
      }
    }
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
  for (std::vector<MooseVariableScalar *>::const_iterator it = scalar_vars.begin(); it != scalar_vars.end(); ++it)
  {
    MooseVariableScalar & ivar = *(*it);
    for (std::vector<MooseVariableScalar *>::const_iterator jt = scalar_vars.begin(); jt != scalar_vars.end(); ++jt)
    {
      MooseVariableScalar & jvar = *(*jt);
      if ((*_cm)(ivar.number(), jvar.number()) != 0)
        addJacobianBlock(jacobian, jacobianBlock(ivar.number(), jvar.number()), ivar.dofIndices(), jvar.dofIndices(), ivar.scalingFactor());
    }
  }
}

void
Assembly::addJacobianOffDiagScalar(SparseMatrix<Number> & jacobian, unsigned int ivar)
{
  const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
  MooseVariableScalar & var_i = _sys.getScalarVariable(_tid, ivar);
  for (std::vector<MooseVariable *>::const_iterator jt = vars.begin(); jt != vars.end(); ++jt)
  {
    MooseVariable & var_j = *(*jt);
    if ((*_cm)(var_i.number(), var_j.number()) != 0)
      addJacobianBlock(jacobian, jacobianBlock(var_i.number(), var_j.number()), var_i.dofIndices(), var_j.dofIndices(), var_i.scalingFactor());
  }
}
