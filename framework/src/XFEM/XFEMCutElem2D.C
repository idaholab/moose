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
#include "XFEMCutElem2D.h"
#include "XFEMMiscFuncs.h"
#include "petscblaslapack.h"

XFEMCutElem2D::XFEMCutElem2D(Elem* elem, const EFAelement2D * const CEMelem, unsigned int n_qpoints):
  XFEMCutElem(elem, n_qpoints),
  _efa_elem2d(CEMelem, true)
{
  calc_physical_volfrac();
//  calc_mf_weights();
}

XFEMCutElem2D::~XFEMCutElem2D()
{
}

Point
XFEMCutElem2D::get_node_coords(EFAnode* CEMnode, MeshBase* displaced_mesh) const
{ Point node_coor(0.0,0.0,0.0);
  std::vector<EFAnode*> master_nodes;
  std::vector<Point> master_points;
  std::vector<double> master_weights;

  _efa_elem2d.getMasterInfo(CEMnode, master_nodes, master_weights);
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
  {
    if (master_nodes[i]->category() == N_CATEGORY_LOCAL_INDEX)
    {
      Node* node = _nodes[master_nodes[i]->id()];
      if (displaced_mesh)
        node = displaced_mesh->node_ptr(node->id());
      Point node_p((*node)(0), (*node)(1), 0.0);
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
XFEMCutElem2D::calc_physical_volfrac()
{
  Real frag_area = 0.0;
  Real el_area = 0.0;

  //Calculate area of entire element and fragment using the formula:
  // A = 1/2 sum_{i=0}^{n-1} (x_i y_{i+1} - x_{i+1} y{i})

  for (unsigned int i = 0; i < _efa_elem2d.get_fragment(0)->num_edges(); ++i)
  {
    Point edge_p1 = get_node_coords(_efa_elem2d.get_frag_edge(0,i)->get_node(0));
    Point edge_p2 = get_node_coords(_efa_elem2d.get_frag_edge(0,i)->get_node(1));
    frag_area += 0.5*(edge_p1(0)-edge_p2(0))*(edge_p1(1)+edge_p2(1));
  }
  _physical_volfrac = frag_area/_elem_volume;
}

void
XFEMCutElem2D::calc_mf_weights()
{
  // Purpose: calculate new weights via moment-fitting method
  std::vector<Point> elem_nodes(_n_nodes, Point(0.0,0.0,0.0));
  std::vector<std::vector<Real> > wsg;

  for (unsigned int i = 0; i < _n_nodes; ++i)
    elem_nodes[i] = (*_nodes[i]);

  if (_efa_elem2d.is_partial() && _n_qpoints <= 6) // ONLY work for <= 6 q_points
  {
    new_weight_mf(_n_nodes, _n_qpoints, elem_nodes, wsg);
    _new_weights.resize(wsg.size(), 1.0);
    for (unsigned int i = 0; i < wsg.size(); ++i)
      _new_weights[i] = wsg[i][2]; // weight multiplier
  }
  else
    _new_weights.resize(_n_qpoints, _physical_volfrac);
}

Point
XFEMCutElem2D::get_origin(unsigned int plane_id, MeshBase* displaced_mesh) const
{
  Point orig(0.0,0.0,0.0);
  std::vector<std::vector<EFAnode*> > cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem2d.get_fragment(0)->num_edges(); ++i)
  {
    if (_efa_elem2d.get_fragment(0)->is_edge_interior(i))
    {
      std::vector<EFAnode*> node_line(2,NULL);
      node_line[0] = _efa_elem2d.get_frag_edge(0,i)->get_node(0);
      node_line[1] = _efa_elem2d.get_frag_edge(0,i)->get_node(1);
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
XFEMCutElem2D::get_normal(unsigned int plane_id, MeshBase* displaced_mesh) const
{
  Point normal(0.0,0.0,0.0);
  std::vector<std::vector<EFAnode*> > cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem2d.get_fragment(0)->num_edges(); ++i)
  {
    if (_efa_elem2d.get_fragment(0)->is_edge_interior(i))
    {
      std::vector<EFAnode*> node_line(2,NULL);
      node_line[0] = _efa_elem2d.get_frag_edge(0,i)->get_node(0);
      node_line[1] = _efa_elem2d.get_frag_edge(0,i)->get_node(1);
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

void
XFEMCutElem2D::get_frag_faces(std::vector<std::vector<Point> > &frag_faces, MeshBase* displaced_mesh) const
{
  frag_faces.clear();
  for (unsigned int i = 0; i < _efa_elem2d.get_fragment(0)->num_edges(); ++i)
  {
    std::vector<Point> edge_points(2, Point(0.0,0.0,0.0));
    edge_points[0] = get_node_coords(_efa_elem2d.get_frag_edge(0,i)->get_node(0), displaced_mesh);
    edge_points[1] = get_node_coords(_efa_elem2d.get_frag_edge(0,i)->get_node(1), displaced_mesh);
    frag_faces.push_back(edge_points);
  }
}

const EFAelement*
XFEMCutElem2D::get_efa_elem() const
{
  return &_efa_elem2d;
}

unsigned int
XFEMCutElem2D::num_cut_planes() const
{
  unsigned int counter = 0;
  for (unsigned int i = 0; i < _efa_elem2d.get_fragment(0)->num_edges(); ++i)
    if (_efa_elem2d.get_fragment(0)->is_edge_interior(i))
      counter += 1;
  return counter;
}

// ****** moment-fitting private methods ******
void
XFEMCutElem2D::new_weight_mf(unsigned int nen, unsigned int nqp, std::vector<Point> &elem_nodes,
                             std::vector<std::vector<Real> > &wsg) // ZZY
{
  std::vector<std::vector<Real> > tsg;
  partial_gauss(nen, tsg); // get tsg - QPs within partial element
  solve_mf(nen, nqp, elem_nodes, tsg, wsg); // get wsg - QPs from moment-fitting
}

void
XFEMCutElem2D::partial_gauss(unsigned int nen, std::vector<std::vector<Real> > &tsg) // ZZY
{
  // Get the coords for parial element nodes
  EFAfragment2D* frag = _efa_elem2d.get_fragment(0);
  unsigned int nnd_pe = frag->num_edges();
  std::vector<Point> frag_points(nnd_pe, Point(0.0,0.0,0.0));// nodal coord of partial elem
  Real jac = 0.0;

  for (unsigned int j = 0; j < nnd_pe; ++j)
    frag_points[j] = get_node_coords(frag->get_edge(j)->get_node(0));

  // Get centroid coords for partial elements
  Point xcrd(0.0,0.0,0.0);
  for (unsigned int j = 0; j < nnd_pe; ++j)
    xcrd += frag_points[j];
  xcrd *= (1.0/nnd_pe);

  // Get tsg - the physical coords of Gaussian Q-points for partial elements
  if ((nnd_pe == 3)||(nnd_pe == 4)) // partial element is a triangle or quad
  {
    std::vector<std::vector<Real> > sg2;
    std::vector<std::vector<Real> > shape(nnd_pe,std::vector<Real>(3,0.0));
    stdQuadr2D(nnd_pe, 2, sg2); // 
    for (unsigned int l = 0; l < sg2.size(); ++l)
    {
      shapeFunc2D(nnd_pe, sg2[l], frag_points, shape, jac, true); // Get shape
      std::vector<Real> tsg_line(3,0.0);
      for (unsigned int k = 0; k < nnd_pe; ++k)
      {
        tsg_line[0] += shape[k][2]*frag_points[k](0);
        tsg_line[1] += shape[k][2]*frag_points[k](1);
      }
      if (nnd_pe == 3) // trig partial elem
        tsg_line[2] = sg2[l][3]*jac; // total weights
      else // quad partial elem
        tsg_line[2] = sg2[l][2]*jac; // total weights
      tsg.push_back(tsg_line);
    }
  }
  else if (nnd_pe >= 5) // parial element is a polygon
  {
    for (unsigned int j = 0; j < nnd_pe; ++j) // loop all sub-trigs
    {
      std::vector<std::vector<Real> > shape(3, std::vector<Real>(3,0.0));
      std::vector<Point> subtrig_points(3, Point(0.0,0.0,0.0)); // sub-trig nodal coords

      int jplus1(j < nnd_pe-1 ? j+1 : 0);
      subtrig_points[0] = xcrd;
      subtrig_points[1] = frag_points[j];
      subtrig_points[2] = frag_points[jplus1];

      std::vector<std::vector<Real> > sg2;
      stdQuadr2D(3, 2, sg2); // get sg2
      for (unsigned int l = 0; l < sg2.size(); ++l) // loop all int pts on a sub-trig
      {
        shapeFunc2D(3, sg2[l], subtrig_points, shape, jac, true); // Get shape
        std::vector<Real> tsg_line(3,0.0);
        for (unsigned int k = 0; k < 3; ++k) // loop sub-trig nodes
        {
          tsg_line[0] += shape[k][2]*subtrig_points[k](0);
          tsg_line[1] += shape[k][2]*subtrig_points[k](1);
        }
        tsg_line[2] = sg2[l][3]*jac;
        tsg.push_back(tsg_line);
      }
    }
  }
  else
    mooseError("Invalid partial element!");
}

void
XFEMCutElem2D::solve_mf(unsigned int nen, unsigned int nqp, std::vector<Point> &elem_nodes, std::vector<std::vector<Real> > &tsg, std::vector<std::vector<Real> > &wsg)
{
  // Get physical coords for the new six-point rule
  std::vector<std::vector<Real> > shape(nen,std::vector<Real>(3,0.0));
  std::vector<std::vector<Real> > wss;

  if(nen == 4){
    wss.resize(_g_points.size());
    for(unsigned int i = 0; i < _g_points.size(); i ++){
      wss[i].resize(3);
      wss[i][0] = _g_points[i](0);
      wss[i][1] = _g_points[i](1);
      wss[i][2] = _g_weights[i];
    }
  }else if(nen == 3){
    wss.resize(_g_points.size());
    for(unsigned int i = 0; i < _g_points.size(); i ++){
      wss[i].resize(4);
      wss[i][0] = _g_points[i](0);
      wss[i][1] = _g_points[i](1);
      wss[i][2] = _g_points[i](2);
      wss[i][3] = _g_weights[i];
    }
  }else
    mooseError("Invalid element");

  wsg.resize(wss.size());
  for (unsigned int i = 0; i < wsg.size(); ++i) wsg[i].resize(3,0.0);
  Real jac = 0.0;
  std::vector<Real> old_weights(wss.size(),0.0);

  for (unsigned int l = 0; l < wsg.size(); ++l)
  {
    shapeFunc2D(nen, wss[l], elem_nodes, shape, jac, true); // Get shape
    if (nen == 4) // 2D quad elem
      old_weights[l] = wss[l][2]*jac; // weights for total element
    else if (nen == 3) // 2D triangle elem
      old_weights[l] = wss[l][3]*jac;
    else
      mooseError("Invalid element!");    
    for (unsigned int k = 0; k < nen; ++k) // physical coords of Q-pts
    {
      wsg[l][0] += shape[k][2]*elem_nodes[k](0);
      wsg[l][1] += shape[k][2]*elem_nodes[k](1);
    }
  }

  // Compute weights via moment fitting
  Real *A;
  A = new Real[wsg.size()*wsg.size()];
  unsigned ind = 0;
  for (unsigned int i = 0; i < wsg.size(); ++i){
     A[ind] = 1.0; // const
    if (nqp > 1) A[1+ind] = wsg[i][0]; // x
    if (nqp > 2) A[2+ind] = wsg[i][1]; // y
    if (nqp > 3) A[3+ind] = wsg[i][0]*wsg[i][1]; // x*y
    if (nqp > 4) A[4+ind] = wsg[i][0]*wsg[i][0]; // x^2
    if (nqp > 5) A[5+ind] = wsg[i][1]*wsg[i][1]; // y^2
    if (nqp > 6) mooseError("Q-points of more than 6 are not allowed now!");
    ind = ind+nqp;
  }

  Real *b;
  b = new Real[wsg.size()];
  for (unsigned int i = 0; i < wsg.size(); ++i)
    b[i] = 0.0;
  for (unsigned int i = 0; i < tsg.size(); ++i)
  {
    b[0] += tsg[i][2];
    if (nqp > 1) b[1] += tsg[i][2]*tsg[i][0];
    if (nqp > 2) b[2] += tsg[i][2]*tsg[i][1];
    if (nqp > 3) b[3] += tsg[i][2]*tsg[i][0]*tsg[i][1];
    if (nqp > 4) b[4] += tsg[i][2]*tsg[i][0]*tsg[i][0];
    if (nqp > 5) b[5] += tsg[i][2]*tsg[i][1]*tsg[i][1];
    if (nqp > 6) mooseError("Q-points of more than 6 are not allowed now!");
  }

  int nrhs = 1;
  int info;
  int n = wsg.size();
  std::vector<int> ipiv(n);
 
  LAPACKgesv_(&n, &nrhs, A, &n, &ipiv[0], b, &n, &info );

  for (unsigned int i = 0; i < wsg.size(); ++i)
    wsg[i][2] = b[i]/old_weights[i]; // get the multiplier

  // delete arrays
  delete[] A;
  delete[] b;
}
