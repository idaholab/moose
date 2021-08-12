//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceMeshCut3DUserObject.h"
#include "XFEMMovingInterfaceVelocityBase.h"
#include "libmesh/enum_point_locator_type.h"

registerMooseObject("XFEMApp", InterfaceMeshCut3DUserObject);

InputParameters
InterfaceMeshCut3DUserObject::validParams()
{
  InputParameters params = InterfaceMeshCutUserObjectBase::validParams();
  params.addClassDescription("A userobject to cut a 3D mesh using a 2D cutter mesh.");
  return params;
}

InterfaceMeshCut3DUserObject::InterfaceMeshCut3DUserObject(const InputParameters & parameters)
  : InterfaceMeshCutUserObjectBase(parameters)
{
  for (const auto & elem : _cutter_mesh->element_ptr_range())
    if (elem->type() != TRI3)
      mooseError("InterfaceMeshCut3DUserObject currently only supports TRI3 elements in the "
                 "cutting mesh.");
}

void
InterfaceMeshCut3DUserObject::calculateNormals()
{
  _pseudo_normal.clear();

  for (const auto & elem : _cutter_mesh->element_ptr_range())
  {
    std::vector<Point> vertices{elem->node_ref(0), elem->node_ref(1), elem->node_ref(2)};
    std::array<Point, 7> normal;
    Plane elem_plane(vertices[0], vertices[1], vertices[2]);
    normal[0] = 2.0 * libMesh::pi * elem_plane.unit_normal(vertices[0]);

    for (unsigned int i = 0; i < elem->n_nodes(); i++)
    {
      Point normal_at_node(0.0);
      const Node & node = elem->node_ref(i);

      Real angle_sum = 0.0;

      for (const auto & node_neigh_elem_id : _node_to_elem_map[node.id()])
      {
        const Elem & node_neigh_elem = _cutter_mesh->elem_ref(node_neigh_elem_id);
        std::vector<Point> vertices{
            node_neigh_elem.node_ref(0), node_neigh_elem.node_ref(1), node_neigh_elem.node_ref(2)};
        Plane elem_plane(vertices[0], vertices[1], vertices[2]);
        unsigned int j = node_neigh_elem.local_node(node.id());
        Point normal_at_node_j = elem_plane.unit_normal(vertices[0]);
        unsigned int m = j + 1 < 3 ? j + 1 : j + 1 - 3;
        unsigned int n = j + 2 < 3 ? j + 2 : j + 2 - 3;
        Point line_1 = node_neigh_elem.node_ref(j) - node_neigh_elem.node_ref(m);
        Point line_2 = node_neigh_elem.node_ref(j) - node_neigh_elem.node_ref(n);
        Real dot = line_1 * line_2;
        Real lenSq1 = line_1 * line_1;
        Real lenSq2 = line_2 * line_2;
        Real angle = std::acos(dot / std::sqrt(lenSq1 * lenSq2));
        normal_at_node += normal_at_node_j * angle;
        angle_sum += angle;
      }
      normal[1 + i] = normal_at_node;
    }

    for (unsigned int i = 0; i < elem->n_sides(); i++)
    {
      std::vector<Point> vertices{elem->node_ref(0), elem->node_ref(1), elem->node_ref(2)};

      Plane elem_plane(vertices[0], vertices[1], vertices[2]);
      Point normal_at_edge = libMesh::pi * elem_plane.unit_normal(vertices[0]);

      const Elem * neighbor = elem->neighbor_ptr(i);

      if (neighbor != nullptr)
      {
        std::vector<Point> vertices{
            neighbor->node_ref(0), neighbor->node_ref(1), neighbor->node_ref(2)};

        Plane elem_plane(vertices[0], vertices[1], vertices[2]);
        normal_at_edge += libMesh::pi * elem_plane.unit_normal(vertices[0]);
      }
      normal[4 + i] = normal_at_edge;
    }
    _pseudo_normal.insert(std::make_pair(elem->id(), normal));
  }
}

Point
InterfaceMeshCut3DUserObject::nodeNormal(const unsigned int & node_id)
{
  Point normal(0.0);

  for (const auto & node_neigh_elem_id : _node_to_elem_map[node_id])
  {
    const auto & elem = _cutter_mesh->elem_ref(node_neigh_elem_id);
    Plane elem_plane(elem.node_ref(0), elem.node_ref(1), elem.node_ref(2));
    normal += elem_plane.unit_normal(elem.node_ref(0));
  }

  unsigned int num = _node_to_elem_map[node_id].size();
  return normal / num;
}

