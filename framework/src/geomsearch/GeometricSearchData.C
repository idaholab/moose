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

#include "GeometricSearchData.h"
//Moose includes
#include "NearestNodeLocator.h"
#include "PenetrationLocator.h"
#include "SubProblem.h"
#include "MooseMesh.h"

static const unsigned int MORTAR_BASE_ID = 2e6;


GeometricSearchData::GeometricSearchData(SubProblem & subproblem, MooseMesh & mesh) :
    _subproblem(subproblem),
    _mesh(mesh),
    _first(true)
{}

GeometricSearchData::~GeometricSearchData()
{
  for (std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator it = _penetration_locators.begin();
      it != _penetration_locators.end();
      ++it)
  {
    delete it->second;
  }

  for (std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator it = _nearest_node_locators.begin();
      it != _nearest_node_locators.end();
      ++it)
  {
    delete it->second;
  }
}

void
GeometricSearchData::update(GeometricSearchType type)
{
  if (type == ALL || type == QUADRATURE || type == NEAREST_NODE)
  {
    if (_first) // Only do this once
    {
      _first = false;

      for(std::map<unsigned int, unsigned int>::iterator it = _slave_to_qslave.begin();
          it != _slave_to_qslave.end();
          ++it)
        generateQuadratureNodes(it->first, it->second);
    }

    // Update the position of quadrature nodes first
    for(std::set<unsigned int>::iterator qbnd_it = _quadrature_boundaries.begin();
        qbnd_it != _quadrature_boundaries.end();
        ++qbnd_it)
      updateQuadratureNodes(*qbnd_it);
  }

  if (type == ALL || type == MORTAR)
    if (_mortar_boundaries.size() > 0)
      updateMortarNodes();

  if (type == ALL || type == NEAREST_NODE)
  {
    std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_it = _nearest_node_locators.begin();
    const std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_end = _nearest_node_locators.end();

    for(; nnl_it != nnl_end; ++nnl_it)
    {
      NearestNodeLocator * nnl = nnl_it->second;

      nnl->findNodes();
    }
  }

  if (type == ALL || type == PENETRATION)
  {
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_it = _penetration_locators.begin();
    std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_end = _penetration_locators.end();

    for(; pl_it != pl_end; ++pl_it)
    {
      PenetrationLocator * pl = pl_it->second;

      pl->detectPenetration();
    }
  }
}

void
GeometricSearchData::reinit()
{
  _mesh.clearQuadratureNodes();
  // Update the position of quadrature nodes first
  for(std::set<unsigned int>::iterator qbnd_it = _quadrature_boundaries.begin();
      qbnd_it != _quadrature_boundaries.end();
      ++qbnd_it)
    reinitQuadratureNodes(*qbnd_it);
  reinitMortarNodes();

  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_it = _nearest_node_locators.begin();
  const std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_end = _nearest_node_locators.end();

  for(; nnl_it != nnl_end; ++nnl_it)
  {
    NearestNodeLocator * nnl = nnl_it->second;

    nnl->reinit();
  }

  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_it = _penetration_locators.begin();
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_end = _penetration_locators.end();

  for(; pl_it != pl_end; ++pl_it)
  {
    PenetrationLocator * pl = pl_it->second;

    pl->reinit();
  }
}

PenetrationLocator &
GeometricSearchData::getPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  _subproblem.addGhostedBoundary(master_id);
  _subproblem.addGhostedBoundary(slave_id);

  PenetrationLocator * pl = _penetration_locators[std::pair<unsigned int, unsigned int>(master_id, slave_id)];

  if (!pl)
  {
    pl = new PenetrationLocator(_subproblem, *this, _mesh, master_id, slave_id, order, getNearestNodeLocator(master_id, slave_id));
    _penetration_locators[std::pair<unsigned int, unsigned int>(master_id, slave_id)] = pl;
  }

  return *pl;
}

PenetrationLocator &
GeometricSearchData::getQuadraturePenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  _subproblem.addGhostedBoundary(master_id);
  _subproblem.addGhostedBoundary(slave_id);

  // Generate a new boundary id
  // TODO: Make this better!
  unsigned int base_id = 1e6;
  unsigned int qslave_id = slave_id + base_id;

  _slave_to_qslave[slave_id] = qslave_id;

  PenetrationLocator * pl = _penetration_locators[std::pair<unsigned int, unsigned int>(master_id, qslave_id)];

  if (!pl)
  {
    pl = new PenetrationLocator(_subproblem, *this, _mesh, master_id, qslave_id, order, getQuadratureNearestNodeLocator(master_id, slave_id));
    _penetration_locators[std::pair<unsigned int, unsigned int>(master_id, qslave_id)] = pl;
  }

  return *pl;
}

