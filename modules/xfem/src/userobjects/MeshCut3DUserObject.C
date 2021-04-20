//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut3DUserObject.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "MooseMesh.h"
#include "libmesh/face_tri3.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/plane.h"
#include "Function.h"

registerMooseObject("XFEMApp", MeshCut3DUserObject);

InputParameters
MeshCut3DUserObject::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addRequiredParam<MeshFileName>(
      "mesh_file",
      "Mesh file for the XFEM geometric cut; currently only the xda type is supported");
  params.addRequiredParam<FunctionName>("function_x", "Growth function for x direction");
  params.addRequiredParam<FunctionName>("function_y", "Growth function for y direction");
  params.addRequiredParam<FunctionName>("function_z", "Growth function for z direction");
  params.addParam<Real>(
      "size_control", 0, "Criterion for refining elements while growing the crack");
  params.addParam<unsigned int>("n_step_growth", 0, "Number of steps for crack growth");
  params.addClassDescription("Creates a UserObject for a mesh cutter in 3D problems");
  return params;
}

// this code does not allow predefined crack growth as a function of time
// all inital cracks are defined at t_start = t_end = 0
MeshCut3DUserObject::MeshCut3DUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _n_step_growth(getParam<unsigned int>("n_step_growth")),
    _func_x(getFunction("function_x")),
    _func_y(getFunction("function_y")),
    _func_z(getFunction("function_z"))
{
  _grow = (_n_step_growth == 0 ? 0 : 1);

  if (_grow)
  {
    if (!isParamValid("size_control"))
      mooseError("Crack growth needs size control");

    _size_control = getParam<Real>("size_control");
  }

  // only the xda type is currently supported
  MeshFileName xfem_cut_mesh_file = getParam<MeshFileName>("mesh_file");
  _cut_mesh = libmesh_make_unique<ReplicatedMesh>(_communicator);
  _cut_mesh->read(xfem_cut_mesh_file);

  // test element type; only tri3 elements are allowed
  for (const auto & cut_elem : _cut_mesh->element_ptr_range())
  {
    if (cut_elem->n_nodes() != _cut_elem_nnode)
      mooseError("The input cut mesh should include tri elements only!");
    if (cut_elem->dim() != _cut_elem_dim)
      mooseError("The input cut mesh should have 2D elements only!");
  }
}

void
MeshCut3DUserObject::initialSetup()
{
  if (_grow)
  {
    findBoundaryNodes();
    findBoundaryEdges();
    sortBoundaryNodes();
    refineBoundary();
  }
}

void
MeshCut3DUserObject::initialize()
{
  if (_grow)
  {
    if (_t_step == 1)
      _last_step_initialized = 1;

    _stop = 0;

    if (_t_step > 1 && _t_step != _last_step_initialized)
    {
      _last_step_initialized = _t_step;

      for (unsigned int i = 0; i < _n_step_growth; ++i)
        findActiveBoundaryNodes();

      if (_stop != 1)
      {
        findActiveBoundaryDirection();
        growFront();
        sortFrontNodes();
        if (_inactive_boundary_pos.size() != 0)
          findFrontIntersection();
        refineFront();
        triangulation();
        joinBoundary();
      }
    }
  }
}

bool
MeshCut3DUserObject::cutElementByGeometry(const Elem * /*elem*/,
                                          std::vector<Xfem::CutEdge> & /*cut_edges*/,
                                          std::vector<Xfem::CutNode> & /*cut_nodes*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
MeshCut3DUserObject::cutElementByGeometry(const Elem * elem,
                                          std::vector<Xfem::CutFace> & cut_faces) const
// With the crack defined by a planar mesh, this method cuts a solid element by all elements in the
// planar mesh
// TODO: Time evolving cuts not yet supported in 3D (hence the lack of use of the time variable)
{
  bool elem_cut = false;

  if (elem->dim() != _elem_dim)
    mooseError("The structural mesh to be cut by a surface mesh must be 3D!");

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

      for (const auto & cut_elem : _cut_mesh->element_ptr_range())
      {
        std::vector<Point> vertices;

        for (auto & node : cut_elem->node_ref_range())
        {
          Point & this_point = node;
          vertices.push_back(this_point);
        }

        Point intersection;
        if (intersectWithEdge(*node1, *node2, vertices, intersection))
        {
          cut_edges.push_back(j);
          cut_pos.emplace_back(getRelativePosition(*node1, *node2, intersection));
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
MeshCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_edges*/,
                                           std::vector<Xfem::CutEdge> & /*cut_edges*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
MeshCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_faces*/,
                                           std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  // TODO: Need this for branching in 3D
  mooseError("cutFragmentByGeometry not yet implemented for 3D mesh cutting");
  return false;
}

