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

#include "libmesh/mesh_base.h"
#include "XFEM_circle_cut.h"
#include "EFAfuncs.h"
#include "XFEMMiscFuncs.h"

XFEM_circle_cut::XFEM_circle_cut(std::vector<Real> circle_nodes):
   XFEM_geometric_cut(0.0, 0.0), 
   _vertices(2, Point(0.0,0.0,0.0)),
   _center(Point(0.0,0.0,0.0)),
   _normal(Point(0.0,0.0,0.0)),
   _radius(0.0),
   _angle(0.0)
{
   _center   = Point(circle_nodes[0], circle_nodes[1], circle_nodes[2]);
  _vertices[0]  = Point(circle_nodes[3], circle_nodes[4], circle_nodes[5]);
  _vertices[1]  = Point(circle_nodes[6], circle_nodes[7], circle_nodes[8]);
  
  Point ray1 = _vertices[0] - _center;
  Point ray2 = _vertices[1] - _center;
  
  _normal = ray1.cross(ray2);
  normalize(_normal);
  
  Real R1 = std::sqrt(ray1.size_sq());
  Real R2 = std::sqrt(ray2.size_sq());
  if( std::abs(R1 - R2) > 1e-10 ) 
    mooseError("this method only works for a circular cut\n");
 
   _radius = 0.5*(R1 + R2);
   _angle = std::acos( (ray1*ray2)/(R1*R2) );
}

XFEM_circle_cut::~XFEM_circle_cut()
{}

bool XFEM_circle_cut::cut_elem_by_geometry(const Elem* elem, std::vector<cutEdge> & cutEdges, Real time)
{
  mooseError("invalid method for 3D mesh cutting\n");
  return false;
}

bool XFEM_circle_cut::cut_elem_by_geometry(const Elem* elem, std::vector<cutFace> & cutFaces, Real time)
{
  bool cut_elem = false;
 
   int hex_ix[6][4] = {{0,3,2,1},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{4,5,6,7}};
   int tet_ix[4][3] = {{0,2,1},{0,1,3},{1,2,3},{2,0,3}};
  
   unsigned int n_face_nodes = 0;
   if(elem->n_nodes() == 8)
     n_face_nodes = 4;
   else if (elem->n_nodes() == 4)
     n_face_nodes = 3;
   else
     mooseError("this method only works for lienar hexes and tets\n");

    for(unsigned int i=0; i < elem->n_sides(); i++)
    {
      std::vector<unsigned int> cut_edges;
      std::vector<Real> cut_pos;

      for(unsigned int j=0; j < n_face_nodes; j++)
      {
        unsigned int jplus1(j < (n_face_nodes-1) ? (j+1) : 0);
        unsigned int node_id1 = 0;
        unsigned int node_id2 = 0;
        if(elem->n_nodes() == 8)  // hex
        { 
          node_id1 = hex_ix[i][j];
          node_id2 = hex_ix[i][jplus1];
        }else if (elem->n_nodes() == 4) // tet
        {
          node_id1 = tet_ix[i][j];
          node_id2 = tet_ix[i][jplus1]; 
        }
        Node *node1 = elem->get_node(node_id1);
        Node *node2 = elem->get_node(node_id2);
        Point p1((*node1)(0), (*node1)(1), (*node1)(2));
        Point p2((*node2)(0), (*node2)(1), (*node2)(2));

        Point pint(0.0,0.0,0.0);
        if (intersect_with_edge(p1,p2,pint))
        {
          cut_edges.push_back(j);
          cut_pos.push_back(getRelativePosition(p1,p2,pint));
	}
      } // j, loop over face edges

      if (cut_edges.size() == 2)
      {
       cut_elem = true;
       cutFace mycut;
       mycut.face_id = i;
       mycut.face_edge.push_back(cut_edges[0]);
       mycut.face_edge.push_back(cut_edges[1]);
       mycut.position.push_back(cut_pos[0]);
       mycut.position.push_back(cut_pos[1]);
       cutFaces.push_back(mycut);
     }

   }  // i, loop over faces; 
  
   return cut_elem;
}	


bool XFEM_circle_cut::cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_edges, std::vector<cutEdge> & cutEdges, Real time)
{
  mooseError("invalid method for 3D mesh cutting\n");
  return false;
}


bool XFEM_circle_cut::cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_faces, std::vector<cutFace> & cutFaces, Real time)
{
  return false;
}

bool XFEM_circle_cut::intersect_with_edge(Point p1, Point p2, Point &pint)
{
  bool has_inters = false;
  double plane_point[3] = {_center(0), _center(1), _center(2)};
  double plane_normal[3] = {_normal(0), _normal(1), _normal(2)};
  double edge_point1[3] = {p1(0), p1(1), p1(2)};
  double edge_point2[3] = {p2(0), p2(1), p2(2)};
  double cut_point[3] = {0.0,0.0,0.0};
 
  if(plane_normal_line_exp_int_3d(plane_point, plane_normal, edge_point1, edge_point2, cut_point) == 1) {
    Point temp_p(cut_point[0], cut_point[1], cut_point[2]);
    if( isInsideCutPlane(temp_p) && isInsideEdge(p1, p2, temp_p) ){
      pint = temp_p;
       has_inters = true;
    }
  }
  return has_inters;
}


bool XFEM_circle_cut::isInsideCutPlane(Point p){
    Point ray = p - _center; 
    if( std::abs(ray*_normal)<1e-15 && std::sqrt(ray.size_sq()) < _radius )   
	return true;
    return false;
}

