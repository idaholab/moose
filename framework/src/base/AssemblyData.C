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

#include "AssemblyData.h"

// MOOSE includes
#include "SubProblem.h"
#include "ArbitraryQuadrature.h"

// libMesh
#include "quadrature_gauss.h"
#include "fe_interface.h"

AssemblyData::AssemblyData(MooseMesh & mesh) :
    _mesh(mesh),

    _fe_helper(getFE(FEType(FIRST, LAGRANGE))),
    _qrule(NULL),
    _qrule_volume(NULL),
    _qrule_arbitrary(NULL),
    _qface_arbitrary(NULL),
    _q_points(_fe_helper->get_xyz()),
    _JxW(_fe_helper->get_JxW()),

    _fe_face_helper(getFEFace(FEType(FIRST, LAGRANGE))),
    _qrule_face(NULL),
    _q_points_face(_fe_face_helper->get_xyz()),
    _JxW_face(_fe_face_helper->get_JxW()),
    _normals(_fe_face_helper->get_normals()),

    _current_elem(NULL),
    _current_side(0),
    _current_side_elem(NULL),
    _neighbor_elem(NULL),
    _current_node(NULL),
    _current_neighbor_node(NULL)
{
}

AssemblyData::~AssemblyData()
{
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    delete it->second;
  for (std::map<FEType, FEBase *>::iterator it = _fe_face.begin(); it != _fe_face.end(); ++it)
    delete it->second;
  for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor.begin(); it != _fe_neighbor.end(); ++it)
    delete it->second;
  delete _qrule_volume;
  delete _qrule_arbitrary;
  delete _qface_arbitrary;
  delete _qrule_face;
  delete _current_side_elem;
}

FEBase * &
AssemblyData::getFE(FEType type)
{
  if (!_fe[type])
    _fe[type] = FEBase::build(_mesh.dimension(), type).release();

  return _fe[type];
}

FEBase * &
AssemblyData::getFEFace(FEType type)
{
  if (!_fe_face[type])
    _fe_face[type] = FEBase::build(_mesh.dimension(), type).release();

  return _fe_face[type];
}

FEBase * &
AssemblyData::getFEFaceNeighbor(FEType type)
{
  if (!_fe_neighbor[type])
    _fe_neighbor[type] = FEBase::build(_mesh.dimension(), type).release();

  return _fe_neighbor[type];
}

void
AssemblyData::createQRules(QuadratureType type, Order o)
{
  _qrule_volume = QBase::build(type, _mesh.dimension(), o).release();
  _qrule_face = QBase::build(type, _mesh.dimension() - 1, o).release();
  _qrule_arbitrary = new ArbitraryQuadrature(_mesh.dimension(), o);

  setVolumeQRule(_qrule_volume);
  setFaceQRule(_qrule_face);
}

void
AssemblyData::setVolumeQRule(QBase * qrule)
{
  _qrule = qrule;

  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->attach_quadrature_rule(_qrule);
}

void
AssemblyData::setFaceQRule(QBase * qrule)
{
  _qrule_face = qrule;

  for (std::map<FEType, FEBase *>::iterator it = _fe_face.begin(); it != _fe_face.end(); ++it)
    it->second->attach_quadrature_rule(_qrule_face);
}

void
AssemblyData::reinit(const Elem * elem)
{
  // Make sure the qrule is the right one
  if(_qrule != _qrule_volume)
    setVolumeQRule(_qrule_volume);

  _current_elem = elem;
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->reinit(elem);
}

void
AssemblyData::reinitAtPhysical(const Elem * elem, const std::vector<Point> & physical_points)
{
  std::vector<Point> reference_points;

  FEInterface::inverse_map(_mesh.dimension(), FEType(), elem, physical_points, reference_points);

  reinit(elem, reference_points);

  // Save off the physical points
  _current_physical_points = physical_points;
}

void
AssemblyData::reinit(const Elem * elem, const std::vector<Point> & reference_points)
{
  // Make sure the qrule is the right one
  if(_qrule != _qrule_arbitrary)
    setVolumeQRule(_qrule_arbitrary);

  _qrule_arbitrary->setPoints(reference_points);

  _current_elem = elem;
  for (std::map<FEType, FEBase *>::iterator it = _fe.begin(); it != _fe.end(); ++it)
    it->second->reinit(elem);
}

void
AssemblyData::reinit(const Elem * elem, unsigned int side)
{
  _current_elem = elem;
  _current_side = side;

  if (_current_side_elem)
    delete _current_side_elem;
  _current_side_elem = elem->build_side(side).release();

  for (std::map<FEType, FEBase *>::iterator it = _fe_face.begin(); it != _fe_face.end(); ++it)
    it->second->reinit(elem, side);
}

void
AssemblyData::reinit(const Node * node)
{
  _current_node = node;
}

void
AssemblyData::reinitNodeNeighbor(const Node * node)
{
  _current_neighbor_node = node;
}

Real
AssemblyData::computeVolume()
{
  Real current_volume = 0;

//  if (_problem.geomType() == Moose::XYZ)
//  {
    for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
      current_volume += _JxW[qp];
//  }
//  else if (_problem.geomType() == Moose::CYLINDRICAL)
//  {
//    for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
//      current_volume += _q_points[qp](0) * _JxW[qp];
//  }
//  else
//    mooseError("geom_type must either be XYZ or CYLINDRICAL\n");

    return current_volume;
}

void
AssemblyData::reinit(const Elem * elem, unsigned int side, const Elem * neighbor)
{
  // reinit this element
  reinit(elem, side);

  // reinit neighbor element
  for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor.begin(); it != _fe_neighbor.end(); ++it)
  {
    FEType fe_type = it->first;

    // Find locations of quadrature points on the neighbor
    std::vector<Point> qface_neighbor_point;
    libMesh::FEInterface::inverse_map (elem->dim(), fe_type, neighbor, _q_points_face, qface_neighbor_point);
    // Calculate the neighbor element shape functions at those locations
    it->second->reinit(neighbor, &qface_neighbor_point);
  }

  _neighbor_elem = neighbor;
}

void
AssemblyData::reinitNeighborAtPhysical(const Elem * neighbor, unsigned int /*neighbor_side*/, const std::vector<Point> & physical_points)
{
  std::vector<Point> reference_points;

  // reinit neighbor element
  for (std::map<FEType, FEBase *>::iterator it = _fe_neighbor.begin(); it != _fe_neighbor.end(); ++it)
  {
    FEType fe_type = it->first;

    FEInterface::inverse_map(_mesh.dimension(), fe_type, neighbor, physical_points, reference_points);

    it->second->reinit(neighbor, &reference_points);
  }

  // Save off the physical points
  _current_physical_points = physical_points;

  // Make sure the qrule is the right one
  if(_qrule != _qrule_arbitrary)
    setVolumeQRule(_qrule_arbitrary);

  _qrule_arbitrary->setPoints(reference_points);
  _neighbor_elem = neighbor;
}