bool
MeshCut3DUserObject::intersectWithEdge(const Point & p1,
                                       const Point & p2,
                                       const std::vector<Point> & vertices,
                                       Point & pint) const
{
  bool has_intersection = false;

  Plane elem_plane(vertices[0], vertices[1], vertices[2]);
  Point point = vertices[0];
  Point normal = elem_plane.unit_normal(point);

  std::array<Real, 3> plane_point = {{point(0), point(1), point(2)}};
  std::array<Real, 3> planenormal = {{normal(0), normal(1), normal(2)}};
  std::array<Real, 3> edge_point1 = {{p1(0), p1(1), p1(2)}};
  std::array<Real, 3> edge_point2 = {{p2(0), p2(1), p2(2)}};
  std::array<Real, 3> cut_point = {{0.0, 0.0, 0.0}};

  if (Xfem::plane_normal_line_exp_int_3d(
          &plane_point[0], &planenormal[0], &edge_point1[0], &edge_point2[0], &cut_point[0]) == 1)
  {
    Point temp_p(cut_point[0], cut_point[1], cut_point[2]);
    if (isInsideCutPlane(vertices, temp_p) && isInsideEdge(p1, p2, temp_p))
    {
      pint = temp_p;
      has_intersection = true;
    }
  }
  return has_intersection;
}

bool
MeshCut3DUserObject::findIntersection(const Point & p1,
                                      const Point & p2,
                                      const std::vector<Point> & vertices,
                                      Point & pint) const
{
  bool has_intersection = false;

  Plane elem_plane(vertices[0], vertices[1], vertices[2]);
  Point point = vertices[0];
  Point normal = elem_plane.unit_normal(point);

  std::array<Real, 3> plane_point = {{point(0), point(1), point(2)}};
  std::array<Real, 3> planenormal = {{normal(0), normal(1), normal(2)}};
  std::array<Real, 3> p_begin = {{p1(0), p1(1), p1(2)}};
  std::array<Real, 3> p_end = {{p2(0), p2(1), p2(2)}};
  std::array<Real, 3> cut_point = {{0.0, 0.0, 0.0}};

  if (Xfem::plane_normal_line_exp_int_3d(
          &plane_point[0], &planenormal[0], &p_begin[0], &p_end[0], &cut_point[0]) == 1)
  {
    Point p(cut_point[0], cut_point[1], cut_point[2]);
    Real dotp = ((p - p1) * (p2 - p1)) / ((p2 - p1) * (p2 - p1));
    if (isInsideCutPlane(vertices, p) && dotp > 1)
    {
      pint = p;
      has_intersection = true;
    }
  }
  return has_intersection;
}

bool
MeshCut3DUserObject::isInsideEdge(const Point & p1, const Point & p2, const Point & p) const
{
  Real dotp1 = (p1 - p) * (p2 - p1);
  Real dotp2 = (p2 - p) * (p2 - p1);
  return (dotp1 * dotp2 <= 0.0);
}

Real
MeshCut3DUserObject::getRelativePosition(const Point & p1, const Point & p2, const Point & p) const
{
  Real full_len = (p2 - p1).norm();
  Real len_p1_p = (p - p1).norm();
  return len_p1_p / full_len;
}

