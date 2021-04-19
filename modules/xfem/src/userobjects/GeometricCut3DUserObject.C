//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricCut3DUserObject.h"

// MOOSE includes
#include "MooseError.h"

#include "libmesh/string_to_enum.h"

// XFEM includes
#include "XFEMFuncs.h"

InputParameters
GeometricCut3DUserObject::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addClassDescription("Base class for 3D XFEM Geometric Cut UserObjects");
  return params;
}

GeometricCut3DUserObject::GeometricCut3DUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters), _center(), _normal()
{
}

bool
GeometricCut3DUserObject::cutElementByGeometry(const Elem * /*elem*/,
                                               std::vector<Xfem::CutEdge> & /*cut_edges*/,
                                               std::vector<Xfem::CutNode> & /*cut_nodes*/) const
{
  mooseError("Invalid method: must use vector of element faces for 3D mesh cutting");
  return false;
}

bool
GeometricCut3DUserObject::cutElementByGeometry(const Elem * elem,
                                               std::vector<Xfem::CutFace> & cut_faces) const
// TODO: Time evolving cuts not yet supported in 3D (hence the lack of use of the time variable)
{
  bool cut_elem = false;

  for (unsigned int i = 0; i < elem->n_sides(); ++i)
  {
    // This returns the lowest-order type of side.
    std::unique_ptr<const Elem> curr_side = elem->side_ptr(i);
    if (curr_side->dim() != 2)
      mooseError("In cutElementByGeometry dimension of side must be 2, but it is ",
                 curr_side->dim());
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

      Point intersection;
      if (intersectWithEdge(*node1, *node2, intersection))
      {
        cut_edges.push_back(j);
        cut_pos.push_back(getRelativePosition(*node1, *node2, intersection));
      }
    }

    if (cut_edges.size() == 2)
    {
      cut_elem = true;
      Xfem::CutFace mycut;
      mycut._face_id = i;
      mycut._face_edge.push_back(cut_edges[0]);
      mycut._face_edge.push_back(cut_edges[1]);
      mycut._position.push_back(cut_pos[0]);
      mycut._position.push_back(cut_pos[1]);
      cut_faces.push_back(mycut);
    }
  }

  return cut_elem;
}

bool
GeometricCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_edges*/,
                                                std::vector<Xfem::CutEdge> & /*cut_edges*/) const
{
  mooseError("Invalid method: must use vector of element faces for 3D mesh cutting");
  return false;
}

bool
GeometricCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_faces*/,
                                                std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  // TODO: Need this for branching in 3D
  mooseError("cutFragmentByGeometry not yet implemented for 3D mesh cutting");
  return false;
}

bool
GeometricCut3DUserObject::intersectWithEdge(const Point & p1, const Point & p2, Point & pint) const
{
  bool has_intersection = false;
  double plane_point[3] = {_center(0), _center(1), _center(2)};
  double plane_normal[3] = {_normal(0), _normal(1), _normal(2)};
  double edge_point1[3] = {p1(0), p1(1), p1(2)};
  double edge_point2[3] = {p2(0), p2(1), p2(2)};
  double cut_point[3] = {0.0, 0.0, 0.0};

  if (Xfem::plane_normal_line_exp_int_3d(
          plane_point, plane_normal, edge_point1, edge_point2, cut_point) == 1)
  {
    Point temp_p(cut_point[0], cut_point[1], cut_point[2]);
    if (isInsideCutPlane(temp_p) && isInsideEdge(p1, p2, temp_p))
    {
      pint = temp_p;
      has_intersection = true;
    }
  }
  return has_intersection;
}

bool
GeometricCut3DUserObject::isInsideEdge(const Point & p1, const Point & p2, const Point & p) const
{
  Real dotp1 = (p1 - p) * (p2 - p1);
  Real dotp2 = (p2 - p) * (p2 - p1);
  return (dotp1 * dotp2 <= 0.0);
}

Real
GeometricCut3DUserObject::getRelativePosition(const Point & p1,
                                              const Point & p2,
                                              const Point & p) const
{
  // get the relative position of p from p1
  Real full_len = (p2 - p1).norm();
  Real len_p1_p = (p - p1).norm();
  return len_p1_p / full_len;
}
