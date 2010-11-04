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

#include "boundary_info.h"
#include "elem.h"
#include "plane.h"

PenetrationLocator::PenetrationLocator(MooseSystem & moose_system, Mesh & mesh, std::vector<unsigned int> master, short int slave)
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

  std::vector<unsigned int>::iterator master_begin = _master_boundary.begin();
  std::vector<unsigned int>::iterator master_end   = _master_boundary.end();

  for(unsigned int i=0; i<n_nodes; i++)
  {
    unsigned int boundary_id = node_boundary_list[i];

    if(std::find(master_begin, master_end, boundary_id) != master_end)
    {
      Node & node = _mesh.node(node_list[i]);
      
      if(node.processor_id() == libMesh::processor_id())
      {
        // See if we already have info about this node
        if(_penetration_info[node.id()])
        {
          PenetrationInfo * info = _penetration_info[node.id()];

          Elem * elem = info->_elem;
          Elem * side = info->_side;

          // See if the same element still contains this point
          if(elem->contains_point(node))
          {
            info->_normal = normal(*side, node);
            info->_distance = normDistance(*elem, *side, node);

            // I hate continues but this is actually cleaner than anything I can think of
            continue;
          }
        } 
          
        Node * closest_node = NULL;
        
        Real closest_distance = 999999999;
        
        for(unsigned int k=0; k<n_nodes; k++)  
        {
          short int other_boundary_id = node_boundary_list[k];

          if(other_boundary_id == _slave_boundary)
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
            if(elem_list[m] == elem_id && id_list[m] == _slave_boundary)
            {
              unsigned int side_num = side_list[m];
              
              Elem *side = (elem->build_side(side_num)).release();

              Real distance = normDistance(*elem, *side, node);

              if(_penetration_info[node.id()] && std::abs(_penetration_info[node.id()]->_distance) > std::abs(distance))
              {
                delete _penetration_info[node.id()];
                _penetration_info[node.id()] = NULL;
              }
                
              if(std::abs(distance) < 999999999)
                _penetration_info[node.id()] =  new PenetrationInfo(elem,
                                                                    side,
                                                                    normal(*side, node),
                                                                    distance);
            }
          }
          
          
        }
      }
    }
  }        

  Moose::perf_log.pop("detectPenetration()","Solve");
}

