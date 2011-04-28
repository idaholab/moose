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

// Moose
#include "PenetrationThread.h"
#include "ParallelUniqueId.h"
#include "FindContactPoint.h"
#include "NearestNodeLocator.h"

// libmesh includes
#include "threads.h"

// Mutex to use when accessing _penetration_info;
Threads::spin_mutex pinfo_mutex;

PenetrationThread::PenetrationThread(const MeshBase & mesh,
                                     unsigned int master_boundary,
                                     unsigned int slave_boundary,
                                     std::map<unsigned int, PenetrationLocator::PenetrationInfo *> & penetration_info,
                                     bool update_location,
                                     std::vector<FEBase * > & fes,
                                     FEType & fe_type,
                                     NearestNodeLocator & nearest_node,
                                     std::vector<std::vector<unsigned int> > & node_to_elem_map,
                                     std::vector< unsigned int > & elem_list,
                                     std::vector< unsigned short int > & side_list,
                                     std::vector< short int > & id_list) :
  _mesh(mesh),
  _master_boundary(master_boundary),
  _slave_boundary(slave_boundary),
  _penetration_info(penetration_info),
  _update_location(update_location),
  _fes(fes),
  _fe_type(fe_type),
  _nearest_node(nearest_node),
  _node_to_elem_map(node_to_elem_map),
  _elem_list(elem_list),
  _side_list(side_list),
  _id_list(id_list),
  _n_elems(elem_list.size())
{
}

// Splitting Constructor
PenetrationThread::PenetrationThread(PenetrationThread & x, Threads::split /*split*/) :
  _mesh(x._mesh),
  _master_boundary(x._master_boundary),
  _slave_boundary(x._slave_boundary),
  _penetration_info(x._penetration_info),
  _update_location(x._update_location),
  _fes(x._fes),
  _fe_type(x._fe_type),
  _nearest_node(x._nearest_node),
  _node_to_elem_map(x._node_to_elem_map),
  _elem_list(x._elem_list),
  _side_list(x._side_list),
  _id_list(x._id_list),
  _n_elems(x._n_elems)
{
}

void
PenetrationThread::operator() (const NodeIdRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // Note this is called _fe for backward compatibility... it's actually _not_ a member variable!
  // TODO: Replace _fe with fe
  FEBase * _fe = _fes[_tid];

  for (NodeIdRange::const_iterator nd = range.begin() ; nd != range.end(); ++nd)
  {
    const Node & node = _mesh.node(*nd);

    // We're going to get a reference to the pointer for the pinfo for this node
    // This will alow us to manipulate this pointer without having to go through
    // the _penetration_info map... meaning this is the only mutex we'll have to do!
    pinfo_mutex.lock();
    PenetrationLocator::PenetrationInfo * & info = _penetration_info[node.id()];
    pinfo_mutex.unlock();
    
    // See if we already have info about this node
    if(info)
    {
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
          delete info;
          info = NULL;
        }
      }
    }

    const Node * closest_node = _nearest_node.nearestNode(node.id());
    std::vector<unsigned int> & closest_elems = _node_to_elem_map[closest_node->id()];
    std::vector<PenetrationLocator::PenetrationInfo*> p_info;


    for(unsigned int j=0; j<closest_elems.size(); j++)
    {
      unsigned int elem_id = closest_elems[j];
      Elem * elem = _mesh.elem(elem_id);

      // TODO: This is a horribly inefficient way to do this!  We need to cache information about which boundary ids elements are connected to
      // and which sides are on those boundaries in MooseMesh!  That way we can look this information up directly!
      for(unsigned int m=0; m<_n_elems; m++)
      {
        if(_elem_list[m] == elem_id && _id_list[m] == static_cast<short>(_master_boundary))
        {
          unsigned int side_num = _side_list[m];

          Elem *side = (elem->build_side(side_num,false)).release();

          Point contact_ref;
          Point contact_phys;
          Real distance;
          RealGradient normal;
          bool contact_point_on_side;
          std::vector<std::vector<Real> > side_phi;

          Moose::findContactPoint(_fe, _fe_type, elem, side_num, node, true, contact_ref, contact_phys, side_phi, distance, normal, contact_point_on_side);

          PenetrationLocator::PenetrationInfo * pen_info =  new PenetrationLocator::PenetrationInfo(&node,
                                                                                                    elem,
                                                                                                    side,
                                                                                                    side_num,
                                                                                                    normal,
                                                                                                    distance,
                                                                                                    contact_phys,
                                                                                                    contact_ref,
                                                                                                    side_phi);

          if(contact_point_on_side && info &&
             (
               (std::abs(info->_distance) > std::abs(distance)) ||
               (info->_distance < 0 && distance > 0)
               )
            )
          {
            delete info;
            info = NULL;
          }

          if(contact_point_on_side && (!info ||
                                       (
                                         (std::abs(info->_distance) > std::abs(distance)) ||
                                         (info->_distance < 0 && distance > 0)
                                         )
               )
            )
          {
            info = pen_info;
          }
          else
          {
            p_info.push_back( pen_info );
          }
        }
      }
    }
    if (!info && p_info.size() > 1)
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

            info = p_info[closest_index];
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
}

void
PenetrationThread::join(const PenetrationThread & /*other*/)
{}
