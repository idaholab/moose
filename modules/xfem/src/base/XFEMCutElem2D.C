//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMCutElem2D.h"

#include "EFANode.h"
#include "EFAEdge.h"
#include "EFAFragment2D.h"
#include "XFEMFuncs.h"
#include "MooseError.h"

#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/petsc_macro.h"
#include "petscblaslapack.h"

XFEMCutElem2D::XFEMCutElem2D(Elem * elem,
                             const EFAElement2D * const CEMelem,
                             unsigned int n_qpoints,
                             unsigned int n_sides)
  : XFEMCutElem(elem, n_qpoints, n_sides), _efa_elem2d(CEMelem, true)
{
  computePhysicalVolumeFraction();
}

XFEMCutElem2D::~XFEMCutElem2D() {}

Point
XFEMCutElem2D::getNodeCoordinates(EFANode * CEMnode, MeshBase * displaced_mesh) const
{
  Point node_coor(0.0, 0.0, 0.0);
  std::vector<EFANode *> master_nodes;
  std::vector<Point> master_points;
  std::vector<double> master_weights;

  _efa_elem2d.getMasterInfo(CEMnode, master_nodes, master_weights);
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
  {
    if (master_nodes[i]->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
    {
      Node * node = _nodes[master_nodes[i]->id()];
      if (displaced_mesh)
        node = displaced_mesh->node_ptr(node->id());
      Point node_p((*node)(0), (*node)(1), 0.0);
      master_points.push_back(node_p);
    }
    else
      mooseError("master nodes must be local");
  }
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
    node_coor += master_weights[i] * master_points[i];
  return node_coor;
}

void
XFEMCutElem2D::computePhysicalVolumeFraction()
{
  Real frag_vol = 0.0;

  // Calculate area of entire element and fragment using the formula:
  // A = 1/2 sum_{i=0}^{n-1} (x_i y_{i+1} - x_{i+1} y{i})

  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
  {
    Point edge_p1 = getNodeCoordinates(_efa_elem2d.getFragmentEdge(0, i)->getNode(0));
    Point edge_p2 = getNodeCoordinates(_efa_elem2d.getFragmentEdge(0, i)->getNode(1));
    frag_vol += 0.5 * (edge_p1(0) - edge_p2(0)) * (edge_p1(1) + edge_p2(1));
  }
  _physical_volfrac = frag_vol / _elem_volume;
}

void
XFEMCutElem2D::computePhysicalFaceAreaFraction(unsigned int side)
{
  Real frag_surf = 0.0;

  EFAEdge * edge = _efa_elem2d.getEdge(side);

  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
  {
    EFANode * node_1 = _efa_elem2d.getFragmentEdge(0, i)->getNode(0);
    EFANode * node_2 = _efa_elem2d.getFragmentEdge(0, i)->getNode(1);

    /// find a fragment edge which is covered by element side
    if (edge->containsNode(node_1) && edge->containsNode(node_2))
    {
      Point edge_p1 = getNodeCoordinates(node_1);
      Point edge_p2 = getNodeCoordinates(node_2);
      frag_surf = (edge_p1 - edge_p2).norm();
      _physical_areafrac[side] = frag_surf / _elem_side_area[side];
      return;
    }
  }
  _physical_areafrac[side] = 1.0;
}

void
XFEMCutElem2D::computeMomentFittingWeights()
{
  // Purpose: calculate new weights via moment-fitting method
  std::vector<Point> elem_nodes(_n_nodes, Point(0.0, 0.0, 0.0));
  std::vector<std::vector<Real>> wsg;

  for (unsigned int i = 0; i < _n_nodes; ++i)
    elem_nodes[i] = (*_nodes[i]);

  if (_efa_elem2d.isPartial() && _n_qpoints <= 6) // ONLY work for <= 6 q_points
  {
    std::vector<std::vector<Real>> tsg;
    getPhysicalQuadraturePoints(tsg); // get tsg - QPs within partial element
    solveMomentFitting(
        _n_nodes, _n_qpoints, elem_nodes, tsg, wsg); // get wsg - QPs from moment-fitting
    _new_weights.resize(wsg.size(), 1.0);
    for (unsigned int i = 0; i < wsg.size(); ++i)
      _new_weights[i] = wsg[i][2]; // weight multiplier
  }
  else
    _new_weights.resize(_n_qpoints, _physical_volfrac);
}

Point
XFEMCutElem2D::getCutPlaneOrigin(unsigned int plane_id, MeshBase * displaced_mesh) const
{
  Point orig(0.0, 0.0, 0.0);
  std::vector<std::vector<EFANode *>> cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
  {
    if (_efa_elem2d.getFragment(0)->isEdgeInterior(i))
    {
      std::vector<EFANode *> node_line(2, NULL);
      node_line[0] = _efa_elem2d.getFragmentEdge(0, i)->getNode(0);
      node_line[1] = _efa_elem2d.getFragmentEdge(0, i)->getNode(1);
      cut_line_nodes.push_back(node_line);
    }
  }
  if (cut_line_nodes.size() == 0)
    mooseError("no cut line found in this element");
  if (plane_id < cut_line_nodes.size()) // valid plane_id
    orig = getNodeCoordinates(cut_line_nodes[plane_id][0], displaced_mesh);
  return orig;
}

Point
XFEMCutElem2D::getCutPlaneNormal(unsigned int plane_id, MeshBase * displaced_mesh) const
{
  Point normal(0.0, 0.0, 0.0);
  std::vector<std::vector<EFANode *>> cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
  {
    if (_efa_elem2d.getFragment(0)->isEdgeInterior(i))
    {
      std::vector<EFANode *> node_line(2, NULL);
      node_line[0] = _efa_elem2d.getFragmentEdge(0, i)->getNode(0);
      node_line[1] = _efa_elem2d.getFragmentEdge(0, i)->getNode(1);
      cut_line_nodes.push_back(node_line);
    }
  }
  if (cut_line_nodes.size() == 0)
    mooseError("no cut line found in this element");
  if (plane_id < cut_line_nodes.size()) // valid plane_id
  {
    Point cut_line_p1 = getNodeCoordinates(cut_line_nodes[plane_id][0], displaced_mesh);
    Point cut_line_p2 = getNodeCoordinates(cut_line_nodes[plane_id][1], displaced_mesh);
    Point cut_line = cut_line_p2 - cut_line_p1;
    Real len = std::sqrt(cut_line.norm_sq());
    cut_line /= len;
    normal = Point(cut_line(1), -cut_line(0), 0.0);
  }
  return normal;
}

void
XFEMCutElem2D::getCrackTipOriginAndDirection(unsigned tip_id,
                                             Point & origin,
                                             Point & direction) const
{
  // TODO: two cut plane case is not working
  std::vector<EFANode *> cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
  {
    if (_efa_elem2d.getFragment(0)->isEdgeInterior(i))
    {
      std::vector<EFANode *> node_line(2, NULL);
      node_line[0] = _efa_elem2d.getFragmentEdge(0, i)->getNode(0);
      node_line[1] = _efa_elem2d.getFragmentEdge(0, i)->getNode(1);
      if (node_line[1]->id() == tip_id)
      {
        cut_line_nodes.push_back(node_line[0]);
        cut_line_nodes.push_back(node_line[1]);
      }
      else if (node_line[0]->id() == tip_id)
      {
        node_line[1] = node_line[0];
        node_line[0] = _efa_elem2d.getFragmentEdge(0, i)->getNode(1);
        cut_line_nodes.push_back(node_line[0]);
        cut_line_nodes.push_back(node_line[1]);
      }
    }
  }
  if (cut_line_nodes.size() == 0)
    mooseError("no cut line found in this element");

  Point cut_line_p1 = getNodeCoordinates(cut_line_nodes[0]);
  Point cut_line_p2 = getNodeCoordinates(cut_line_nodes[1]);
  Point cut_line = cut_line_p2 - cut_line_p1;
  Real len = std::sqrt(cut_line.norm_sq());
  cut_line /= len;
  origin = cut_line_p2;
  direction = Point(cut_line(0), cut_line(1), 0.0);
}

void
XFEMCutElem2D::getFragmentFaces(std::vector<std::vector<Point>> & frag_faces,
                                MeshBase * displaced_mesh) const
{
  frag_faces.clear();
  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
  {
    std::vector<Point> edge_points(2, Point(0.0, 0.0, 0.0));
    edge_points[0] =
        getNodeCoordinates(_efa_elem2d.getFragmentEdge(0, i)->getNode(0), displaced_mesh);
    edge_points[1] =
        getNodeCoordinates(_efa_elem2d.getFragmentEdge(0, i)->getNode(1), displaced_mesh);
    frag_faces.push_back(edge_points);
  }
}

const EFAElement *
XFEMCutElem2D::getEFAElement() const
{
  return &_efa_elem2d;
}

unsigned int
XFEMCutElem2D::numCutPlanes() const
{
  unsigned int counter = 0;
  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
    if (_efa_elem2d.getFragment(0)->isEdgeInterior(i))
      counter += 1;
  return counter;
}

void
XFEMCutElem2D::getPhysicalQuadraturePoints(std::vector<std::vector<Real>> & tsg)
{
  // Get the coords for parial element nodes
  EFAFragment2D * frag = _efa_elem2d.getFragment(0);
  unsigned int nnd_pe = frag->numEdges();
  std::vector<Point> frag_points(nnd_pe, Point(0.0, 0.0, 0.0)); // nodal coord of partial elem
  Real jac = 0.0;

  for (unsigned int j = 0; j < nnd_pe; ++j)
    frag_points[j] = getNodeCoordinates(frag->getEdge(j)->getNode(0));

  // Get centroid coords for partial elements
  Point xcrd(0.0, 0.0, 0.0);
  for (unsigned int j = 0; j < nnd_pe; ++j)
    xcrd += frag_points[j];
  xcrd *= (1.0 / nnd_pe);

  // Get tsg - the physical coords of Gaussian Q-points for partial elements
  if ((nnd_pe == 3) || (nnd_pe == 4)) // partial element is a triangle or quad
  {
    std::vector<std::vector<Real>> sg2;
    std::vector<std::vector<Real>> shape(nnd_pe, std::vector<Real>(3, 0.0));
    Xfem::stdQuadr2D(nnd_pe, 2, sg2);
    for (unsigned int l = 0; l < sg2.size(); ++l)
    {
      Xfem::shapeFunc2D(nnd_pe, sg2[l], frag_points, shape, jac, true); // Get shape
      std::vector<Real> tsg_line(3, 0.0);
      for (unsigned int k = 0; k < nnd_pe; ++k)
      {
        tsg_line[0] += shape[k][2] * frag_points[k](0);
        tsg_line[1] += shape[k][2] * frag_points[k](1);
      }
      if (nnd_pe == 3)                 // tri partial elem
        tsg_line[2] = sg2[l][3] * jac; // total weights
      else                             // quad partial elem
        tsg_line[2] = sg2[l][2] * jac; // total weights
      tsg.push_back(tsg_line);
    }
  }
  else if (nnd_pe >= 5) // partial element is a polygon
  {
    for (unsigned int j = 0; j < nnd_pe; ++j) // loop all sub-tris
    {
      std::vector<std::vector<Real>> shape(3, std::vector<Real>(3, 0.0));
      std::vector<Point> subtri_points(3, Point(0.0, 0.0, 0.0)); // sub-tri nodal coords

      int jplus1 = j < nnd_pe - 1 ? j + 1 : 0;
      subtri_points[0] = xcrd;
      subtri_points[1] = frag_points[j];
      subtri_points[2] = frag_points[jplus1];

      std::vector<std::vector<Real>> sg2;
      Xfem::stdQuadr2D(3, 2, sg2);                  // get sg2
      for (unsigned int l = 0; l < sg2.size(); ++l) // loop all int pts on a sub-tri
      {
        Xfem::shapeFunc2D(3, sg2[l], subtri_points, shape, jac, true); // Get shape
        std::vector<Real> tsg_line(3, 0.0);
        for (unsigned int k = 0; k < 3; ++k) // loop sub-tri nodes
        {
          tsg_line[0] += shape[k][2] * subtri_points[k](0);
          tsg_line[1] += shape[k][2] * subtri_points[k](1);
        }
        tsg_line[2] = sg2[l][3] * jac;
        tsg.push_back(tsg_line);
      }
    }
  }
  else
    mooseError("Invalid partial element!");
}

void
XFEMCutElem2D::solveMomentFitting(unsigned int nen,
                                  unsigned int nqp,
                                  std::vector<Point> & elem_nodes,
                                  std::vector<std::vector<Real>> & tsg,
                                  std::vector<std::vector<Real>> & wsg)
{
  // Get physical coords for the new six-point rule
  std::vector<std::vector<Real>> shape(nen, std::vector<Real>(3, 0.0));
  std::vector<std::vector<Real>> wss;

  if (nen == 4)
  {
    wss.resize(_qp_points.size());
    for (unsigned int i = 0; i < _qp_points.size(); i++)
    {
      wss[i].resize(3);
      wss[i][0] = _qp_points[i](0);
      wss[i][1] = _qp_points[i](1);
      wss[i][2] = _qp_weights[i];
    }
  }
  else if (nen == 3)
  {
    wss.resize(_qp_points.size());
    for (unsigned int i = 0; i < _qp_points.size(); i++)
    {
      wss[i].resize(4);
      wss[i][0] = _qp_points[i](0);
      wss[i][1] = _qp_points[i](1);
      wss[i][2] = 1.0 - _qp_points[i](0) - _qp_points[i](1);
      wss[i][3] = _qp_weights[i];
    }
  }
  else
    mooseError("Invalid element");

  wsg.resize(wss.size());
  for (unsigned int i = 0; i < wsg.size(); ++i)
    wsg[i].resize(3, 0.0);
  Real jac = 0.0;
  std::vector<Real> old_weights(wss.size(), 0.0);

  for (unsigned int l = 0; l < wsg.size(); ++l)
  {
    Xfem::shapeFunc2D(nen, wss[l], elem_nodes, shape, jac, true); // Get shape
    if (nen == 4)                                                 // 2D quad elem
      old_weights[l] = wss[l][2] * jac;                           // weights for total element
    else if (nen == 3)                                            // 2D triangle elem
      old_weights[l] = wss[l][3] * jac;
    else
      mooseError("Invalid element!");
    for (unsigned int k = 0; k < nen; ++k) // physical coords of Q-pts
    {
      wsg[l][0] += shape[k][2] * elem_nodes[k](0);
      wsg[l][1] += shape[k][2] * elem_nodes[k](1);
    }
  }

  // Compute weights via moment fitting
  Real * A;
  A = new Real[wsg.size() * wsg.size()];
  unsigned ind = 0;
  for (unsigned int i = 0; i < wsg.size(); ++i)
  {
    A[ind] = 1.0; // const
    if (nqp > 1)
      A[1 + ind] = wsg[i][0]; // x
    if (nqp > 2)
      A[2 + ind] = wsg[i][1]; // y
    if (nqp > 3)
      A[3 + ind] = wsg[i][0] * wsg[i][1]; // x*y
    if (nqp > 4)
      A[4 + ind] = wsg[i][0] * wsg[i][0]; // x^2
    if (nqp > 5)
      A[5 + ind] = wsg[i][1] * wsg[i][1]; // y^2
    if (nqp > 6)
      mooseError("Q-points of more than 6 are not allowed now!");
    ind = ind + nqp;
  }

  Real * b;
  b = new Real[wsg.size()];
  for (unsigned int i = 0; i < wsg.size(); ++i)
    b[i] = 0.0;
  for (unsigned int i = 0; i < tsg.size(); ++i)
  {
    b[0] += tsg[i][2];
    if (nqp > 1)
      b[1] += tsg[i][2] * tsg[i][0];
    if (nqp > 2)
      b[2] += tsg[i][2] * tsg[i][1];
    if (nqp > 3)
      b[3] += tsg[i][2] * tsg[i][0] * tsg[i][1];
    if (nqp > 4)
      b[4] += tsg[i][2] * tsg[i][0] * tsg[i][0];
    if (nqp > 5)
      b[5] += tsg[i][2] * tsg[i][1] * tsg[i][1];
    if (nqp > 6)
      mooseError("Q-points of more than 6 are not allowed now!");
  }

  PetscBLASInt nrhs = 1;
  PetscBLASInt info;
  PetscBLASInt n = wsg.size();
  std::vector<PetscBLASInt> ipiv(n);

  LAPACKgesv_(&n, &nrhs, A, &n, &ipiv[0], b, &n, &info);

  for (unsigned int i = 0; i < wsg.size(); ++i)
    wsg[i][2] = b[i] / old_weights[i]; // get the multiplier

  // delete arrays
  delete[] A;
  delete[] b;
}

void
XFEMCutElem2D::getIntersectionInfo(unsigned int plane_id,
                                   Point & normal,
                                   std::vector<Point> & intersectionPoints,
                                   MeshBase * displaced_mesh) const
{
  intersectionPoints.resize(2); // 2D: the intersection line is straight and can be represented by
                                // two ending points.(may have issues with double cuts case!)

  std::vector<std::vector<EFANode *>> cut_line_nodes;
  for (unsigned int i = 0; i < _efa_elem2d.getFragment(0)->numEdges(); ++i)
  {
    if (_efa_elem2d.getFragment(0)->isEdgeInterior(i))
    {
      std::vector<EFANode *> node_line(2, NULL);
      node_line[0] = _efa_elem2d.getFragmentEdge(0, i)->getNode(0);
      node_line[1] = _efa_elem2d.getFragmentEdge(0, i)->getNode(1);
      cut_line_nodes.push_back(node_line);
    }
  }
  if (cut_line_nodes.size() == 0)
    mooseError("No cut line found in this element");

  if (plane_id < cut_line_nodes.size()) // valid plane_id
  {
    intersectionPoints[0] = getNodeCoordinates(cut_line_nodes[plane_id][0], displaced_mesh);
    intersectionPoints[1] = getNodeCoordinates(cut_line_nodes[plane_id][1], displaced_mesh);
  }

  normal = getCutPlaneNormal(plane_id, displaced_mesh);
}