bool
InterfaceMeshCut3DUserObject::cutElementByGeometry(const Elem * /*elem*/,
                                                   std::vector<Xfem::CutEdge> & /*cut_edges*/,
                                                   std::vector<Xfem::CutNode> & /*cut_nodes*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
InterfaceMeshCut3DUserObject::cutElementByGeometry(const Elem * elem,
                                                   std::vector<Xfem::CutFace> & cut_faces) const
{
  mooseAssert(elem->dim() == 3, "Dimension of element to be cut must be 3");

  bool elem_cut = false;

  for (unsigned int i = 0; i < elem->n_sides(); ++i)
  {
    // This returns the lowest-order type of side.
    std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);

    mooseAssert(curr_side->dim() == 2, "Side dimension must be 2");

    unsigned int n_edges = curr_side->n_sides();

    std::vector<unsigned int> cut_edges;
    std::vector<Real> cut_pos;

    for (unsigned int j = 0; j < n_edges; j++)
    {
      // This returns the lowest-order type of side.
      std::unique_ptr<const Elem> curr_edge = curr_side->side_ptr(j);
      if (curr_edge->type() != EDGE2)
        mooseError("In cutElementByGeometry face edge must be EDGE2, but type is: ",
                   libMesh::Utility::enum_to_string(curr_edge->type()),
                   " base element type is: ",
                   libMesh::Utility::enum_to_string(elem->type()));
      const Node * node1 = curr_edge->node_ptr(0);
      const Node * node2 = curr_edge->node_ptr(1);

      for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
      {
        std::vector<Point> vertices;

        for (auto & node : cut_elem->node_ref_range())
        {
          Point & this_point = node;
          vertices.push_back(this_point);
        }

        Point intersection;
        if (Xfem::intersectWithEdge(*node1, *node2, vertices, intersection) &&
            std::find(cut_edges.begin(), cut_edges.end(), j) == cut_edges.end())
        {
          cut_edges.push_back(j);
          cut_pos.push_back(Xfem::getRelativePosition(*node1, *node2, intersection));
        }
      }
    }

    // if two edges of an element are cut, it is considered as an element being cut
    if (cut_edges.size() == 2)
    {
      elem_cut = true;
      Xfem::CutFace mycut;
      mycut._face_id = i;
      mycut._face_edge.push_back(cut_edges[0]);
      mycut._face_edge.push_back(cut_edges[1]);
      mycut._position.push_back(cut_pos[0]);
      mycut._position.push_back(cut_pos[1]);
      cut_faces.push_back(mycut);
    }
  }

  return elem_cut;
}

bool
InterfaceMeshCut3DUserObject::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_edges*/,
    std::vector<Xfem::CutEdge> & /*cut_edges*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
InterfaceMeshCut3DUserObject::cutFragmentByGeometry(
    std::vector<std::vector<Point>> & /*frag_faces*/,
    std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("cutFragmentByGeometry not yet implemented for 3D mesh cutting");
  return false;
}

Real
InterfaceMeshCut3DUserObject::calculateSignedDistance(Point p) const
{
  std::vector<Real> distance;
  Real min_dist = std::numeric_limits<Real>::max();
  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    std::vector<Point> vertices{
        cut_elem->node_ref(0), cut_elem->node_ref(1), cut_elem->node_ref(2)};
    unsigned int region;
    Point xp;
    Real dist = Xfem::pointTriangleDistance(
        p, cut_elem->node_ref(0), cut_elem->node_ref(1), cut_elem->node_ref(2), xp, region);

    distance.push_back(std::abs(dist));

    if (dist < std::abs(min_dist))
    {
      min_dist = dist;
      Point normal = (_pseudo_normal.find(cut_elem->id())->second)[region];
      if (normal * (p - xp) < 0.0)
        min_dist *= -1.0;
    }
  }
  std::sort(distance.begin(), distance.end());
  Real sum_dist = 0.0;
  for (std::vector<Real>::iterator it = distance.begin(); it != distance.begin() + 1; ++it)
    sum_dist += *it;

  if (min_dist < 0.0)
    return -sum_dist / 1.0;
  else
    return sum_dist / 1.0;
}