bool
MeshCut3DUserObject::isInsideCutPlane(const std::vector<Point> & vertices, const Point & p) const
{
  unsigned int n_node = vertices.size();

  Plane elem_plane(vertices[0], vertices[1], vertices[2]);
  Point normal = elem_plane.unit_normal(vertices[0]);

  bool inside = false;
  unsigned int counter = 0;

  for (unsigned int i = 0; i < n_node; ++i)
  {
    unsigned int iplus1 = (i < n_node - 1 ? i + 1 : 0);
    Point middle2p = p - 0.5 * (vertices[i] + vertices[iplus1]);
    const Point side_tang = vertices[iplus1] - vertices[i];
    Point side_norm = side_tang.cross(normal);
    Xfem::normalizePoint(middle2p);
    Xfem::normalizePoint(side_norm);
    if (middle2p * side_norm <= 0.0)
      counter += 1;
  }
  if (counter == n_node)
    inside = true;
  return inside;
}

void
MeshCut3DUserObject::findBoundaryNodes()
{
  unsigned int n_nodes = _cut_mesh->n_nodes();
  std::vector<Real> angle(n_nodes, 0); // this assumes that the cutter mesh has compressed node id
  std::vector<dof_id_type> node_id(_cut_elem_nnode);

  std::vector<Point> vertices(_cut_elem_nnode);

  for (const auto & cut_elem : _cut_mesh->element_ptr_range())
  {
    for (unsigned int i = 0; i < _cut_elem_nnode; ++i)
    {
      Node * this_node = cut_elem->node_ptr(i);
      Point & this_point = *this_node;
      vertices[i] = this_point;
      node_id[i] = this_node->id();
    }

    for (unsigned int i = 0; i < _cut_elem_nnode; ++i)
      mooseAssert(node_id[i] < n_nodes, "Node ID is out of range");

    angle[node_id[0]] += Xfem::angle_rad_3d(&vertices[2](0), &vertices[0](0), &vertices[1](0));
    angle[node_id[1]] += Xfem::angle_rad_3d(&vertices[0](0), &vertices[1](0), &vertices[2](0));
    angle[node_id[2]] += Xfem::angle_rad_3d(&vertices[1](0), &vertices[2](0), &vertices[0](0));
  }

  // In each element, angles at three vertices are calculated.  Angles associated with all nodes are
  // evaluated.
  // Interior nodes will have a total angle = 2*pi; otherwise, it is a boundary node
  // This assumes the cutter surface is flat.
  for (const auto & node : _cut_mesh->node_ptr_range())
  {
    dof_id_type id = node->id();
    if (!MooseUtils::relativeFuzzyEqual(angle[id], libMesh::pi * 2))
    {
      std::vector<dof_id_type> neighbors;
      _boundary_map[id] = neighbors;
    }
  }
}