PenetrationLocator &
GeometricSearchData::getMortarPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Moose::ConstraintType side_type, Order order)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  // Generate a new boundary id
  // TODO: Make this better!
  unsigned int mortar_boundary_id, boundary_id;
  switch (side_type)
  {
  case Moose::Master:
    boundary_id = master_id;
    mortar_boundary_id = MORTAR_BASE_ID + slave_id;
    _boundary_to_mortarboundary[slave_id] = mortar_boundary_id;
    break;

  case Moose::Slave:
    boundary_id = slave_id;
    mortar_boundary_id = MORTAR_BASE_ID + master_id;
    _boundary_to_mortarboundary[master_id] = mortar_boundary_id;
    break;
  }

  PenetrationLocator * pl = _penetration_locators[std::pair<unsigned int, unsigned int>(boundary_id, mortar_boundary_id)];
  if (!pl)
  {
    pl = new PenetrationLocator(_subproblem, *this, _mesh, boundary_id, mortar_boundary_id, order, getMortarNearestNodeLocator(master_id, slave_id, side_type));
    _penetration_locators[std::pair<unsigned int, unsigned int>(boundary_id, mortar_boundary_id)] = pl;
  }

  return *pl;
}

NearestNodeLocator &
GeometricSearchData::getNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  _subproblem.addGhostedBoundary(master_id);
  _subproblem.addGhostedBoundary(slave_id);

  return getNearestNodeLocator(master_id, slave_id);
}

NearestNodeLocator &
GeometricSearchData::getNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id)
{
  NearestNodeLocator * nnl = _nearest_node_locators[std::pair<unsigned int, unsigned int>(master_id, slave_id)];

  _subproblem.addGhostedBoundary(master_id);
  _subproblem.addGhostedBoundary(slave_id);

  if (!nnl)
  {
    nnl = new NearestNodeLocator(_subproblem, _mesh, master_id, slave_id);
    _nearest_node_locators[std::pair<unsigned int, unsigned int>(master_id, slave_id)] = nnl;
  }

  return *nnl;
}

NearestNodeLocator &
GeometricSearchData::getQuadratureNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  _subproblem.addGhostedBoundary(master_id);
  _subproblem.addGhostedBoundary(slave_id);

  return getQuadratureNearestNodeLocator(master_id, slave_id);
}

NearestNodeLocator &
GeometricSearchData::getQuadratureNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id)
{
  // TODO: Make this better!
  unsigned int base_id = 1e6;
  unsigned int qslave_id = slave_id + base_id;

  _slave_to_qslave[slave_id] = qslave_id;

  return getNearestNodeLocator(master_id, qslave_id);
}

void
GeometricSearchData::generateQuadratureNodes(unsigned int slave_id, unsigned int qslave_id)
{
  // Have we already generated quadrature nodes for this boundary id?
  if (_quadrature_boundaries.find(slave_id) != _quadrature_boundaries.end())
    return;

  _quadrature_boundaries.insert(slave_id);

  const MooseArray<Point> & points_face = _subproblem.assembly(0).qPointsFace();

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (ConstBndElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
  {
    const BndElement * belem = *elem_it;

    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;

    if (elem->processor_id() == libMesh::processor_id())
    {
      if (boundary_id == (BoundaryID)slave_id)
      {
        _subproblem.prepare(elem, 0);
        _subproblem.reinitElemFace(elem, side, boundary_id, 0);

        for(unsigned int qp=0; qp<points_face.size(); qp++)
        {
          _mesh.addQuadratureNode(elem, side, qp, qslave_id, points_face[qp]);
        }
      }
    }
  }
}

NearestNodeLocator &
GeometricSearchData::getMortarNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave, Moose::ConstraintType side_type)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  return getMortarNearestNodeLocator(master_id, slave_id, side_type);
}

