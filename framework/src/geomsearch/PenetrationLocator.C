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

#include "PenetrationLocator.h"

#include "Moose.h"
#include "ArbitraryQuadrature.h"
#include "LineSegment.h"
#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "GeometricSearchData.h"
#include "FindContactPoint.h"
#include "PenetrationThread.h"

PenetrationLocator::PenetrationLocator(SubProblem & subproblem, GeometricSearchData & geom_search_data, MooseMesh & mesh, unsigned int master, unsigned int slave) :
    _subproblem(subproblem),
    _mesh(mesh),
    _master_boundary(master),
    _slave_boundary(slave),
    _fe_type(),
    _nearest_node(geom_search_data.getNearestNodeLocator(master, slave)),
    _update_location(true)
{
  // Preconstruct an FE object for each thread we're going to use
  // This is a time savings so that the thread objects don't do this themselves multiple times
  _fe.resize(libMesh::n_threads());
  for(unsigned int i=0; i < libMesh::n_threads(); i++)
    _fe[i] = FEBase::build(_mesh.dimension()-1, _fe_type).release();  
}

PenetrationLocator::~PenetrationLocator()
{
  for(unsigned int i=0; i < libMesh::n_threads(); i++)  
    delete _fe[i];

  for (std::map<unsigned int, PenetrationInfo *>::iterator it = _penetration_info.begin(); it != _penetration_info.end(); ++it)
    delete it->second;
}

void
PenetrationLocator::detectPenetration()
{
  Moose::perf_log.push("detectPenetration()","Solve");

  // Data structures to hold the element boundary information
  std::vector< unsigned int > elem_list;
  std::vector< unsigned short int > side_list;
  std::vector< short int > id_list;

  // Retrieve the Element Boundary data structures from the mesh
  _mesh.build_side_list(elem_list, side_list, id_list);

  // Grab the slave nodes we need to worry about from the NearestNodeLocator
  NodeIdRange & slave_node_range = _nearest_node.slaveNodeRange();

  PenetrationThread pt(_mesh,
                       _master_boundary,
                       _slave_boundary,
                       _penetration_info,
                       _update_location,
                       _fe,
                       _fe_type,
                       _nearest_node,
                       _mesh.nodeToElemMap(),
                       elem_list,
                       side_list,
                       id_list);

  Threads::parallel_reduce(slave_node_range, pt);

  Moose::perf_log.pop("detectPenetration()","Solve");
}

Real
PenetrationLocator::penetrationDistance(unsigned int node_id)
{
  PenetrationInfo * info = _penetration_info[node_id];

  if (info)
    return info->_distance;
  else
    return 0;
}


RealVectorValue
PenetrationLocator::penetrationNormal(unsigned int node_id)
{
  std::map<unsigned int, PenetrationInfo *>::const_iterator found_it( _penetration_info.find(node_id) );

  if (found_it != _penetration_info.end())
    return found_it->second->_normal;
  else
    return RealVectorValue(0, 0, 0);
}

void
PenetrationLocator::setUpdate( bool update )
{
  _update_location = update;
}


PenetrationLocator::PenetrationInfo::PenetrationInfo(const Node * node, Elem * elem, Elem * side, unsigned int side_num, RealVectorValue norm, Real norm_distance, const Point & closest_point, const Point & closest_point_ref, const std::vector<std::vector<Real> > & side_phi)
  :_node(node),
   _elem(elem),
   _side(side),
   _side_num(side_num),
   _normal(norm),
   _distance(norm_distance),
   _closest_point(closest_point),
   _closest_point_ref(closest_point_ref),
   _side_phi(side_phi)
{}


PenetrationLocator::PenetrationInfo::PenetrationInfo(const PenetrationInfo & p) :
    _node(p._node),
    _elem(p._elem),
    _side(p._side), // Which one now owns _side?  There will be trouble if (when)
                    // both delete _side
    _side_num(p._side_num),
    _normal(p._normal),
    _distance(p._distance),
    _closest_point(p._closest_point),
    _closest_point_ref(p._closest_point_ref),
    _side_phi(p._side_phi)
{}

PenetrationLocator::PenetrationInfo::~PenetrationInfo()
{
  delete _side;
}