void
MeshCut3DUserObject::findBoundaryEdges()
{
  _boundary_edges.clear();

  std::vector<dof_id_type> corner_elem_id;
  unsigned int counter = 0;

  std::vector<dof_id_type> node_id(_cut_elem_nnode);
  std::vector<bool> is_node_on_boundary(_cut_elem_nnode);

  for (const auto & cut_elem : _cut_mesh->element_ptr_range())
  {
    for (unsigned int i = 0; i < _cut_elem_nnode; ++i)
    {
      node_id[i] = cut_elem->node_ptr(i)->id();
      is_node_on_boundary[i] = (_boundary_map.find(node_id[i]) != _boundary_map.end());
    }

    if (is_node_on_boundary[0] && is_node_on_boundary[1] && is_node_on_boundary[2])
    {
      // this is an element at the corner; all nodes are on the boundary but not all edges are on
      // the boundary
      corner_elem_id.push_back(counter);
    }
    else
    {
      // for other elements, find and store boundary edges
      for (unsigned int i = 0; i < _cut_elem_nnode; ++i)
      {
        // if both nodes on an edge are on the boundary, it is a boundary edge.
        if (is_node_on_boundary[i] && is_node_on_boundary[(i + 1 <= 2) ? i + 1 : 0])
        {
          dof_id_type node1 = node_id[i];
          dof_id_type node2 = node_id[(i + 1 <= 2) ? i + 1 : 0];
          if (node1 > node2)
            std::swap(node1, node2);

          Xfem::CutEdge ce;

          if (node1 > node2)
            std::swap(node1, node2);
          ce._id1 = node1;
          ce._id2 = node2;

          _boundary_edges.insert(ce);
        }
      }
    }
    ++counter;
  }

  // loop over edges in corner elements
  // if an edge is shared by two elements, it is not an boundary edge (is_edge_inside = 1)
  for (unsigned int i = 0; i < corner_elem_id.size(); ++i)
  {
    auto elem_it = _cut_mesh->elements_begin();

    for (dof_id_type j = 0; j < corner_elem_id[i]; ++j)
      ++elem_it;
    Elem * cut_elem = *elem_it;

    for (unsigned int j = 0; j < _cut_elem_nnode; ++j)
    {
      bool is_edge_inside = 0;

      dof_id_type node1 = cut_elem->node_ptr(j)->id();
      dof_id_type node2 = cut_elem->node_ptr((j + 1 <= 2) ? j + 1 : 0)->id();
      if (node1 > node2)
        std::swap(node1, node2);

      unsigned int counter = 0;
      for (const auto & cut_elem2 : _cut_mesh->element_ptr_range())
      {
        if (counter != corner_elem_id[i])
        {
          for (unsigned int k = 0; k < _cut_elem_nnode; ++k)
          {
            dof_id_type node3 = cut_elem2->node_ptr(k)->id();
            dof_id_type node4 = cut_elem2->node_ptr((k + 1 <= 2) ? k + 1 : 0)->id();
            if (node3 > node4)
              std::swap(node3, node4);

            if (node1 == node3 && node2 == node4)
            {
              is_edge_inside = 1;
              goto endloop;
            }
          }
        }
        ++counter;
      }
    endloop:
      if (is_edge_inside == 0)
      {
        // store boundary edges
        Xfem::CutEdge ce;

        if (node1 > node2)
          std::swap(node1, node2);
        ce._id1 = node1;
        ce._id2 = node2;

        _boundary_edges.insert(ce);
      }
      else
      {
        // this is not a boundary edge; remove it from existing edge list
        for (auto it = _boundary_edges.begin(); it != _boundary_edges.end();)
        {
          if ((*it)._id1 == node1 && (*it)._id2 == node2)
            it = _boundary_edges.erase(it);
          else
            ++it;
        }
      }
    }
  }
}

void
MeshCut3DUserObject::sortBoundaryNodes()
{
  _boundary.clear();

  for (auto it = _boundary_edges.begin(); it != _boundary_edges.end(); ++it)
  {
    dof_id_type node1 = (*it)._id1;
    dof_id_type node2 = (*it)._id2;

    mooseAssert(_boundary_map.find(node1) != _boundary_map.end(),
                "_boundary_map does not have this key");
    mooseAssert(_boundary_map.find(node2) != _boundary_map.end(),
                "_boundary_map does not have this key");

    _boundary_map.find(node1)->second.push_back(node2);
    _boundary_map.find(node2)->second.push_back(node1);
  }

  auto it = _boundary_map.begin();
  while (it != _boundary_map.end())
  {
    if (it->second.size() != 2)
      mooseError(
          "Boundary nodes in the cutter mesh must have exactly two neighbors; this one has: ",
          it->second.size());
    ++it;
  }

  auto it2 = _boundary_edges.begin();
  dof_id_type node1 = (*it2)._id1;
  dof_id_type node2 = (*it2)._id2;
  _boundary.push_back(node1);
  _boundary.push_back(node2);

  for (unsigned int i = 0; i < _boundary_edges.size() - 1; ++i)
  {
    mooseAssert(_boundary_map.find(node2) != _boundary_map.end(),
                "_boundary_map does not have this key");

    dof_id_type node3 = _boundary_map.find(node2)->second[0];
    dof_id_type node4 = _boundary_map.find(node2)->second[1];

    if (node3 == node1)
    {
      _boundary.push_back(node4);
      node1 = node2;
      node2 = node4;
    }
    else if (node4 == node1)
    {
      _boundary.push_back(node3);
      node1 = node2;
      node2 = node3;
    }
    else
      mooseError("Discontinuity in cutter boundary");
  }
}

