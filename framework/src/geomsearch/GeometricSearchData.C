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

GeometricSearchData::GeometricSearchData(SubProblem & subproblem, MooseMesh & mesh) :
    _subproblem(subproblem),
    _mesh(mesh)
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
GeometricSearchData::update()
{
  std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_it = _nearest_node_locators.begin();
  const std::map<std::pair<unsigned int, unsigned int>, NearestNodeLocator *>::iterator nnl_end = _nearest_node_locators.end();

  for(; nnl_it != nnl_end; ++nnl_it)
  {
    NearestNodeLocator * nnl = nnl_it->second;

    nnl->findNodes();
  }

  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_it = _penetration_locators.begin();
  std::map<std::pair<unsigned int, unsigned int>, PenetrationLocator *>::iterator pl_end = _penetration_locators.end();

  for(; pl_it != pl_end; ++pl_it)
  {
    PenetrationLocator * pl = pl_it->second;

    pl->detectPenetration();
  }
}

PenetrationLocator &
GeometricSearchData::getPenetrationLocator(const BoundaryName & master, const BoundaryName & slave, Order order)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  PenetrationLocator * pl = _penetration_locators[std::pair<unsigned int, unsigned int>(master_id, slave_id)];

  if(!pl)
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

  // Generate a new boundary id
  // TODO: Make this better!
  unsigned int base_id = 1e6;
  unsigned int qslave_id = slave_id + base_id;

  PenetrationLocator * pl = _penetration_locators[std::pair<unsigned int, unsigned int>(master_id, qslave_id)];

  if(!pl)
  {
    pl = new PenetrationLocator(_subproblem, *this, _mesh, master_id, qslave_id, order, getQuadratureNearestNodeLocator(master_id, slave_id));
    _penetration_locators[std::pair<unsigned int, unsigned int>(master_id, qslave_id)] = pl;
  }

  return *pl;
}

NearestNodeLocator &
GeometricSearchData::getNearestNodeLocator(const BoundaryName & master, const BoundaryName & slave)
{
  unsigned int master_id = _mesh.getBoundaryID(master);
  unsigned int slave_id  = _mesh.getBoundaryID(slave);

  return getNearestNodeLocator(master_id, slave_id);
}

NearestNodeLocator &
GeometricSearchData::getNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id)
{
  NearestNodeLocator * nnl = _nearest_node_locators[std::pair<unsigned int, unsigned int>(master_id, slave_id)];

  if(!nnl)
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

  return getQuadratureNearestNodeLocator(master_id, slave_id);
}

NearestNodeLocator &
GeometricSearchData::getQuadratureNearestNodeLocator(const unsigned int master_id, const unsigned int slave_id)
{
  // TODO: Make this better!
  unsigned int base_id = 1e6;
  unsigned int qslave_id = slave_id + base_id;

  generateQuadratureNodes(slave_id, qslave_id);

  return getNearestNodeLocator(master_id, qslave_id);
}

void
GeometricSearchData::generateQuadratureNodes(unsigned int slave_id, unsigned int qslave_id)
{
  QBase * & qrule_face = _subproblem.qRuleFace(0);
  const MooseArray<Point> & points_face = _subproblem.pointsFace(0);

  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();
  for (ConstBndElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
  {
    const BndElement * belem = *elem_it;

    const Elem * elem = belem->_elem;
    unsigned short int side = belem->_side;
    BoundaryID boundary_id = belem->_bnd_id;

    if (elem->processor_id() == libMesh::processor_id())
    {
      if(boundary_id == slave_id)
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