RealVectorValue
PenetrationLocator::normal(const Elem & side, const Point & /*p0*/)
{
  
  unsigned int dim = _mesh.mesh_dimension();

  if (dim == 2)
  {
    // TODO: Does this always work? Does the ordering of the points
    // work out such that we always obtain the correct direction vector?
    RealVectorValue b = (side.point(0) - side.point(1)).unit();

    return RealVectorValue (-b(1), b(0),  0);
  }
  else if (dim == 3)
    // TODO
    return RealVectorValue ();
  else
    mooseError("Unsupported dimension");
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
PenetrationLocator::normDistance(const Elem & elem, const Elem & side, const Point & p0)
{
  Real d;
  unsigned int dim = _mesh.mesh_dimension();

  Point p1 = side.point(0);
  Point p2 = side.point(1);
  Point p3;

  if (dim == 2)
  {
//    libmesh_assert(side->n_points() == 2);

//    Point p1 = side.point(0);
//    Point p2 = side.point(1);
    p3 = p1 + Point(0,0,p1(0)+p2(0)+p1(1)+p2(1));

//    std::cerr << "\nLine Segment: (" << p1(0) << "," << p1(1) << ") - (" << p2(0) << "," << p2(1) << ") "
//ç              << "Point: (" << p0(0) << "," << p0(1) << ")\n";
    
    
//    d = std::sqrt(std::pow(((p2(1)-p1(1))*(p0(0)-p1(0))-(p2(0)-p1(0))*(p0(1)-p1(1))),2) /
//                   (std::pow(p2(0)-p1(0),2) + std::pow(p2(1)-p1(1),2)));

    // From http://local.wasp.uwa.edu.au/%7Epbourke/geometry/pointline/
/*    Real u = ((p0(0)-p1(0))*(p2(0)-p1(0)) + (p0(1)-p1(1))*(p2(1)-p1(1))) / (p2 - p1).size();

    // See if the intersection is on the line segment
    if(u >= 0 && u<=1)
    {
      Point closest_point(p1(0) + u*(p2(0)-p1(0)), p1(1) + u*(p2(1) - p1(1)));
      d = (p0 - closest_point).size();
    }
    else
      d = 9999999999;
*/
  }
  else if (dim == 3)
  {
//    libmesh_assert(side.n_points() >= 3);

    // TODO: Fix Plane linking problem
    
//    d = 0.0;

    p3 = side.point(2);
    
  }

  Plane p = Plane(p1, p2, p3);

  Point closest_point = p.closest_point(p0);

  // Make sure that the point is in the xy plane
  if(dim == 2)
    closest_point(2) = 0;

  if(elem.contains_point(closest_point))
    d = (p0 - p.closest_point(p0)).size();
  else
  {
    d = 9999999999;
  }

  if(p.above_surface(p0))
    d = -d;

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




// Copyright 2001, softSurfer (www.softsurfer.com)
// This code may be freely used and modified for any purpose
// providing that this copyright notice is included with it.
// SoftSurfer makes no warranty for this code, and cannot be held
// liable for any real or imagined damage resulting from its use.
// Users of this code must verify correctness for their application.

// Assume that classes are already given for the objects:
//    Point and Vector with
//        coordinates {float x, y, z;}
//        operators for:
//            == to test equality
//            != to test inequality
//            Point  = Point ± Vector
//            Vector = Point - Point
//            Vector = Scalar * Vector    (scalar product)
//            Vector = Vector * Vector    (3D cross product)
//    Line and Ray and Segment with defining points {Point P0, P1;}
//        (a Line is infinite, Rays and Segments start at P0)
//        (a Ray extends beyond P1, but a Segment ends at P1)
//    Plane with a point and a normal {Point V0; Vector n;}
//===================================================================

#define SMALL_NUM  0.00000001 // anything that avoids division overflow
// dot product (3D) which allows vector operations in arguments
#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
//#define perp(u,v)  ((u).x * (v).y - (u).y * (v).x)  // perp product (2D)
#define perp(u,v)  (u(0) * v(1) - u(1) * v(0)) 

// intersect2D_2Segments(): the intersection of 2 finite 2D segments
//    Input:  two finite segments S1 and S2
//    Output: *I0 = intersect point (when it exists)
//            *I1 = endpoint of intersect segment [I0,I1] (when it exists)
//    Return: 0=disjoint (no intersect)
//            1=intersect in unique point I0
//            2=overlap in segment from I0 to I1
int
PenetrationLocator::intersect2D_Segments( Point S1P0, Point S1P1, Point S2P0, Point S2P1, Point* I0, Point* I1 )
{
  VectorValue<Real> u = S1P1 - S1P0;
  VectorValue<Real> v = S2P1 - S2P0;
  VectorValue<Real> w = S1P0 - S2P0;
  Real D = perp(u,v);

    // test if they are parallel (includes either being a point)
  if (std::fabs(D) < SMALL_NUM) {          // S1 and S2 are parallel
    if (perp(u,w) != 0 || perp(v,w) != 0) {
      return 0;                   // they are NOT collinear
    }
    // they are collinear or degenerate
    // check if they are degenerate points
    Real du = u * u;
    Real dv = v * v;
    
    if (du==0 && dv==0) {           // both segments are points
      if (S1P0 != S2P0)         // they are distinct points
        return 0;
      *I0 = S1P0;                // they are the same point
      return 1;
    }
    if (du==0) {                    // S1 is a single point
      if (inSegment(S1P0, S2P0, S2P1) == 0)  // but is not in S2
        return 0;
      *I0 = S1P0;
      return 1;
    }
    if (dv==0) {                    // S2 a single point
      if (inSegment(S2P0, S1P0, S1P1) == 0)  // but is not in S1
        return 0;
      *I0 = S2P0;
      return 1;
    }
    // they are collinear segments - get overlap (or not)
    Real t0, t1;                   // endpoints of S1 in eqn for S2
    VectorValue<Real> w2 = S1P1 - S2P0;
    if (v(0) != 0) {
      t0 = w(0) / v(0);
      t1 = w2(0) / v(0);
    }
    else {
      t0 = w(1) / v(1);
      t1 = w2(1) / v(1);
    }
    if (t0 > t1) {                  // must have t0 smaller than t1
      Real t=t0; t0=t1; t1=t;    // swap if not
    }
    if (t0 > 1 || t1 < 0) {
      return 0;     // NO overlap
    }
    t0 = t0<0? 0 : t0;              // clip to min 0
    t1 = t1>1? 1 : t1;              // clip to max 1
    if (t0 == t1) {                 // intersect is a point
      *I0 = S2P0 + t0 * v;
      return 1;
    }

    // they overlap in a valid subsegment
    *I0 = S2P0 + t0 * v;
    *I1 = S2P0 + t1 * v;
    return 2;
  }

  // the segments are skew and may intersect in a point
  // get the intersect parameter for S1
  Real sI = perp(v,w) / D;
  if (sI < 0 || sI > 1)               // no intersect with S1
    return 0;

  // get the intersect parameter for S2
  Real     tI = perp(u,w) / D;
  if (tI < 0 || tI > 1)               // no intersect with S2
    return 0;

  *I0 = S1P0 + sI * u;               // compute S1 intersect point
  return 1;
}

//===================================================================

// inSegment(): determine if a point is inside a segment
//    Input:  a point P, and a collinear segment S
//    Return: 1 = P is inside S
//            0 = P is not inside S
int
PenetrationLocator::inSegment(Point P, Point SP0, Point SP1)
{
  if (SP0(0) != SP1(0)) {    // S is not vertical
    if (SP0(0) <= P(0) && P(0) <= SP1(0))
      return 1;
    if (SP0(0) >= P(0) && P(0) >= SP1(0))
      return 1;
  }
  else {    // S is vertical, so test y coordinate
    if (SP0(1) <= P(1) && P(1) <= SP1(1))
      return 1;
    if (SP0(1) >= P(1) && P(1) >= SP1(1))
      return 1;
  }
  return 0;
}
//===================================================================


PenetrationLocator::PenetrationInfo::PenetrationInfo(Elem * elem, Elem * side, RealVectorValue norm, Real norm_distance)
  : _elem(elem),
    _side(side),
    _normal(norm),
    _distance(norm_distance)
{}

  
PenetrationLocator::PenetrationInfo::PenetrationInfo(const PenetrationInfo & p)
  : _elem(p._elem),
    _side(p._side),
    _normal(p._normal),
    _distance(p._distance)
{}

PenetrationLocator::PenetrationInfo::~PenetrationInfo()
{
  delete _side;
}   
