//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMCutElem3D.h"

#include "EFANode.h"
#include "EFAFace.h"
#include "EFAFragment3D.h"
#include "EFAFuncs.h"
#include "XFEMFuncs.h"
#include "MooseError.h"

#include "libmesh/mesh.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"

XFEMCutElem3D::XFEMCutElem3D(Elem * elem,
                             const EFAElement3D * const CEMelem,
                             unsigned int n_qpoints,
                             unsigned int n_sides)
  : XFEMCutElem(elem, n_qpoints, n_sides), _efa_elem3d(CEMelem, true)
{
  computePhysicalVolumeFraction();
  computeMomentFittingWeights();
}

XFEMCutElem3D::~XFEMCutElem3D() {}

Point
XFEMCutElem3D::getNodeCoordinates(EFANode * CEMnode, MeshBase * displaced_mesh) const
{
  Point node_coor(0.0, 0.0, 0.0);
  std::vector<EFANode *> master_nodes;
  std::vector<Point> master_points;
  std::vector<double> master_weights;

  _efa_elem3d.getMasterInfo(CEMnode, master_nodes, master_weights);
  for (unsigned int i = 0; i < master_nodes.size(); ++i)
  {
    if (master_nodes[i]->category() == EFANode::N_CATEGORY_LOCAL_INDEX)
    {
      Node * node = _nodes[master_nodes[i]->id()];
      if (displaced_mesh)
        node = displaced_mesh->node_ptr(node->id());
      Point node_p((*node)(0), (*node)(1), (*node)(2));
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
XFEMCutElem3D::computePhysicalVolumeFraction()
{
  Real frag_vol = 0.0;

  // collect fragment info needed by polyhedron_volume_3d()
  std::vector<std::vector<unsigned int>> frag_face_indices;
  std::vector<EFANode *> frag_nodes;
  _efa_elem3d.getFragment(0)->getNodeInfo(frag_face_indices, frag_nodes);
  int face_num = frag_face_indices.size();
  int node_num = frag_nodes.size();

  int order_max = 0;
  int * order = new int[face_num];
  for (int i = 0; i < face_num; ++i)
  {
    if (frag_face_indices[i].size() > (unsigned int)order_max)
      order_max = frag_face_indices[i].size();
    order[i] = frag_face_indices[i].size();
  }

  double * coord = new double[3 * node_num];
  for (unsigned int i = 0; i < frag_nodes.size(); ++i)
  {
    Point p = getNodeCoordinates(frag_nodes[i]);
    coord[3 * i + 0] = p(0);
    coord[3 * i + 1] = p(1);
    coord[3 * i + 2] = p(2);
  }

  int * node = new int[face_num * order_max];
  Xfem::i4vec_zero(face_num * order_max, node);
  for (int i = 0; i < face_num; ++i)
    for (unsigned int j = 0; j < frag_face_indices[i].size(); ++j)
      node[order_max * i + j] = frag_face_indices[i][j];

  // compute fragment volume and volume fraction
  frag_vol = Xfem::polyhedron_volume_3d(coord, order_max, face_num, node, node_num, order);
  _physical_volfrac = frag_vol / _elem_volume;

  delete[] order;
  delete[] coord;
  delete[] node;
}

void
XFEMCutElem3D::computePhysicalFaceAreaFraction(unsigned int side)
{
  Real frag_surf = 0.0;

  std::vector<std::vector<unsigned int>> frag_face_indices;
  std::vector<EFANode *> frag_nodes;
  _efa_elem3d.getFragment(0)->getNodeInfo(frag_face_indices, frag_nodes);
  int face_num = frag_face_indices.size();

  EFAFace * efa_face = _efa_elem3d.getFace(side);
  bool contains_all = true;

  /// find a fragment surface which is covered by element side
  for (int i = 0; i < face_num; ++i)
  {
    contains_all = true;
    for (unsigned int j = 0; j < frag_face_indices[i].size(); ++j)
    {
      EFANode * efa_node = frag_nodes[frag_face_indices[i][j]];
      if (!efa_face->containsNode(efa_node))
      {
        contains_all = false;
        break;
      }
    }

    if (contains_all)
    {
      for (unsigned int j = 0; j < frag_face_indices[i].size(); ++j)
      {
        unsigned int m = ((j + 1) == frag_face_indices[i].size()) ? 0 : j + 1;
        Point edge_p1 = getNodeCoordinates(frag_nodes[frag_face_indices[i][j]]);
        Point edge_p2 = getNodeCoordinates(frag_nodes[frag_face_indices[i][m]]);

        frag_surf += 0.5 * (edge_p1(0) - edge_p2(0)) * (edge_p1(1) + edge_p2(1));
      }
      _physical_areafrac[side] = std::abs(frag_surf) / _elem_side_area[side];
      return;
    }
  }
  _physical_areafrac[side] = 1.0;
}

void
XFEMCutElem3D::computeMomentFittingWeights()
{
  // TODO: 3D moment fitting method nod coded yet - use volume fraction for now
  _new_weights.resize(_n_qpoints, _physical_volfrac);
}

Point
XFEMCutElem3D::getCutPlaneOrigin(unsigned int plane_id, MeshBase * displaced_mesh) const
{
  Point orig(0.0, 0.0, 0.0);
  std::vector<std::vector<EFANode *>> cut_plane_nodes;
  for (unsigned int i = 0; i < _efa_elem3d.getFragment(0)->numFaces(); ++i)
  {
    if (_efa_elem3d.getFragment(0)->isFaceInterior(i))
    {
      EFAFace * face = _efa_elem3d.getFragment(0)->getFace(i);
      std::vector<EFANode *> node_line;
      for (unsigned int j = 0; j < face->numNodes(); ++j)
        node_line.push_back(face->getNode(j));
      cut_plane_nodes.push_back(node_line);
    }
  }
  if (cut_plane_nodes.size() == 0)
    mooseError("no cut plane found in this element");
  if (plane_id < cut_plane_nodes.size()) // valid plane_id
  {
    std::vector<Point> cut_plane_points;
    for (unsigned int i = 0; i < cut_plane_nodes[plane_id].size(); ++i)
      cut_plane_points.push_back(getNodeCoordinates(cut_plane_nodes[plane_id][i], displaced_mesh));

    for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
      orig += cut_plane_points[i];
    orig /= cut_plane_points.size();
  }
  return orig;
}

Point
XFEMCutElem3D::getCutPlaneNormal(unsigned int plane_id, MeshBase * displaced_mesh) const
{
  Point normal(0.0, 0.0, 0.0);
  std::vector<std::vector<EFANode *>> cut_plane_nodes;
  for (unsigned int i = 0; i < _efa_elem3d.getFragment(0)->numFaces(); ++i)
  {
    if (_efa_elem3d.getFragment(0)->isFaceInterior(i))
    {
      EFAFace * face = _efa_elem3d.getFragment(0)->getFace(i);
      std::vector<EFANode *> node_line;
      for (unsigned int j = 0; j < face->numNodes(); ++j)
        node_line.push_back(face->getNode(j));
      cut_plane_nodes.push_back(node_line);
    }
  }
  if (cut_plane_nodes.size() == 0)
    mooseError("no cut plane found in this element");
  if (plane_id < cut_plane_nodes.size()) // valid plane_id
  {
    std::vector<Point> cut_plane_points;
    for (unsigned int i = 0; i < cut_plane_nodes[plane_id].size(); ++i)
      cut_plane_points.push_back(getNodeCoordinates(cut_plane_nodes[plane_id][i], displaced_mesh));

    Point center(0.0, 0.0, 0.0);
    for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
      center += cut_plane_points[i];
    center /= cut_plane_points.size();

    for (unsigned int i = 0; i < cut_plane_points.size(); ++i)
    {
      unsigned int iplus1 = i < cut_plane_points.size() - 1 ? i + 1 : 0;
      Point ray1 = cut_plane_points[i] - center;
      Point ray2 = cut_plane_points[iplus1] - center;
      normal += ray1.cross(ray2);
    }
    normal /= cut_plane_points.size();
  }
  Xfem::normalizePoint(normal);
  return normal;
}

void
XFEMCutElem3D::getCrackTipOriginAndDirection(unsigned /*tip_id*/,
                                             Point & /*origin*/,
                                             Point & /*direction*/) const
{
  // TODO: not implemented for 3D
  mooseError("getCrackTipOriginAndDirection not yet implemented for XFEMCutElem3D");
}

void
XFEMCutElem3D::getFragmentFaces(std::vector<std::vector<Point>> & /*frag_faces*/,
                                MeshBase * /*displaced_mesh*/) const
{
  // TODO: not implemented for 3D
  mooseError("getFragmentFaces not yet implemented for XFEMCutElem3D");
}

const EFAElement *
XFEMCutElem3D::getEFAElement() const
{
  return &_efa_elem3d;
}

unsigned int
XFEMCutElem3D::numCutPlanes() const
{
  unsigned int counter = 0;
  for (unsigned int i = 0; i < _efa_elem3d.getFragment(0)->numFaces(); ++i)
    if (_efa_elem3d.getFragment(0)->isFaceInterior(i))
      counter += 1;
  return counter;
}

void
XFEMCutElem3D::getIntersectionInfo(unsigned int plane_id,
                                   Point & normal,
                                   std::vector<Point> & intersectionPoints,
                                   MeshBase * displaced_mesh) const
{
  intersectionPoints.clear();
  std::vector<std::vector<EFANode *>> cut_plane_nodes;
  for (unsigned int i = 0; i < _efa_elem3d.getFragment(0)->numFaces(); ++i)
  {
    if (_efa_elem3d.getFragment(0)->isFaceInterior(i))
    {
      EFAFace * face = _efa_elem3d.getFragment(0)->getFace(i);
      std::vector<EFANode *> node_line;
      for (unsigned int j = 0; j < face->numNodes(); ++j)
        node_line.push_back(face->getNode(j));
      cut_plane_nodes.push_back(node_line);
    }
  }
  if (cut_plane_nodes.size() == 0)
    mooseError("No cut plane found in this element");

  if (plane_id < cut_plane_nodes.size()) // valid plane_id
  {
    intersectionPoints.resize(cut_plane_nodes[plane_id].size());
    for (unsigned int i = 0; i < cut_plane_nodes[plane_id].size(); ++i)
      intersectionPoints[i] = getNodeCoordinates(cut_plane_nodes[plane_id][i], displaced_mesh);
  }

  normal = getCutPlaneNormal(plane_id, displaced_mesh);
}
