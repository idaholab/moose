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
#include "MooseSystem.h"
#include "ArbitraryQuadrature.h"

#include "boundary_info.h"
#include "elem.h"
#include "plane.h"
#include "fe_interface.h"

PenetrationLocator::PenetrationLocator(MooseSystem & moose_system, Mesh & mesh, unsigned int master, unsigned int slave)
  :_moose_system(moose_system),
  _mesh(mesh),
  _master_boundary(master),
  _slave_boundary(slave)
{}


void
PenetrationLocator::detectPenetration()
{
  Moose::perf_log.push("detectPenetration()","Solve");

  // Data structures to hold the element boundary information
  std::vector< unsigned int > elem_list;
  std::vector< unsigned short int > side_list;
  std::vector< short int > id_list;

  // Retrieve the Element Boundary data structures from the mesh
  _mesh.boundary_info->build_side_list(elem_list, side_list, id_list);

  // Data strcutres to hold the Nodal Boundary conditions
  std::vector< unsigned int > node_list;
  std::vector< short int > node_boundary_list;
  _mesh.boundary_info->build_node_list_from_side_list();
  _mesh.boundary_info->build_node_list(node_list, node_boundary_list);

  const unsigned int n_nodes = node_list.size();
  const unsigned int n_elems = elem_list.size();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    unsigned int boundary_id = node_boundary_list[i];

    if(boundary_id == _slave_boundary)
    {
      Node & node = _mesh.node(node_list[i]);
/*      
      if(node.processor_id() == libMesh::processor_id())
      {
*/
        // See if we already have info about this node
        if(_penetration_info[node.id()])
        {
          PenetrationInfo * info = _penetration_info[node.id()];

          Elem * elem = info->_elem;
          Elem * side = info->_side;

          // See if the same element still contains this point
          if(elem->contains_point(node))
          {
            Point closest_point;
            RealVectorValue normal;

            info->_distance = normDistance(*elem, *side, node, closest_point, normal);
            info->_normal = normal;
            
            mooseAssert(info->_distance >= 0, "Error in PenetrationLocator: Slave node contained in element but contact distance was negative!");
            
            info->_closest_point = closest_point;
//            _has_penetrated[node.id()] = true;

            // I hate continues but this is actually cleaner than anything I can think of
            continue;
          }
          else
          {
            Point closest_point;            
            RealVectorValue normal;

            // See if this element still has the same one across from it
            Real distance = normDistance(*elem, *side, node, closest_point, normal);

            if(std::abs(distance) < 999999999)
            {
              info->_normal = normal;
              info->_distance = distance;
              mooseAssert(info->_distance <= 0, "Error in PenetrationLocator: Slave node not contained in element but distance was positive!");

              info->_closest_point = closest_point;

              continue;
            }
            else
            {
              delete _penetration_info[node.id()];
              _penetration_info[node.id()] = NULL;
            }
          }
        } 
          
        Node * closest_node = NULL;
        
        Real closest_distance = 999999999;
        
        for(unsigned int k=0; k<n_nodes; k++)  
        {
          short int other_boundary_id = node_boundary_list[k];

          if(other_boundary_id == _master_boundary)
          {
            Node * cur_node = _mesh.node_ptr(node_list[k]);
                                             
            Real distance = ((*cur_node) - node).size();

            if(distance < closest_distance)
            {
              closest_distance = distance;
              closest_node = cur_node;
            }
          }
        }

        std::vector<unsigned int> & closest_elems = _moose_system.node_to_elem_map[closest_node->id()];

        for(unsigned int j=0; j<closest_elems.size(); j++)
        {          
          unsigned int elem_id = closest_elems[j];
          Elem * elem = _mesh.elem(elem_id);
            
          for(unsigned int m=0; m<n_elems; m++)
          {
            if(elem_list[m] == elem_id && id_list[m] == _master_boundary)
            {
              unsigned int side_num = side_list[m];
              
              Elem *side = (elem->build_side(side_num)).release();

              Point closest_point;
              RealVectorValue normal;
              
              Real distance = normDistance(*elem, *side, node, closest_point, normal);

              if(_penetration_info[node.id()] && std::abs(_penetration_info[node.id()]->_distance) > std::abs(distance))
              {
                delete _penetration_info[node.id()];
                _penetration_info[node.id()] = NULL;
              }
                
              if(std::abs(distance) < 999999999 && (!_penetration_info[node.id()] || std::abs(_penetration_info[node.id()]->_distance) > std::abs(distance)))
              {
                _penetration_info[node.id()] =  new PenetrationInfo(&node,
                                                                    elem,
                                                                    side,
                                                                    normal,
                                                                    distance,
                                                                    closest_point);
              }
              else
              {
                delete side;
              }
            }
          }
        }
//      }
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

Real
PenetrationLocator::normDistance(const Elem & elem, const Elem & side, const Point & p0, Point & closest_point, RealVectorValue & normal)
{
  Real d;
  unsigned int dim = _mesh.mesh_dimension();

  Point p1 = side.point(0);    
  Point p2 = side.point(1);
  Point p3;
  
  if (dim == 2)
  {
    //Create a point that is just off in the z axis.
    //I do it this way so that it is about the same order of magnitude as the points themselves
    //TODO: Think about this some more!  Maybe need some absolute values.
    p3 = p1 + Point(0,0,p1(0)+p2(0)+p1(1)+p2(1));
  }
  else if (dim == 3)
    p3 = side.point(2);

  Plane p = Plane(p1, p2, p3);

  closest_point = p.closest_point(p0);

  // Make sure that the point is in the xy plane
  if(dim == 2)
    closest_point(2) = 0;

  if(elem.contains_point(closest_point))
  {
    d = (p0 - p.closest_point(p0)).size();
    if(p.above_surface(p0))
      d = -d;
    normal = p.unit_normal(closest_point);

    /********* Computes an Average Normal in 2D *************/

    if(dim == 2)
    {    
      Real dedge = 9999999999;
      unsigned int side_node_num = 0;
    
      for(unsigned int n=0; n<side.n_nodes(); n++)
      {
        Real cur_distance = (p0 - side.point(n)).size();
      
        if(cur_distance < dedge)
        {
          dedge = cur_distance;
          side_node_num = n;
        }
      }

      std::map<unsigned int, unsigned int> elems_to_sides;

      Real blend_length = side.hmax()*2e-1;
    
      if(dedge < blend_length)
      {
        Node * node = side.get_node(side_node_num);
        unsigned int node_id = node->id();

        std::vector<unsigned int> & elems_connected_to_node = _moose_system.node_to_elem_map[node_id];

        for(unsigned int i=0; i< elems_connected_to_node.size(); i++)
        {
          Elem * connected_elem = _mesh.elem(elems_connected_to_node[i]);

          elems_to_sides[connected_elem->id()] = _mesh.boundary_info->side_with_boundary_id(connected_elem, _master_boundary);
        }
      
        std::map<unsigned int, unsigned int>::iterator elems_it = elems_to_sides.begin();
        std::map<unsigned int, unsigned int>::iterator elems_end = elems_to_sides.end();

        Point neighbor_normal;
        Point my_normal;
      
        for(; elems_it != elems_end; ++elems_it)
        {
          Elem * connected_elem = _mesh.elem(elems_it->first);
        
          FEType fe_type;
          AutoPtr<FEBase> fe(FEBase::build(_moose_system.getDim(), fe_type));
          ArbitraryQuadrature arbitrary_qrule(_moose_system.getDim()-1, _moose_system._max_quadrature_order);
          fe->attach_quadrature_rule(&arbitrary_qrule);
          const std::vector<Point>& normals = fe->get_normals();
      
          { 
            Point mapped = libMesh::FEInterface::inverse_map (2, fe_type, connected_elem, *node);
            std::vector<Point> mapped_points;
            mapped_points.push_back(mapped);
            arbitrary_qrule.setPoints(mapped_points);
            fe->reinit(connected_elem, elems_it->second);
          }      

          if(connected_elem->id() == elem.id())
            my_normal = normals[0];
          else
            neighbor_normal = normals[0];
        }
      
        Real theta = (-0.5*(1.0/blend_length)*dedge)+0.5;
        normal = (neighbor_normal*theta)+(my_normal*(1-theta));
      }
    }
  }
  /*
  else if(elem.contains_point(p0))  // If the point is in the element but the plane point wasn't...
  {
    std::cout<<"junk!"<<std::endl;
    
    d = 9999999999;
    unsigned int neighbor_num = 0;
    for(unsigned int n=0; n<side.n_nodes(); n++)
    {
      Real cur_distance = (p0 - side.point(n)).size();
      if(cur_distance < d)
      {
        d = cur_distance;
        closest_point = side.point(n);
        neighbor_num = n;
        normal = closest_point - p0;
        normal /= normal.size();
      }
    }
  */

/*
//      std::cout<<"--"<<std::endl<<dedge<<std::endl<<elem.neighbor(neighbor_num)->id()<<std::endl<<elem.id()<<std::endl;
      Elem * neighbor = elem.neighbor(neighbor_num);

      unsigned int node_id = side.node(neighbor_num);
      Node * node = side.get_node(neighbor_num);

//      std::cout<<node->id()<<std::endl;
      
      

      unsigned int nside = 0;

      for(unsigned int ns=0; ns<neighbor->n_sides(); ns++)
      {
        if(neighbor->neighbor(ns) == NULL)
          nside = ns;
      }


//      std::cout<<nside<<std::endl;
      nside = 3;
      
      AutoPtr<Elem> neighbor_side(neighbor->build_side(nside));
      
      FEType fe_type;
      AutoPtr<FEBase> fe(FEBase::build(_moose_system.getDim(), fe_type));
      ArbitraryQuadrature arbitrary_qrule(_moose_system.getDim()-1, _moose_system._max_quadrature_order);
      fe->attach_quadrature_rule(&arbitrary_qrule);
      const std::vector<Point>& normals = fe->get_normals();
      
      { 
        Point mapped = libMesh::FEInterface::inverse_map (2, fe_type, neighbor, *node);
        std::vector<Point> mapped_points;
        mapped_points.push_back(mapped);
        arbitrary_qrule.setPoints(mapped_points);
        fe->reinit(neighbor, nside);
      }      

      Point neighbor_normal = normals[0];

      { 
        Point mapped = libMesh::FEInterface::inverse_map (2, fe_type, &elem, *node);
        std::vector<Point> mapped_points;
        mapped_points.push_back(mapped);
        arbitrary_qrule.setPoints(mapped_points);
        fe->reinit(&elem, nside);
      }      

      Point my_normal = normals[0];

//      std::cout<<"Neighbor: "<<neighbor_normal;
//      std::cout<<"Mine: "<<my_normal;


//      Real theta = (-0.5e4*dedge)+0.5;

      Real theta = 0.5;
      
      
//      std::cout<<"new: "<<(neighbor_normal*theta)+(my_normal*(1-theta))<<std::endl;
      normal = (neighbor_normal*theta)+(my_normal*(1-theta));

//      std::cout<<normal<<std::endl;
      
//      std::cout<<normal<<std::endl;
  }
*/
  else
  {
    d = 9999999999;
  }
  
  return d;
}


RealVectorValue
PenetrationLocator::penetrationNormal(unsigned int node_id)
{
  std::map<unsigned int, PenetrationInfo *>::const_iterator found_it;

  if ((found_it = _penetration_info.find(node_id)) != _penetration_info.end())
    return found_it->second->_normal;
  else
    return RealVectorValue(0, 0, 0);
}


PenetrationLocator::PenetrationInfo::PenetrationInfo(Node * node, Elem * elem, Elem * side, RealVectorValue norm, Real norm_distance, const Point & closest_point)
  : _node(node),
    _elem(elem),
    _side(side),
    _normal(norm),
    _distance(norm_distance),
    _closest_point(closest_point)
{}

  
PenetrationLocator::PenetrationInfo::PenetrationInfo(const PenetrationInfo & p)
  : _node(p._node),
    _elem(p._elem),
    _side(p._side),
    _normal(p._normal),
    _distance(p._distance),
    _closest_point(p._closest_point)
{}

PenetrationLocator::PenetrationInfo::~PenetrationInfo()
{
  delete _side;
}   