Real
MeshCut3DUserObject::findDistance(dof_id_type node1, dof_id_type node2)
{
  Node * n1 = _cut_mesh->node_ptr(node1);
  mooseAssert(n1 != nullptr, "Node is NULL");
  Node * n2 = _cut_mesh->node_ptr(node2);
  mooseAssert(n2 != nullptr, "Node is NULL");
  Real distance = (*n1 - *n2).norm();
  return distance;
}

void
MeshCut3DUserObject::refineBoundary()
{
  std::vector<dof_id_type> new_boundary_order(_boundary.begin(), _boundary.end());

  mooseAssert(_boundary.size() >= 2, "Boundary should have at least two nodes");

  for (unsigned int i = _boundary.size() - 1; i >= 1; --i)
  {
    dof_id_type node1 = _boundary[i - 1];
    dof_id_type node2 = _boundary[i];

    Real distance = findDistance(node1, node2);

    if (distance > _size_control)
    {
      unsigned int n = static_cast<unsigned int>(distance / _size_control);
      std::array<Real, LIBMESH_DIM> x1;
      std::array<Real, LIBMESH_DIM> x2;

      Node * n1 = _cut_mesh->node_ptr(node1);
      mooseAssert(n1 != nullptr, "Node is NULL");
      Point & p1 = *n1;
      Node * n2 = _cut_mesh->node_ptr(node2);
      mooseAssert(n2 != nullptr, "Node is NULL");
      Point & p2 = *n2;

      for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      {
        x1[j] = p1(j);
        x2[j] = p2(j);
      }

      for (unsigned int j = 0; j < n; ++j)
      {
        Point x;
        for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
          x(k) = x2[k] - (x2[k] - x1[k]) * (j + 1) / (n + 1);

        Node * this_node = Node::build(x, _cut_mesh->n_nodes()).release();
        _cut_mesh->add_node(this_node);

        dof_id_type id = _cut_mesh->n_nodes() - 1;
        auto it = new_boundary_order.begin();
        new_boundary_order.insert(it + i, id);
      }
    }
  }

  _boundary = new_boundary_order;
  mooseAssert(_boundary.size() > 0, "Boundary should not have zero size");
  _boundary.pop_back();
}

void
MeshCut3DUserObject::findActiveBoundaryNodes()
{
  _active_boundary.clear();
  _inactive_boundary_pos.clear();

  std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();

  unsigned int n_boundary = _boundary.size();

  // if the node is outside of the structural model, store its position in _boundary to
  // _inactive_boundary_pos
  for (unsigned int j = 0; j < n_boundary; ++j)
  {
    Node * this_node = _cut_mesh->node_ptr(_boundary[j]);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;

    const Elem * elem = (*pl)(this_point);
    if (elem == NULL)
      _inactive_boundary_pos.push_back(j);
  }

  unsigned int n_inactive_boundary = _inactive_boundary_pos.size();

  // all nodes are inactive, stop
  if (n_inactive_boundary == n_boundary)
    _stop = 1;

  // find and store active boundary segments in "_active_boundary"
  if (n_inactive_boundary == 0)
    _active_boundary.push_back(_boundary);
  else
  {
    for (unsigned int i = 0; i < n_inactive_boundary - 1; ++i)
    {
      if (_inactive_boundary_pos[i + 1] - _inactive_boundary_pos[i] != 1)
      {
        std::vector<dof_id_type> temp;
        for (unsigned int j = _inactive_boundary_pos[i]; j <= _inactive_boundary_pos[i + 1]; ++j)
        {
          temp.push_back(_boundary[j]);
        }
        _active_boundary.push_back(temp);
      }
    }
    if (_inactive_boundary_pos[n_inactive_boundary - 1] - _inactive_boundary_pos[0] <
        n_boundary - 1)
    {
      std::vector<dof_id_type> temp;
      for (unsigned int j = _inactive_boundary_pos[n_inactive_boundary - 1]; j < n_boundary; ++j)
        temp.push_back(_boundary[j]);
      for (unsigned int j = 0; j <= _inactive_boundary_pos[0]; ++j)
        temp.push_back(_boundary[j]);
      _active_boundary.push_back(temp);
    }
  }
}

