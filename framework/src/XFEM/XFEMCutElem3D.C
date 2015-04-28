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

#include <cstdlib> // *must* precede <cmath> for proper std:abs() on PGI, Sun Studio CC
#include <cmath> // for isnan(), when it's defined
#include "XFEMCutElem3D.h"
#include "EFAfuncs.h"

XFEMCutElem3D::XFEMCutElem3D(Elem* elem, const EFAelement3D * const CEMelem, unsigned int n_qpoints):
  XFEMCutElem(elem, n_qpoints),
  _efa_elem3d(CEMelem, true)
{
  calc_physical_volfrac();
  calc_mf_weights();
}

XFEMCutElem3D::~XFEMCutElem3D()
{}

Point
XFEMCutElem3D::get_node_coords(EFAnode* CEMnode, MeshBase* displaced_mesh) const
{
  Point node_coor(0.0,0.0,0.0);
  std::vector<EFAnode*> master_nodes;
  std::vector<Point> master_points;
  std::vector<double> master_weights;

  _efa_elem3d.getMasterInfo(CEMnode, master_nodes, master_weights);
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
  {
    if (master_nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
    {
      Node* node = _nodes[master_nodes[i]->id()];
      if (displaced_mesh)
        node = displaced_mesh->node_ptr(node->id());
      Point node_p((*node)(0), (*node)(1), (*node)(2));
      master_points.push_back(node_p);
    }
    else
    {
      libMesh::err << " ERROR: master nodes must be local" << std::endl;
      exit(1);
    }
  } // i
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
    node_coor += master_weights[i]*master_points[i];
  return node_coor;
}

void
XFEMCutElem3D::calc_physical_volfrac()
{
  Real frag_vol = 0.0;

  // collect fragment info needed by polyhedron_volume_3d()
  std::vector<std::vector<unsigned int> > frag_face_ix;
  std::vector<EFAnode*> frag_nodes;
  _efa_elem3d.get_fragment(0)->get_node_info(frag_face_ix, frag_nodes);
  int face_num = frag_face_ix.size();
  int node_num = frag_nodes.size();

  int order_max = 0;
  int order[face_num];
  for (unsigned int i = 0; i < face_num; ++i)
  {
    if (frag_face_ix[i].size() > order_max)
      order_max = frag_face_ix[i].size();
    order[i] = frag_face_ix[i].size();
  }

  double coord[3*node_num];
  for (unsigned int i = 0; i < frag_nodes.size(); ++i)
  {
    Point p = get_node_coords(frag_nodes[i]);
    coord[3*i + 0] = p(0);
    coord[3*i + 1] = p(1);
    coord[3*i + 2] = p(2);
  }

  int node[face_num*order_max];
  i4vec_zero(face_num*order_max, node);
  for (unsigned int i = 0; i < face_num; ++i)
    for (unsigned int j = 0; j < frag_face_ix[i].size(); ++j)
      node[order_max*i + j] = frag_face_ix[i][j];

  // compute fragment volume and volume fraction
  frag_vol = polyhedron_volume_3d(coord, order_max, face_num, node, node_num, order);
  _physical_volfrac = frag_vol/_elem_volume;
}

void
XFEMCutElem3D::calc_mf_weights()
{
  // TODO: 3D moment fitting method nod coded yet - use volume fraction for now
  _new_weights.resize(_n_qpoints, _physical_volfrac);
}

Point
XFEMCutElem3D::get_origin(unsigned int plane_id, MeshBase* displaced_mesh) const
{
  Point orig(0.0,0.0,0.0);
  std::vector<std::vector<EFAnode*> > cut_plane_nodes;
  for (unsigned int i = 0; i < _efa_elem3d.get_fragment(0)->num_faces(); ++i)
  {
    if (_efa_elem3d.get_fragment(0)->is_face_interior(i))
    {
      EFAface* face = _efa_elem3d.get_fragment(0)->get_face(i);
      std::vector<EFAnode*> node_line;
      for (unsigned int j = 0; j < face->num_nodes(); ++j)
        node_line.push_back(face->get_node(j));
      cut_plane_nodes.push_back(node_line);
    }
  } // i
  if (cut_plane_nodes.size() == 0)
  {
    libMesh::err << " ERROR: no cut plane found in this element"<<std::endl;
    exit(1);
  }
  if (plane_id < cut_plane_nodes.size()) // valid plane_id
  {
    std::vector<Point> cut_plane_points;
    for (unsigned int i = 0; i < cut_plane_nodes[plane_id].size(); ++i)
      cut_plane_points.push_back(get_node_coords(cut_plane_nodes[plane_id][i], displaced_mesh));

    for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
      orig += cut_plane_points[i];
    orig *= (1.0/cut_plane_points.size());
  }
  return orig;
}

Point
XFEMCutElem3D::get_normal(unsigned int plane_id, MeshBase* displaced_mesh) const
{
  Point normal(0.0,0.0,0.0);
  std::vector<std::vector<EFAnode*> > cut_plane_nodes;
  for (unsigned int i = 0; i < _efa_elem3d.get_fragment(0)->num_faces(); ++i)
  {
    if (_efa_elem3d.get_fragment(0)->is_face_interior(i))
    {
      EFAface* face = _efa_elem3d.get_fragment(0)->get_face(i);
      std::vector<EFAnode*> node_line;
      for (unsigned int j = 0; j < face->num_nodes(); ++j)
        node_line.push_back(face->get_node(j));
      cut_plane_nodes.push_back(node_line);
    }
  } // i
  if (cut_plane_nodes.size() == 0)
  {
    libMesh::err << " ERROR: no cut plane found in this element"<<std::endl;
    exit(1);
  }
  if (plane_id < cut_plane_nodes.size()) // valid plane_id
  {
    std::vector<Point> cut_plane_points;
    for (unsigned int i = 0; i < cut_plane_nodes[plane_id].size(); ++i)
      cut_plane_points.push_back(get_node_coords(cut_plane_nodes[plane_id][i], displaced_mesh));

    Point center(0.0,0.0,0.0);
    for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
      center += cut_plane_points[i];
    center *= (1.0/cut_plane_points.size());

    for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
    {
      unsigned int iplus1(i < cut_plane_points.size()-1 ? i+1 : 0);
      Point ray1 = cut_plane_points[i] - center;
      Point ray2 = cut_plane_points[iplus1] - center;
      normal += cross_product(ray1, ray2);
    }
    normal *= (1.0/cut_plane_points.size());
  }
  normalize(normal);
  return normal;
}

const EFAelement*
XFEMCutElem3D::get_efa_elem() const
{
  return &_efa_elem3d;
}

unsigned int
XFEMCutElem3D::num_cut_planes() const
{
  unsigned int counter = 0;
  for (unsigned int i = 0; i < _efa_elem3d.get_fragment(0)->num_faces(); ++i)
    if (_efa_elem3d.get_fragment(0)->is_face_interior(i))
      counter += 1;
  return counter;
}

// ****** private geometry toolkit ******
double
XFEMCutElem3D::polyhedron_volume_3d(double coord[], int order_max, int face_num,
                                    int node[], int node_num, int order[]) const
//****************************************************************************80
//
//  Purpose:
//
//    POLYHEDRON_VOLUME_3D computes the volume of a polyhedron in 3D.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    21 August 2003
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, double COORD[NODE_NUM*3], the 3D coordinates of the vertices.
//    The vertices may be listed in any order.
//
//    Input, int ORDER_MAX, the maximum number of vertices that make
//    up a face of the polyhedron.
//
//    Input, int FACE_NUM, the number of faces of the polyhedron.
//
//    Input, int NODE[FACE_NUM*ORDER_MAX].  Face I is defined by
//    the vertices NODE(I,1) through NODE(I,ORDER(I)).  These vertices
//    are listed in neighboring order.
//
//    Input, int NODE_NUM, the number of points stored in COORD.
//
//    Input, int ORDER[FACE_NUM], the number of vertices making up
//    each face.
//
//    Output, double POLYHEDRON_VOLUME_3D, the volume of the polyhedron.
//
{
# define DIM_NUM 3

  int face;
  int n1;
  int n2;
  int n3;
  double term;
  int v;
  double volume;
  double x1;
  double x2;
  double x3;
  double y1;
  double y2;
  double y3;
  double z1;
  double z2;
  double z3;
//
  volume = 0.0;
//
//  Triangulate each face.
//
  for ( face = 0; face < face_num; face++ )
  {
    n3 = node[order[face]-1+face*order_max];
    x3 = coord[0+n3*3];
    y3 = coord[1+n3*3];
    z3 = coord[2+n3*3];

    for ( v = 0; v < order[face] - 2; v++ )
    {
      n1 = node[v+face*order_max];
      x1 = coord[0+n1*3];
      y1 = coord[1+n1*3];
      z1 = coord[2+n1*3];

      n2 = node[v+1+face*order_max];
      x2 = coord[0+n2*3];
      y2 = coord[1+n2*3];
      z2 = coord[2+n2*3];

      term = x1 * y2 * z3 - x1 * y3 * z2
           + x2 * y3 * z1 - x2 * y1 * z3
           + x3 * y1 * z2 - x3 * y2 * z1;

      volume = volume + term;
    }

  }

  volume = volume / 6.0;

  return volume;
# undef DIM_NUM
}

void
XFEMCutElem3D::i4vec_zero ( int n, int a[] ) const
//****************************************************************************80
//
//  Purpose:
//
//    I4VEC_ZERO zeroes an I4VEC.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license.
//
//  Modified:
//
//    01 August 2005
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Input, int N, the number of entries in the vector.
//
//    Output, int A[N], a vector of zeroes.
//
{
  int i;

  for ( i = 0; i < n; i++ )
  {
    a[i] = 0;
  }
  return;
}
