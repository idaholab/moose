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

#include "XFEMGeometricCut3D.h"

#include "MooseError.h"
#include "XFEMFuncs.h"

XFEMGeometricCut3D::XFEMGeometricCut3D(Real t0, Real t1):
  XFEMGeometricCut(t0,t1),
  _center(Point(0.0,0.0,0.0)),
  _normal(Point(0.0,0.0,0.0))
{}

XFEMGeometricCut3D::~XFEMGeometricCut3D()
{}

bool
XFEMGeometricCut3D::cutElementByGeometry(const Elem* /*elem*/,
                                         std::vector<cutEdge> & /*cutEdges*/,
                                         Real /*time*/)
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
XFEMGeometricCut3D::cutElementByGeometry(const Elem* elem,
                                         std::vector<cutFace> & cutFaces,
                                         Real /*time*/)
//TODO: Time evolving cuts not yet supported in 3D (hence the lack of use of the time variable)
{
  bool cut_elem = false;

   int hex_indices[6][4] = {{0,3,2,1},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{4,5,6,7}};
   int tet_indices[4][3] = {{0,2,1},{0,1,3},{1,2,3},{2,0,3}};

   unsigned int n_face_nodes = 0;
   if(elem->n_nodes() == 8)
     n_face_nodes = 4;
   else if (elem->n_nodes() == 4)
     n_face_nodes = 3;
   else
     mooseError("this method only works for lienar hexes and tets");

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
          node_id1 = hex_indices[i][j];
          node_id2 = hex_indices[i][jplus1];
        }else if (elem->n_nodes() == 4) // tet
        {
          node_id1 = tet_indices[i][j];
          node_id2 = tet_indices[i][jplus1];
        }
        Node *node1 = elem->get_node(node_id1);
        Node *node2 = elem->get_node(node_id2);
        Point p1((*node1)(0), (*node1)(1), (*node1)(2));
        Point p2((*node2)(0), (*node2)(1), (*node2)(2));

        Point pint(0.0,0.0,0.0);
        if (intersectWithEdge(p1,p2,pint))
        {
          cut_edges.push_back(j);
          cut_pos.push_back(getRelativePosition(p1,p2,pint));
        }
      }

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

   }

   return cut_elem;
}

bool
XFEMGeometricCut3D::cutFragmentByGeometry(std::vector<std::vector<Point> > & /*frag_edges*/,
                                          std::vector<cutEdge> & /*cutEdges*/,
                                          Real /*time*/)
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}


bool
XFEMGeometricCut3D::cutFragmentByGeometry(std::vector<std::vector<Point> > & /*frag_faces*/,
                                          std::vector<cutFace> & /*cutFaces*/,
                                          Real /*time*/)
{
  //TODO: Need this for branching in 3D
  mooseError("cutFragmentByGeometry not yet implemented for 3D mesh cutting");
  return false;
}

bool
XFEMGeometricCut3D::intersectWithEdge(Point p1, Point p2, Point &pint)
{
  bool has_intersection = false;
  double plane_point[3] = {_center(0), _center(1), _center(2)};
  double plane_normal[3] = {_normal(0), _normal(1), _normal(2)};
  double edge_point1[3] = {p1(0), p1(1), p1(2)};
  double edge_point2[3] = {p2(0), p2(1), p2(2)};
  double cut_point[3] = {0.0,0.0,0.0};

  if(plane_normal_line_exp_int_3d(plane_point, plane_normal, edge_point1, edge_point2, cut_point) == 1) {
    Point temp_p(cut_point[0], cut_point[1], cut_point[2]);
    if( isInsideCutPlane(temp_p) && isInsideEdge(p1, p2, temp_p) )
    {
      pint = temp_p;
      has_intersection = true;
    }
  }
  return has_intersection;
}

bool
XFEMGeometricCut3D::isInsideEdge(Point p1, Point p2, Point p)
{
  Point p1_to_p2 = p2 - p1;
  Point p_to_p1 = p1 - p;
  Point p_to_p2 = p2 - p;
  Real dotp1 = p_to_p1*p1_to_p2;
  Real dotp2 = p_to_p2*p1_to_p2;
  if (dotp1*dotp2 <= 0.0)
    return true;
  else
    return false;
}

Real
XFEMGeometricCut3D::getRelativePosition(Point p1, Point p2, Point p)
{
  // get the relative position of p from p1
  Real full_len = std::sqrt((p2 - p1).size_sq());
  Real len_p1_p = std::sqrt((p - p1).size_sq());
  return len_p1_p/full_len;
}
