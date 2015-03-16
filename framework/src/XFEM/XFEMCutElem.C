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
#include "XFEMCutElem.h"

XFEMCutElem::XFEMCutElem(Elem* elem, const EFAelement * const CEMelem):
  _n_nodes(elem->n_nodes()),
  _nodes(_n_nodes,NULL),
  _efa_elem(CEMelem,true)
{
  for (unsigned int i = 0; i < _n_nodes; ++i)
    _nodes[i] = elem->get_node(i);
  
  calc_physical_volfrac();
}

XFEMCutElem::~XFEMCutElem()
{
}

Point
XFEMCutElem::get_node_coords(EFAnode* CEMnode, MeshBase* displaced_mesh) const
{
  Point node_coor(0.0,0.0,0.0);
  std::vector<EFAnode*> master_nodes;
  std::vector<Point> master_points;
  std::vector<double> master_weights;

  _efa_elem.getMasterInfo(CEMnode, master_nodes, master_weights);
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
      libMesh::err << " ERROR: master nodes must be local"<<std::endl;
      exit(1);
    }
  } // i
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
    node_coor += master_weights[i]*master_points[i];
  return node_coor;
}

void
XFEMCutElem::calc_physical_volfrac()
{
  Real frag_area = 0.0;
  Real el_area = 0.0;

  //Calculate area of entire element and fragment using the formula:
  // A = 1/2 sum_{i=0}^{n-1} (x_i y_{i+1} - x_{i+1} y{i})

  for (unsigned int i = 0; i < _efa_elem.get_fragment(0)->num_edges(); ++i)
  {
    Point edge_p1 = get_node_coords(_efa_elem.get_frag_edge(0,i)->get_node(0));
    Point edge_p2 = get_node_coords(_efa_elem.get_frag_edge(0,i)->get_node(1));
    frag_area += 0.5*(edge_p1(0)-edge_p2(0))*(edge_p1(1)+edge_p2(1));
  }

  for (unsigned int i = 0; i < _efa_elem.num_edges(); ++i)
  {
    Point edge_p1 = get_node_coords(_efa_elem.get_edge(i)->get_node(0));
    Point edge_p2 = get_node_coords(_efa_elem.get_edge(i)->get_node(1));
    el_area += 0.5*(edge_p1(0)-edge_p2(0))*(edge_p1(1)+edge_p2(1));
  }

  _physical_volfrac = frag_area/el_area;
}

Point
XFEMCutElem::get_origin(unsigned int plane_id, MeshBase* displaced_mesh) const
{
  Point orig(0.0,0.0,0.0);
  std::vector<std::vector<EFAnode*> > cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem.get_fragment(0)->num_edges(); ++i)
  {
    if (_efa_elem.get_fragment(0)->is_edge_interior(i))
    {
      std::vector<EFAnode*> node_line(2,NULL);
      node_line[0] = _efa_elem.get_frag_edge(0,i)->get_node(0);
      node_line[1] = _efa_elem.get_frag_edge(0,i)->get_node(1);
      cut_line_nodes.push_back(node_line);
    }  
  }
  if (cut_line_nodes.size() == 0)
  {
    libMesh::err << " ERROR: no cut line found in this element"<<std::endl;
    exit(1);
  }
  if (plane_id < cut_line_nodes.size()) // valid plane_id
    orig = get_node_coords(cut_line_nodes[plane_id][0], displaced_mesh);
  return orig;
}

Point
XFEMCutElem::get_normal(unsigned int plane_id, MeshBase* displaced_mesh) const
{
  Point normal(0.0,0.0,0.0);
  std::vector<std::vector<EFAnode*> > cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem.get_fragment(0)->num_edges(); ++i)
  {
    if (_efa_elem.get_fragment(0)->is_edge_interior(i))
    {
      std::vector<EFAnode*> node_line(2,NULL);
      node_line[0] = _efa_elem.get_frag_edge(0,i)->get_node(0);
      node_line[1] = _efa_elem.get_frag_edge(0,i)->get_node(1);
      cut_line_nodes.push_back(node_line);
    }  
  }
  if (cut_line_nodes.size() == 0)
  {
    libMesh::err << " ERROR: no cut line found in this element"<<std::endl;
    exit(1);
  }
  if (plane_id < cut_line_nodes.size()) // valid plane_id
  {
    Point cut_line_p1 = get_node_coords(cut_line_nodes[plane_id][0], displaced_mesh);
    Point cut_line_p2 = get_node_coords(cut_line_nodes[plane_id][1], displaced_mesh);
    Point cut_line = cut_line_p2 - cut_line_p1;
    Real len = std::sqrt(cut_line.size_sq());
    cut_line *= (1.0/len);
    normal = Point(cut_line(1), -cut_line(0), 0.0);
  }
  return normal;
}

const EFAelement*
XFEMCutElem::get_efa_elem() const
{
  return &_efa_elem;
}
