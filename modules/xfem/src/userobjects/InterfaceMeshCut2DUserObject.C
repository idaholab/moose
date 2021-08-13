//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceMeshCut2DUserObject.h"
#include "XFEMMovingInterfaceVelocityBase.h"

registerMooseObject("XFEMApp", InterfaceMeshCut2DUserObject);

InputParameters
InterfaceMeshCut2DUserObject::validParams()
{
  InputParameters params = InterfaceMeshCutUserObjectBase::validParams();
  params.addClassDescription("A userobject to cut a 2D mesh using a 1D cutter mesh.");
  return params;
}

InterfaceMeshCut2DUserObject::InterfaceMeshCut2DUserObject(const InputParameters & parameters)
  : InterfaceMeshCutUserObjectBase(parameters)
{
  for (const auto & elem : _cutter_mesh->element_ptr_range())
    if (elem->type() != EDGE2)
      mooseError(
          "InterfaceMeshCut2DUserObject currently only supports EDGE2 element in the cut mesh.");
}

void
InterfaceMeshCut2DUserObject::calculateNormals()
{
  _element_normals.clear();

  for (const auto & elem : _cutter_mesh->element_ptr_range())
  {
    Point a = elem->node_ref(1);
    Point b = elem->node_ref(0);

    Point normal_ab = Point(-(b - a)(1), (b - a)(0), 0);
    normal_ab /= normal_ab.norm();

    _element_normals.insert(std::make_pair(elem->id(), normal_ab));
  }
}

Point
InterfaceMeshCut2DUserObject::nodeNormal(const unsigned int & node_id)
{
  Point normal(0.0);

  for (const auto & node_neigh_elem_id : _node_to_elem_map[node_id])
  {
    const auto & elem = _cutter_mesh->elem_ref(node_neigh_elem_id);

    Point a = elem.node_ref(1);
    Point b = elem.node_ref(0);

    Point normal_ab = Point(-(b - a)(1), (b - a)(0), 0);
    normal_ab /= normal_ab.norm();

    normal += normal_ab;
  }

  unsigned int num = _node_to_elem_map[node_id].size();

  if (num == 0)
    mooseError("InterfaceMeshCut2DUserObject, the node is not found in node_to_elem_map in "
               "calculting its normal.");

  return normal / num;
}

bool
InterfaceMeshCut2DUserObject::cutElementByGeometry(const Elem * elem,
                                                   std::vector<Xfem::CutEdge> & cut_edges,
                                                   std::vector<Xfem::CutNode> & cut_nodes) const
{
  mooseAssert(elem->dim() == 2, "Dimension of element to be cut must be 2");

  bool elem_cut = false;

  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    unsigned int n_sides = elem->n_sides();

    for (unsigned int i = 0; i < n_sides; ++i)
    {
      std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);

      mooseAssert(curr_side->type() == EDGE2, "Element side type must be EDGE2.");

      const Node * node1 = curr_side->node_ptr(0);
      const Node * node2 = curr_side->node_ptr(1);
      Real seg_int_frac = 0.0;

      const std::pair<Point, Point> elem_endpoints(cut_elem->node_ref(0), cut_elem->node_ref(1));

      if (Xfem::intersectSegmentWithCutLine(*node1, *node2, elem_endpoints, 1.0, seg_int_frac))
      {
        if (seg_int_frac > Xfem::tol && seg_int_frac < 1.0 - Xfem::tol)
        {
          elem_cut = true;
          Xfem::CutEdge mycut;
          mycut._id1 = node1->id();
          mycut._id2 = node2->id();
          mycut._distance = seg_int_frac;
          mycut._host_side_id = i;
          cut_edges.push_back(mycut);
        }
        else if (seg_int_frac < Xfem::tol)
        {
          elem_cut = true;
          Xfem::CutNode mycut;
          mycut._id = node1->id();
          mycut._host_id = i;
          cut_nodes.push_back(mycut);
        }
      }
    }
  }
  return elem_cut;
}

bool
InterfaceMeshCut2DUserObject::cutElementByGeometry(const Elem * /* elem*/,
                                                   std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("invalid method for InterfaceMeshCut2DUserObject");
  return false;
}

bool
InterfaceMeshCut2DUserObject::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_edges*/,
    std::vector<Xfem::CutEdge> & /*cut_edges*/) const
{
  mooseError("cutFragmentByGeometry not yet implemented for InterfaceMeshCut2DUserObject");
  return false;
}

bool
InterfaceMeshCut2DUserObject::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_faces*/,
    std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("invalid method for InterfaceMeshCut2DUserObject");
  return false;
}

Real
InterfaceMeshCut2DUserObject::calculateSignedDistance(Point p) const
{
  Real min_dist = std::numeric_limits<Real>::max();

  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    Point a = cut_elem->node_ref(0);
    Point b = cut_elem->node_ref(1);

    Point c = p - a;
    Point v = (b - a) / (b - a).norm();
    Real d = (b - a).norm();
    Real t = v * c;

    Real dist;
    Point nearest_point;

    if (t < 0)
    {
      dist = (p - a).norm();
      nearest_point = a;
    }
    else if (t > d)
    {
      dist = (p - b).norm();
      nearest_point = b;
    }
    else
    {
      v *= t;
      dist = (p - a - v).norm();
      nearest_point = (a + v);
    }

    Point p_nearest_point = nearest_point - p;

    Point normal_ab = Point(-(b - a)(1), (b - a)(0), 0);

    if (normal_ab * p_nearest_point < 0)
      dist = -dist;

    if (std::abs(dist) < std::abs(min_dist))
      min_dist = dist;
  }

  return min_dist;
}