NearestNodeLocator &
GeometricSearchData::getMortarNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id, Moose::ConstraintType side_type)
{
  unsigned int mortarboundary_id, boundary;

  switch (side_type)
  {
  case Moose::Master:
    boundary = master_id;
    mortarboundary_id = MORTAR_BASE_ID + slave_id;
    _boundary_to_mortarboundary[slave_id] = mortarboundary_id;
    break;

  case Moose::Slave:
    boundary = slave_id;
    mortarboundary_id = MORTAR_BASE_ID + master_id;
    _boundary_to_mortarboundary[master_id] = mortarboundary_id;
    break;
  }

  generateMortarNodes(master_id, slave_id, 1001);

  return getNearestNodeLocator(boundary, 1001);
}

void
GeometricSearchData::generateMortarNodes(unsigned int master_id, unsigned int slave_id, unsigned int qslave_id)
{
  // Have we already generated quadrature nodes for this boundary id?
  if (_mortar_boundaries.find(std::pair<unsigned int, unsigned int>(master_id, slave_id)) != _mortar_boundaries.end())
    return;

  _mortar_boundaries.insert(std::pair<unsigned int, unsigned int>(master_id, slave_id));

  MooseMesh::MortarInterface * iface = _mesh.getMortarInterface(master_id, slave_id);

  const MooseArray<Point> & qpoints = _subproblem.assembly(0).qPoints();
  for (std::vector<Elem *>::iterator it = iface->_elems.begin(); it != iface->_elems.end(); ++it)
  {
    Elem * elem = *it;
    _subproblem.assembly(0).reinit(elem);

    for (unsigned int qp = 0; qp < qpoints.size(); qp++)
      _mesh.addQuadratureNode(elem, 0, qp, qslave_id, qpoints[qp]);
  }
}


void
GeometricSearchData::updateQuadratureNodes(unsigned int slave_id)
{
  const MooseArray<Point> & points_face = _subproblem.assembly(0).qPointsFace();

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (ConstBndElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
  {
    const BndElement * belem = *elem_it;

    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;

    if (elem->processor_id() == libMesh::processor_id())
    {
      if (boundary_id == (BoundaryID)slave_id)
      {
        _subproblem.prepare(elem, 0);
        _subproblem.reinitElemFace(elem, side, boundary_id, 0);

        for(unsigned int qp=0; qp<points_face.size(); qp++)
          (*_mesh.getQuadratureNode(elem, side, qp)) = points_face[qp];
      }
    }
  }
}

void
GeometricSearchData::reinitQuadratureNodes(unsigned int /*slave_id*/)
{
  // Regenerate the quadrature nodes
  for(std::map<unsigned int, unsigned int>::iterator it = _slave_to_qslave.begin();
      it != _slave_to_qslave.end();
      ++it)
    generateQuadratureNodes(it->first, it->second);
}


void
GeometricSearchData::updateMortarNodes()
{
  const MooseArray<Point> & qpoints = _subproblem.assembly(0).qPoints();

  std::vector<MooseMesh::MortarInterface *> & ifaces = _mesh.getMortarInterfaces();
  for (std::vector<MooseMesh::MortarInterface *>::iterator iface_it = ifaces.begin(); iface_it != ifaces.end(); ++iface_it)
  {
    MooseMesh::MortarInterface * iface = *iface_it;

    for (std::vector<Elem *>::iterator it = iface->_elems.begin(); it != iface->_elems.end(); ++it)
    {
      Elem * elem = *it;
      _subproblem.assembly(0).reinit(elem);

      for (unsigned int qp = 0; qp < qpoints.size(); qp++)
        (*_mesh.getQuadratureNode(elem, 0, qp)) = qpoints[qp];
    }
  }
}

void
GeometricSearchData::reinitMortarNodes()
{
  _mortar_boundaries.clear();
  // Regenerate the quadrature nodes for mortar spaces
  std::vector<MooseMesh::MortarInterface *> & ifaces = _mesh.getMortarInterfaces();
  for (std::vector<MooseMesh::MortarInterface *>::iterator it = ifaces.begin(); it != ifaces.end(); ++it)
  {
    MooseMesh::MortarInterface * iface = *it;
    unsigned int master_id = _mesh.getBoundaryID(iface->_master);
    unsigned int slave_id  = _mesh.getBoundaryID(iface->_slave);
    generateMortarNodes(master_id, slave_id, 0);
  }
}