void
MeshCut3DUserObject::findActiveBoundaryDirection()
// find growth direction of each boundary node; this is currently assgined by parsed functions;
// it will be updated to (1) growth function specified in the input file,
// and/or (2) growth direction determined by fracture mechanics
{
  _active_direction.clear();

  for (unsigned int i = 0; i < _active_boundary.size(); ++i)
  {
    std::vector<Point> temp;
    Point dir;

    if (_inactive_boundary_pos.size() != 0)
    {
      for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
        dir(j) = 0;
      temp.push_back(dir);
    }

    unsigned int i1 = 1;
    unsigned int i2 = _active_boundary[i].size() - 1;
    if (_inactive_boundary_pos.size() == 0)
    {
      i1 = 0;
      i2 = _active_boundary[i].size();
    }

    for (unsigned int j = i1; j < i2; ++j)
    {
      Node * this_node = _cut_mesh->node_ptr(_active_boundary[i][j]);
      mooseAssert(this_node, "Node is NULL");
      Point & this_point = *this_node;

      dir(0) = _func_x.value(0, this_point);
      dir(1) = _func_y.value(0, this_point);
      dir(2) = _func_z.value(0, this_point);

      temp.push_back(dir);
    }

    if (_inactive_boundary_pos.size() != 0)
    {
      for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
        dir(j) = 0;
      temp.push_back(dir);
    }

    _active_direction.push_back(temp);
  }

  Real maxl = 0;

  for (unsigned int i = 0; i < _active_direction.size(); ++i)
    for (unsigned int j = 0; j < _active_direction[i].size(); ++j)
    {
      Point pt = _active_direction[i][j];
      Real length = std::sqrt(pt * pt);
      if (length > maxl)
        maxl = length;
    }

  for (unsigned int i = 0; i < _active_direction.size(); ++i)
    for (unsigned int j = 0; j < _active_direction[i].size(); ++j)
      _active_direction[i][j] /= maxl;
}

void
MeshCut3DUserObject::growFront()
{
  _front.clear();

  for (unsigned int i = 0; i < _active_boundary.size(); ++i)
  {
    std::vector<dof_id_type> temp;

    unsigned int i1 = 1;
    unsigned int i2 = _active_boundary[i].size() - 1;
    if (_inactive_boundary_pos.size() == 0)
    {
      i1 = 0;
      i2 = _active_boundary[i].size();
    }

    for (unsigned int j = i1; j < i2; ++j)
    {
      Node * this_node = _cut_mesh->node_ptr(_active_boundary[i][j]);
      mooseAssert(this_node, "Node is NULL");
      Point & this_point = *this_node;
      Point dir = _active_direction[i][j];

      Point x;
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        x(k) = this_point(k) + dir(k) * _size_control;

      this_node = Node::build(x, _cut_mesh->n_nodes()).release();
      _cut_mesh->add_node(this_node);

      dof_id_type id = _cut_mesh->n_nodes() - 1;
      temp.push_back(id);
    }

    _front.push_back(temp);
  }
}

void
MeshCut3DUserObject::sortFrontNodes()
// TBD; it is not needed for current problems but will be useful for fracture growth
{
}

