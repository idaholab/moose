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
#include "XFEM_square_cut.h"

XFEM_square_cut::XFEM_square_cut(std::vector<Real> square_nodes):
  XFEM_geometric_cut(0.0, 0.0),
  _v1(Point(square_nodes[0], square_nodes[1], square_nodes[2])),
  _v2(Point(square_nodes[3], square_nodes[4], square_nodes[5])),
  _v3(Point(square_nodes[6], square_nodes[7], square_nodes[8])),
  _v4(Point(square_nodes[9], square_nodes[10], square_nodes[11]))
{
  _center = 0.25*(_v1 + _v2 + _v3 + _v4);
  Point v1_to_v3 = _v3 - _v1;
  Point v2_to_v4 = _v4 - _v2;
  _normal = cross_product(v1_to_v3, v2_to_v4);
  normalize(_normal);
}

XFEM_square_cut::~XFEM_square_cut()
{}

bool
XFEM_square_cut::cut_elem_by_geometry(const Elem* elem, std::vector<cutEdge> & cutEdges, Real time)
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
XFEM_square_cut::cut_elem_by_geometry(const Elem* elem, std::vector<cutFace> & cutFaces, Real time)
{
  bool cut_elem = false;
  int hex_ix[6][4] = {{0,3,2,1},{0,1,5,4},{1,2,6,5},{2,3,7,6},{3,0,4,7},{4,5,6,7}};
  int tet_ix[4][3] = {{0,2,1},{0,1,3},{1,2,3},{2,0,3}};

  unsigned int n_face_nodes = 0;
  if (elem->n_nodes() == 8)
    n_face_nodes = 4;
  else if (elem->n_nodes() == 4)
    n_face_nodes = 3;
  else
    mooseError("this method only works for linear hexes and tets");

  for (unsigned int i = 0; i < elem->n_sides(); ++i)
  {
    std::vector<unsigned int> cut_edges;
    std::vector<Real> cut_pos;
    for (unsigned int j = 0; j < n_face_nodes; ++j)
    {
      unsigned int jplus1(j < (n_face_nodes-1) ? (j+1) : 0);
      unsigned int node_id1 = 0;
      unsigned int node_id2 = 0;
      if (elem->n_nodes() == 8) // hex
      {
        node_id1 = hex_ix[i][j];
        node_id2 = hex_ix[i][jplus1];
      }
      else if (elem->n_nodes() == 4) // tet
      {
        node_id1 = tet_ix[i][j];
        node_id2 = tet_ix[i][jplus1];
      }
      Node *node1 = elem->get_node(node_id1);
      Node *node2 = elem->get_node(node_id2);
      Point p1((*node1)(0), (*node1)(1), (*node1)(2));
      Point p2((*node2)(0), (*node2)(1), (*node2)(2));
      Point pint(0.0,0.0,0.0);
      if (intersect_with_edge(p1, p2, pint))
      {
        cut_edges.push_back(j);
        cut_pos.push_back(getRelativePosition(p1, p2, pint));
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
  } // i, loop over faces
  return cut_elem;
}

bool
XFEM_square_cut::cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_edges,
                                      std::vector<cutEdge> & cutEdges, Real time)
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
XFEM_square_cut::cut_frag_by_geometry(std::vector<std::vector<Point> > & frag_faces,
                                      std::vector<cutFace> & cutFaces, Real time)
{
  // TODO: finish this for 3D problems with branching
  return false;
}

// private methods below
bool
XFEM_square_cut::intersect_with_edge(Point p1, Point p2, Point &pint)
{
  bool has_inters = false;
  double plane_point[3] = {_center(0), _center(1), _center(2)};
  double plane_normal[3] = {_normal(0), _normal(1), _normal(2)};
  double edge_point1[3] = {p1(0), p1(1), p1(2)};
  double edge_point2[3] = {p2(0), p2(1), p2(2)};
  double cut_point[3] = {0.0,0.0,0.0};
  if (plane_normal_line_exp_int_3d(plane_point, plane_normal, edge_point1, edge_point2, cut_point) == 1)
  {
    Point temp_p(cut_point[0], cut_point[1], cut_point[2]);
    if (isInsideCutPlane(temp_p) && isInsideEdge(p1, p2, temp_p))
    {
      pint = temp_p;
      has_inters = true;
    }
  }
  return has_inters;
}

int
XFEM_square_cut::plane_normal_line_exp_int_3d(double pp[3], double normal[3], double p1[3], double p2[3], double pint[3])
{
//    John Burkardt geometry.cpp
//  Parameters:
//
//    Input, double PP[3], a point on the plane.
//
//    Input, double NORMAL[3], a normal vector to the plane.
//
//    Input, double P1[3], P2[3], two distinct points on the line.
//
//    Output, double PINT[3], the coordinates of a
//    common point of the plane and line, when IVAL is 1 or 2.
//
//    Output, integer PLANE_NORMAL_LINE_EXP_INT_3D, the kind of intersection;
//    0, the line and plane seem to be parallel and separate;
//    1, the line and plane intersect at a single point;
//    2, the line and plane seem to be parallel and joined.
# define DIM_NUM 3

  double direction[DIM_NUM];
  int ival;
  double temp;
  double temp2;
//
//  Make sure the line is not degenerate.
  if (line_exp_is_degenerate_nd(DIM_NUM, p1, p2))
  {
    std::cerr << "\n";
    std::cerr << "PLANE_NORMAL_LINE_EXP_INT_3D - Fatal error!\n";
    std::cerr << "  The line is degenerate.\n";
    exit ( 1 );
  }
//
//  Make sure the plane normal vector is a unit vector.
  temp = r8vec_norm(DIM_NUM, normal);
  if (temp == 0.0)
  {
    std::cerr << "\n";
    std::cerr << "PLANE_NORMAL_LINE_EXP_INT_3D - Fatal error!\n";
    std::cerr << "  The normal vector of the plane is degenerate.\n";
    exit ( 1 );
  }

  for (unsigned int i = 0; i < DIM_NUM; ++i)
    normal[i] = normal[i]/temp;
//
//  Determine the unit direction vector of the line.
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    direction[i] = p2[i] - p1[i];
  temp = r8vec_norm(DIM_NUM, direction);

  for (unsigned int i = 0; i < DIM_NUM; ++i)
    direction[i] = direction[i]/temp;
//
//  If the normal and direction vectors are orthogonal, then
//  we have a special case to deal with.
  if (r8vec_dot_product(DIM_NUM, normal, direction) == 0.0)
  {
    temp = 0.0;
    for (unsigned int i = 0; i < DIM_NUM; ++i)
      temp = temp + normal[i]*(p1[i] - pp[i]);

    if ( temp == 0.0 )
    {
      ival = 2;
      r8vec_copy(DIM_NUM, p1, pint);
    }
    else
    {
      ival = 0;
      for (unsigned int i = 0; i < DIM_NUM; ++i)
        pint[i] = 1.0e20; // dummy huge value
    }
    return ival;
  }
//
//  Determine the distance along the direction vector to the intersection point.
  temp = 0.0;
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    temp = temp + normal[i]*(pp[i] - p1[i]);
  temp2 = 0.0;
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    temp2 = temp2 + normal[i]*direction[i];

  ival = 1;
  for (unsigned int i = 0; i < DIM_NUM; ++i)
    pint[i] = p1[i] + temp*direction[i]/temp2;

  return ival;
# undef DIM_NUM
}

bool
XFEM_square_cut::line_exp_is_degenerate_nd(int dim_num, double p1[], double p2[])
{
//    John Burkardt geometry.cpp
  bool value;
  value = r8vec_eq(dim_num, p1, p2);
  return value;
}

double
XFEM_square_cut::r8vec_norm(int n, double a[])
{
//    John Burkardt geometry.cpp
  double v = 0.0;
  for (unsigned int i = 0; i < n; ++i)
    v = v + a[i] * a[i];
  v = std::sqrt(v);
  return v;
}

void
XFEM_square_cut::r8vec_copy(int n, double a1[], double a2[])
{
//    John Burkardt geometry.cpp
  for (unsigned int i = 0; i < n; ++i)
    a2[i] = a1[i];
  return;
}

bool
XFEM_square_cut::r8vec_eq (int n, double a1[], double a2[])
{
//    John Burkardt geometry.cpp
  for (unsigned int i = 0; i < n; ++i)
    if ( a1[i] != a2[i] )
      return false;
  return true;
}

double
XFEM_square_cut::r8vec_dot_product(int n, double a1[], double a2[])
{
//    John Burkardt geometry.cpp
  double value = 0.0;
  for (unsigned int i = 0; i < n; ++i)
    value += a1[i] * a2[i];
  return value;
}

Point
XFEM_square_cut::cross_product(Point p1, Point p2)
{
  Point r(0.0,0.0,0.0);
  r(0) = p1(1)*p2(2) - p1(2)*p2(1);
  r(1) = p1(2)*p2(0) - p1(0)*p2(2);
  r(2) = p1(0)*p2(1) - p1(1)*p2(0);
  return r;
}

Real
XFEM_square_cut::dot_product(Point p1, Point p2)
{
  return p1(0)*p2(0) + p1(1)*p2(1) + p1(2)*p2(2);
}

bool
XFEM_square_cut::isInsideCutPlane(Point p)
{
  bool inside = false;
  std::vector<Point> square_nodes;
  square_nodes.push_back(_v1);
  square_nodes.push_back(_v2);
  square_nodes.push_back(_v3);
  square_nodes.push_back(_v4);

  unsigned int counter = 0;
  for (unsigned int i = 0; i < 4; ++i)
  {
    unsigned int iplus1(i < 3 ? i+1 : 0);
    Point middle2p = p - 0.5*(square_nodes[i] + square_nodes[iplus1]);
    Point side_tang = square_nodes[iplus1] - square_nodes[i];
    Point side_norm = cross_product(side_tang, _normal);
    normalize(middle2p); // normalize
    normalize(side_norm); // normalize
    Real dotp = dot_product(middle2p, side_norm);
    if (dotp <= 0.0)
      counter += 1;
  }
  if (counter == 4)
    inside = true;
  return inside;
}

bool
XFEM_square_cut::isInsideEdge(Point p1, Point p2, Point p)
{
  Point p1_to_p2 = p2 - p1;
  Point p_to_p1 = p1 - p;
  Point p_to_p2 = p2 - p;
  Real dotp1 = dot_product(p_to_p1, p1_to_p2);
  Real dotp2 = dot_product(p_to_p2, p1_to_p2);
  if (dotp1*dotp2 <= 0.0)
    return true;
  else
    return false;
}

Real
XFEM_square_cut::getRelativePosition(Point p1, Point p2, Point p)
{
  // get the relative position of p from p1
  Real full_len = std::sqrt((p2 - p1).size_sq());
  Real len_p1_p = std::sqrt((p - p1).size_sq());
  return len_p1_p/full_len;
}

void
XFEM_square_cut::normalize(Point & p)
{
  Real len = std::sqrt(p.size_sq());
  if (len != 0.0)
    p = (1.0/len)*p;
}
