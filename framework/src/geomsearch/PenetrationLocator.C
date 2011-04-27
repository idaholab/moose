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

PenetrationLocator::PenetrationLocator(SubProblem & subproblem, GeometricSearchData & geom_search_data, MooseMesh & mesh, unsigned int master, unsigned int slave) :
    _subproblem(subproblem),
    _mesh(mesh),
    _master_boundary(master),
    _slave_boundary(slave),
    _fe_type(),
    _fe(FEBase::build(_mesh.dimension()-1, _fe_type).release()),
    _nearest_node(geom_search_data.getNearestNodeLocator(master, slave)),
    _update_location(true)
{}

PenetrationLocator::~PenetrationLocator()
{
  delete _fe;

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
  const std::vector<unsigned int> & node_list = _nearest_node.slaveNodes();

//  const std::vector<short int> & node_boundary_list = _mesh.getBoundaryNodeListIds();

  const unsigned int n_nodes = node_list.size();
  const unsigned int n_elems = elem_list.size();

  for(unsigned int i=0; i<n_nodes; ++i)
  {
    Node & node = _mesh.node(node_list[i]);

    // See if we already have info about this node
    if(_penetration_info[node.id()])
    {
      PenetrationInfo * info = _penetration_info[node.id()];

      Elem * elem = info->_elem;

      if (!_update_location && info->_distance > 0)
      {
        Point contact_ref = info->_closest_point_ref;
        Point contact_phys;
        Real distance;
        RealGradient normal;
        bool contact_point_on_side;
        std::vector<std::vector<Real> > side_phi;

        // Slave position must be the previous contact point
        // Use the previous reference coordinates
        std::vector<Point> points(1);
        points[0] = contact_ref;
        _fe->reinit(info->_side, &points);
        const std::vector<Point> & slave_pos = _fe->get_xyz();

        Moose::findContactPoint(_fe, _fe_type, elem, info->_side_num, slave_pos[0],
                                false, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

        info->_normal = normal;
        // info->_closest_point_ref = contact_ref;
        // info->_distance = distance;
        info->_side_phi = side_phi;

        mooseAssert(info->_distance >= 0, "Error in PenetrationLocator: Slave node contained in element but contact distance was negative!");

        info->_closest_point = contact_phys;

        continue;
      }

      // See if the same element still contains this point
      if(elem->contains_point(node))
      {
        Point contact_ref = info->_closest_point_ref;
        Point contact_phys;
        Real distance;
        RealGradient normal;
        bool contact_point_on_side;
        std::vector<std::vector<Real> > side_phi;

        Moose::findContactPoint(_fe, _fe_type, elem, info->_side_num, node, false, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

        info->_normal = normal;
        info->_closest_point_ref = contact_ref;
        info->_distance = distance;
        info->_side_phi = side_phi;

        mooseAssert(info->_distance >= 0, "Error in PenetrationLocator: Slave node contained in element but contact distance was negative!");

        info->_closest_point = contact_phys;

        // I hate continues but this is actually cleaner than anything I can think of
        continue;
      }
      else
      {
        // See if this element still has the same one across from it

        Point contact_ref = info->_closest_point_ref;
        Point contact_phys;
        Real distance;
        RealGradient normal;
        bool contact_point_on_side;
        std::vector<std::vector<Real> > side_phi;

        Moose::findContactPoint(_fe, _fe_type, elem, info->_side_num, node, false, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

        if(contact_point_on_side)
        {
          info->_normal = normal;
          info->_closest_point_ref = contact_ref;
          info->_distance = distance;
          mooseAssert(info->_distance <= 0, "Error in PenetrationLocator: Slave node not contained in element but distance was positive!");
          info->_side_phi = side_phi;

          info->_closest_point = contact_phys;

          continue;
        }
        else
        {
          delete _penetration_info[node.id()];
          _penetration_info[node.id()] = NULL;
        }
      }
    }

    const Node * closest_node = _nearest_node.nearestNode(node.id());
    std::vector<unsigned int> & closest_elems = _mesh.nodeToElemMap()[closest_node->id()];
    std::vector<PenetrationInfo*> p_info;


    for(unsigned int j=0; j<closest_elems.size(); j++)
    {
      unsigned int elem_id = closest_elems[j];
      Elem * elem = _mesh.elem(elem_id);

      for(unsigned int m=0; m<n_elems; m++)
      {
        if(elem_list[m] == elem_id && id_list[m] == _master_boundary)
        {
          unsigned int side_num = side_list[m];

          Elem *side = (elem->build_side(side_num,false)).release();

          Point contact_ref;
          Point contact_phys;
          Real distance;
          RealGradient normal;
          bool contact_point_on_side;
          std::vector<std::vector<Real> > side_phi;

          Moose::findContactPoint(_fe, _fe_type, elem, side_num, node, true, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

          PenetrationInfo * pen_info =  new PenetrationInfo(&node,
                                                            elem,
                                                            side,
                                                            side_num,
                                                            normal,
                                                            distance,
                                                            contact_phys,
                                                            contact_ref,
                                                            side_phi);

          if(contact_point_on_side && _penetration_info[node.id()] &&
             (
               (std::abs(_penetration_info[node.id()]->_distance) > std::abs(distance)) ||
               (_penetration_info[node.id()]->_distance < 0 && distance > 0)
               )
            )
          {
            delete _penetration_info[node.id()];
            _penetration_info[node.id()] = NULL;
          }

          if(contact_point_on_side && (!_penetration_info[node.id()] ||
                                       (
                                         (std::abs(_penetration_info[node.id()]->_distance) > std::abs(distance)) ||
                                         (_penetration_info[node.id()]->_distance < 0 && distance > 0)
                                         )
               )
            )
          {
            _penetration_info[node.id()] = pen_info;
          }
          else
          {
            p_info.push_back( pen_info );
          }
        }
      }
    }
    if (!_penetration_info[node.id()] && p_info.size() > 1)
    {
      // No face is clearly above/below this node.
      // See if we need to force a match.

      // Restrict the parametric coordinates to the domain of the face
      for ( unsigned int j(0); j < p_info.size(); ++j )
      {
        for ( unsigned int k(0); k < p_info[j]->_elem->dim(); ++k )
        {
          if ( p_info[j]->_closest_point_ref(k) < -1 )
          {
            p_info[j]->_closest_point_ref(k) = -1;
          }
          if ( p_info[j]->_closest_point_ref(k) > 1 )
          {
            p_info[j]->_closest_point_ref(k) = 1;
          }
        }
      }
      // Find the element/face closest to the node
      unsigned int closest_index(0);
      std::vector<Point> points(1);
      points[0] = p_info[0]->_closest_point_ref;
      _fe->reinit(p_info[0]->_side, &points);
      Point closest_coor = _fe->get_xyz()[0];
      Real dist = (closest_coor - node).size();
      for ( unsigned int j(1); j < p_info.size(); ++j )
      {
        points[0] = p_info[j]->_closest_point_ref;
        _fe->reinit(p_info[j]->_side, &points);
        const Point coor = _fe->get_xyz()[0];
        Real dist2 = (coor - node).size();
        if (dist2 < dist)
        {
          dist = dist2;
          closest_coor = coor;
          closest_index = j;
        }
      }

      // We now have the index for the closest face.
      for ( unsigned int j(0); j < p_info.size(); ++j )
      {

        if ( j != closest_index )
        {
          Point contact_ref;
          Point contact_phys;
          Real distance;
          RealGradient normal;
          bool contact_point_on_side(false);
          std::vector<std::vector<Real> > side_phi;

          Moose::findContactPoint(_fe, _fe_type, p_info[j]->_elem, p_info[j]->_side_num, closest_coor, true, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);
          if ( contact_point_on_side )
          {
            // We have found a match.  This node is in contact.
            p_info[closest_index]->_closest_point = closest_coor;
            p_info[closest_index]->_distance = (p_info[closest_index]->_distance > 0 ? 1 : -1) * dist;
            Point normal(closest_coor - node);
            const Real len(normal.size());
            if (len > 0)
            {
              normal /= len;
            }
            const Real dot(normal * p_info[closest_index]->_normal);
            if (dot < 0)
            {
              normal *= -1;
            }
            p_info[closest_index]->_normal = normal;

            points[0] = p_info[closest_index]->_closest_point_ref;
            _fe->reinit(p_info[closest_index]->_side, &points);
            p_info[closest_index]->_side_phi = _fe->get_phi();

            _penetration_info[node.id()] = p_info[closest_index];
            // Set the entry in p_info to NULL so that we don't delete it (it is now
            // owned by _penetration_info).
            p_info[closest_index] = NULL;

            break;
          }
        }
      }
    }
    for ( unsigned int j(0); j < p_info.size(); ++j )
    {
      delete p_info[j];
    }
  }

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


PenetrationLocator::PenetrationInfo::PenetrationInfo(Node * node, Elem * elem, Elem * side, unsigned int side_num, RealVectorValue norm, Real norm_distance, const Point & closest_point, const Point & closest_point_ref, const std::vector<std::vector<Real> > & side_phi)
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