void
MeshCut3DUserObject::findFrontIntersection()
{
  ConstBndElemRange & range = *_mesh.getBoundaryElementRange();

  for (unsigned int i = 0; i < _front.size(); ++i)
  {
    if (_front[i].size() >= 2)
    {
      std::vector<Point> pint1;
      std::vector<Point> pint2;
      std::vector<Real> length1;
      std::vector<Real> length2;

      Real node_id = _front[i][0];
      Node * this_node = _cut_mesh->node_ptr(node_id);
      mooseAssert(this_node, "Node is NULL");
      Point & p2 = *this_node;

      node_id = _front[i][1];
      this_node = _cut_mesh->node_ptr(node_id);
      mooseAssert(this_node, "Node is NULL");
      Point & p1 = *this_node;

      node_id = _front[i].back();
      this_node = _cut_mesh->node_ptr(node_id);
      mooseAssert(this_node, "Node is NULL");
      Point & p4 = *this_node;

      node_id = _front[i][_front[i].size() - 2];
      this_node = _cut_mesh->node_ptr(node_id);
      mooseAssert(this_node, "Node is NULL");
      Point & p3 = *this_node;

      bool do_inter1 = 1;
      bool do_inter2 = 1;

      std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
      pl->enable_out_of_mesh_mode();
      const Elem * elem = (*pl)(p1);
      if (elem == NULL)
        do_inter1 = 0;
      elem = (*pl)(p4);
      if (elem == NULL)
        do_inter2 = 0;

      for (const auto & belem : range)
      {
        Point pt;
        std::vector<Point> vertices;

        elem = belem->_elem;
        std::unique_ptr<const Elem> curr_side = elem->side_ptr(belem->_side);
        for (unsigned int j = 0; j < curr_side->n_nodes(); ++j)
        {
          const Node * node = curr_side->node_ptr(j);
          const Point & this_point = *node;
          vertices.push_back(this_point);
        }

        if (findIntersection(p1, p2, vertices, pt))
        {
          pint1.push_back(pt);
          length1.push_back((pt - p1) * (pt - p1));
        }
        if (findIntersection(p3, p4, vertices, pt))
        {
          pint2.push_back(pt);
          length2.push_back((pt - p3) * (pt - p3));
        }
      }

      if (length1.size() != 0 && do_inter1)
      {
        auto it1 = std::min_element(length1.begin(), length1.end());
        Point inter1 = pint1[std::distance(length1.begin(), it1)];
        inter1 += (inter1 - p1) * _const_intersection;

        Node * this_node = Node::build(inter1, _cut_mesh->n_nodes()).release();
        _cut_mesh->add_node(this_node);

        mooseAssert(_cut_mesh->n_nodes() - 1 > 0, "The cut mesh should have at least one element.");
        unsigned int n = _cut_mesh->n_nodes() - 1;

        auto it = _front[i].begin();
        _front[i].insert(it, n);
      }

      if (length2.size() != 0 && do_inter2)
      {
        auto it2 = std::min_element(length2.begin(), length2.end());
        Point inter2 = pint2[std::distance(length2.begin(), it2)];
        inter2 += (inter2 - p2) * _const_intersection;

        Node * this_node = Node::build(inter2, _cut_mesh->n_nodes()).release();
        _cut_mesh->add_node(this_node);

        dof_id_type n = _cut_mesh->n_nodes() - 1;

        auto it = _front[i].begin();
        unsigned int m = _front[i].size();
        _front[i].insert(it + m, n);
      }
    }
  }
}

void
MeshCut3DUserObject::refineFront()
{
  std::vector<std::vector<dof_id_type>> new_front(_front.begin(), _front.end());

  for (unsigned int ifront = 0; ifront < _front.size(); ++ifront)
  {
    unsigned int i1 = _front[ifront].size() - 1;
    if (_inactive_boundary_pos.size() == 0)
      i1 = _front[ifront].size();

    for (unsigned int i = i1; i >= 1; --i)
    {
      unsigned int i2 = i;
      if (_inactive_boundary_pos.size() == 0)
        i2 = (i <= _front[ifront].size() - 1 ? i : 0);

      dof_id_type node1 = _front[ifront][i - 1];
      dof_id_type node2 = _front[ifront][i2];
      Real distance = findDistance(node1, node2);

      if (distance > _size_control)
      {
        unsigned int n = static_cast<int>(distance / _size_control);
        std::array<Real, LIBMESH_DIM> x1;
        std::array<Real, LIBMESH_DIM> x2;

        Node * this_node = _cut_mesh->node_ptr(node1);
        mooseAssert(this_node, "Node is NULL");
        Point & p1 = *this_node;
        this_node = _cut_mesh->node_ptr(node2);
        mooseAssert(this_node, "Node is NULL");
        Point & p2 = *this_node;

        for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
        {
          x1[j] = p1(j);
          x2[j] = p2(j);
        }

        for (unsigned int j = 0; j < n; ++j)
        {
          Point x;
          for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
            x(k) = x2[k] - (x2[k] - x1[k]) * (j + 1) / (n + 1);

          Node * this_node = Node::build(x, _cut_mesh->n_nodes()).release();
          _cut_mesh->add_node(this_node);

          dof_id_type id = _cut_mesh->n_nodes() - 1;

          auto it = new_front[ifront].begin();
          new_front[ifront].insert(it + i, id);
        }
      }
    }
  }

  _front = new_front;
}

