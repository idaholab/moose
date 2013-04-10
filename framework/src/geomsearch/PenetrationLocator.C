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
#include "ArbitraryQuadrature.h"
#include "LineSegment.h"
#include "NearestNodeLocator.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "GeometricSearchData.h"
#include "PenetrationThread.h"
#include "Moose.h"

PenetrationLocator::PenetrationLocator(SubProblem & subproblem, GeometricSearchData & /*geom_search_data*/, MooseMesh & mesh, const unsigned int master_id, const unsigned int slave_id, Order order, NearestNodeLocator & nearest_node) :
    _subproblem(subproblem),
    _mesh(mesh),
    _master_boundary(master_id),
    _slave_boundary(slave_id),
    _fe_type(order),
    _nearest_node(nearest_node),
    _update_location(true),
    _tangential_tolerance(0.0),
    _do_normal_smoothing(false),
    _normal_smoothing_distance(0.0)
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
  _mesh.buildSideList(elem_list, side_list, id_list);

  // Grab the slave nodes we need to worry about from the NearestNodeLocator
  NodeIdRange & slave_node_range = _nearest_node.slaveNodeRange();

  PenetrationThread pt(_mesh,
                       _master_boundary,
                       _slave_boundary,
                       _penetration_info,
                       _update_location,
                       _tangential_tolerance,
                       _do_normal_smoothing,
                       _normal_smoothing_distance,
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

void
PenetrationLocator::setTangentialTolerance(Real tangential_tolerance)
{
  _tangential_tolerance = tangential_tolerance;
}

void
PenetrationLocator::setNormalSmoothingDistance(Real normal_smoothing_distance)
{
  _normal_smoothing_distance = normal_smoothing_distance;
  if (_normal_smoothing_distance > 0.0)
    _do_normal_smoothing = true;
}

void
PenetrationLocator::setStartingContactPoint()
{
  std::map<unsigned int, PenetrationInfo *>::iterator it( _penetration_info.begin() );
  const std::map<unsigned int, PenetrationInfo *>::iterator it_end( _penetration_info.end() );
  for ( ; it != it_end; ++it )
  {
    if (it->second != NULL)
    {
      it->second->_starting_elem = it->second->_elem;
      it->second->_starting_side_num = it->second->_side_num;
      it->second->_starting_closest_point_ref = it->second->_closest_point_ref;
    }
  }
}

void
PenetrationLocator::saveContactForce()
{
  std::map<unsigned int, PenetrationInfo *>::iterator it( _penetration_info.begin() );
  const std::map<unsigned int, PenetrationInfo *>::iterator it_end( _penetration_info.end() );
  for ( ; it != it_end; ++it )
  {
    if (it->second != NULL)
    {
      it->second->_contact_force_old = it->second->_contact_force;
    }
  }
}



PenetrationLocator::PenetrationInfo::PenetrationInfo(const Node * node, const Elem * elem, Elem * side, unsigned int side_num, RealVectorValue norm, Real norm_distance, Real tangential_distance, const Point & closest_point, const Point & closest_point_ref, const Point & closest_point_on_face_ref, std::vector<Node*> off_edge_nodes, const std::vector<std::vector<Real> > & side_phi, const std::vector<RealGradient> & dxyzdxi, const std::vector<RealGradient> & dxyzdeta, const std::vector<RealGradient> & d2xyzdxideta)
  :_node(node),
   _elem(elem),
   _side(side),
   _side_num(side_num),
   _normal(norm),
   _distance(norm_distance),
   _tangential_distance(tangential_distance),
   _closest_point(closest_point),
   _closest_point_ref(closest_point_ref),
   _closest_point_on_face_ref(closest_point_on_face_ref),
   _off_edge_nodes(off_edge_nodes),
   _side_phi(side_phi),
   _dxyzdxi(dxyzdxi),
   _dxyzdeta(dxyzdeta),
   _d2xyzdxideta(d2xyzdxideta),
   _update(true)
{}


PenetrationLocator::PenetrationInfo::PenetrationInfo(const PenetrationInfo & p) :
    _node(p._node),
    _elem(p._elem),
    _side(p._side), // Which one now owns _side?  There will be trouble if (when)
                    // both delete _side
    _side_num(p._side_num),
    _normal(p._normal),
    _distance(p._distance),
    _tangential_distance(p._tangential_distance),
    _closest_point(p._closest_point),
    _closest_point_ref(p._closest_point_ref),
    _closest_point_on_face_ref(p._closest_point_on_face_ref),
    _off_edge_nodes(p._off_edge_nodes),
    _side_phi(p._side_phi),
    _dxyzdxi(p._dxyzdxi),
    _dxyzdeta(p._dxyzdeta),
    _d2xyzdxideta(p._d2xyzdxideta),
    _starting_elem(p._starting_elem),
    _starting_side_num(p._starting_side_num),
    _starting_closest_point_ref(p._starting_closest_point_ref),
    _contact_force(p._contact_force),
    _contact_force_old(p._contact_force_old),
    _update(p._update)
{}

PenetrationLocator::PenetrationInfo::~PenetrationInfo()
{
  delete _side;
}