void
MeshCut3DUserObject::triangulation()
{

  mooseAssert(_active_boundary.size() == _front.size(),
              "_active_boundary and _front should have the same size!");

  if (_inactive_boundary_pos.size() == 0)
  {
    _active_boundary[0].push_back(_active_boundary[0][0]);
    _front[0].push_back(_front[0][0]);
  }

  // loop over active segments
  for (unsigned int k = 0; k < _front.size(); ++k)
  {
    unsigned int n1 = _active_boundary[k].size();
    unsigned int n2 = _front[k].size();

    unsigned int i1 = 0;
    unsigned int i2 = 0;

    // stop when all nodes are associated with an element
    while (!(i1 == n1 - 1 && i2 == n2 - 1))
    {
      std::vector<dof_id_type> elem;

      dof_id_type p1 = _active_boundary[k][i1]; // node in the old front
      dof_id_type p2 = _front[k][i2];           // node in the new front

      if (i1 != n1 - 1 && i2 != n2 - 1)
      {
        dof_id_type p3 = _active_boundary[k][i1 + 1]; // next node in the old front
        dof_id_type p4 = _front[k][i2 + 1];           // next node in the new front

        elem.push_back(p1);
        elem.push_back(p2);

        Real d1 = findDistance(p1, p4);
        Real d2 = findDistance(p3, p2);

        if (d1 < d2)
        {
          elem.push_back(p4);
          i2++;
        }

        else
        {
          elem.push_back(p3);
          i1++;
        }
      }

      else if (i1 == n1 - 1)
      {
        dof_id_type p4 = _front[k][i2 + 1]; // next node in the new front

        elem.push_back(p1);
        elem.push_back(p2);
        elem.push_back(p4);
        i2++;
      }

      else if (i2 == n2 - 1)
      {
        dof_id_type p3 = _active_boundary[k][i1 + 1]; // next node in the old front

        elem.push_back(p1);
        elem.push_back(p2);
        elem.push_back(p3);
        i1++;
      }

      Elem * new_elem = Elem::build(TRI3).release();

      for (unsigned int i = 0; i < _cut_elem_nnode; ++i)
      {
        mooseAssert(_cut_mesh->node_ptr(elem[i]) != nullptr, "Node is NULL");
        new_elem->set_node(i) = _cut_mesh->node_ptr(elem[i]);
      }

      _cut_mesh->add_elem(new_elem);
    }
  }
}

void
MeshCut3DUserObject::joinBoundary()
{
  if (_inactive_boundary_pos.size() == 0)
  {
    _boundary = _front[0];
    _boundary.pop_back();
    return;
  }

  std::vector<dof_id_type> full_front;

  unsigned int size1 = _active_boundary.size();

  for (unsigned int i = 0; i < size1; ++i)
  {
    unsigned int size2 = _active_boundary[i].size();

    dof_id_type bd1 = _active_boundary[i][size2 - 1];
    dof_id_type bd2 = _active_boundary[i + 1 < size1 ? i + 1 : 0][0];

    full_front.insert(full_front.end(), _front[i].begin(), _front[i].end());

    auto it1 = std::find(_boundary.begin(), _boundary.end(), bd1);
    unsigned int pos1 = std::distance(_boundary.begin(), it1);
    auto it2 = std::find(_boundary.begin(), _boundary.end(), bd2);
    unsigned int pos2 = std::distance(_boundary.begin(), it2);

    if (pos1 <= pos2)
      full_front.insert(full_front.end(), _boundary.begin() + pos1, _boundary.begin() + pos2 + 1);
    else
    {
      full_front.insert(full_front.end(), _boundary.begin() + pos1, _boundary.end());
      full_front.insert(full_front.end(), _boundary.begin(), _boundary.begin() + pos2 + 1);
    }
  }

  _boundary = full_front;
}

const std::vector<Point>
MeshCut3DUserObject::getCrackFrontPoints(unsigned int /*num_crack_front_points*/) const
{
  mooseError("getCrackFrontPoints() is not implemented for this object.");
}
