//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackMeshCut3DUserObject.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "MooseMesh.h"
#include "MooseEnum.h"
#include "libmesh/face_tri3.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/plane.h"
#include "libmesh/mesh_tools.h"
#include "Function.h"

registerMooseObject("XFEMApp", CrackMeshCut3DUserObject);

InputParameters
CrackMeshCut3DUserObject::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addRequiredParam<MeshFileName>(
      "mesh_file",
      "Mesh file for the XFEM geometric cut; currently only the xda type is supported");
  MooseEnum growthDirection("MAX_HOOP_STRESS FUNCTION", "FUNCTION");
  params.addParam<MooseEnum>(
      "growth_dir_method", growthDirection, "choose from FUNCTION, MAX_HOOP_STRESS");
  MooseEnum growthRate("FATIGUE FUNCTION", "FUNCTION");
  params.addParam<MooseEnum>("growth_rate_method", growthRate, "choose from FUNCTION, FATIGUE");
  params.addParam<FunctionName>("growth_direction_x",
                                "Function defining x-component of crack growth direction");
  params.addParam<FunctionName>("growth_direction_y",
                                "Function defining y-component of crack growth direction");
  params.addParam<FunctionName>("growth_direction_z",
                                "Function defining z-component of crack growth direction");

  params.addParam<FunctionName>("growth_rate", "Function defining crack growth rate");
  params.addParam<Real>(
      "size_control", 0, "Criterion for refining elements while growing the crack");
  params.addParam<unsigned int>("n_step_growth", 0, "Number of steps for crack growth");
  params.addParam<std::vector<dof_id_type>>("crack_front_nodes",
                                            "Set of nodes to define crack front");
  params.addClassDescription("Creates a UserObject for a mesh cutter in 3D problems");
  return params;
}

// This code does not allow predefined crack growth as a function of time
// all inital cracks are defined at t_start = t_end = 0
CrackMeshCut3DUserObject::CrackMeshCut3DUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters, true),
    _mesh(_subproblem.mesh()),
    _growth_dir_method(getParam<MooseEnum>("growth_dir_method").getEnum<GrowthDirectionEnum>()),
    _growth_rate_method(getParam<MooseEnum>("growth_rate_method").getEnum<GrowthRateEnum>()),
    _n_step_growth(getParam<unsigned int>("n_step_growth")),
    _is_mesh_modified(false),
    _func_x(parameters.isParamValid("growth_direction_x") ? &getFunction("growth_direction_x")
                                                          : NULL),
    _func_y(parameters.isParamValid("growth_direction_y") ? &getFunction("growth_direction_y")
                                                          : NULL),
    _func_z(parameters.isParamValid("growth_direction_z") ? &getFunction("growth_direction_z")
                                                          : NULL),
    _func_v(parameters.isParamValid("growth_rate") ? &getFunction("growth_rate") : NULL)
{
  _grow = (_n_step_growth == 0 ? 0 : 1);

  if (_grow)
  {
    if (!isParamValid("size_control"))
      mooseError("Crack growth needs size control");

    _size_control = getParam<Real>("size_control");

    if (_growth_dir_method == GrowthDirectionEnum::FUNCTION &&
        (_func_x == NULL || _func_y == NULL || _func_z == NULL))
      mooseError("function is not specified for the function method that defines growth direction");

    if (_growth_dir_method == GrowthDirectionEnum::FUNCTION && _func_v == NULL)
      mooseError("function is not specified for the function method that defines growth rate");

    if (_growth_dir_method == GrowthDirectionEnum::FUNCTION && _func_v == NULL)
      mooseError("function with a variable is not specified for the fatigue method that defines "
                 "growth rate");

    if (isParamValid("crack_front_nodes"))
    {
      _tracked_crack_front_points = getParam<std::vector<dof_id_type>>("crack_front_nodes");
      _num_crack_front_points = _tracked_crack_front_points.size();
      _cfd = true;
    }
    else
      _cfd = false;
  }

  if ((_growth_dir_method == GrowthDirectionEnum::MAX_HOOP_STRESS ||
       _growth_rate_method == GrowthRateEnum::FATIGUE) &&
      !_cfd)
    mooseError("'crack_front_nodes' is not specified to use crack growth criteria!");

  // only the xda type is currently supported
  MeshFileName xfem_cut_mesh_file = getParam<MeshFileName>("mesh_file");
  _cut_mesh = std::make_unique<ReplicatedMesh>(_communicator);
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
CrackMeshCut3DUserObject::initialSetup()
{
  if (_cfd)
  {
    _crack_front_definition =
        &_fe_problem.getUserObject<CrackFrontDefinition>("crackFrontDefinition");
    _crack_front_points = _tracked_crack_front_points;
  }

  if (_grow)
  {
    findBoundaryNodes();
    findBoundaryEdges();
    sortBoundaryNodes();
  }

  if (_growth_rate_method == GrowthRateEnum::FATIGUE)
  {
    _dn.clear();
    _n.clear();
  }
}

void
CrackMeshCut3DUserObject::initialize()
{
  _is_mesh_modified = false;

  if (_grow)
  {
    if (_t_step == 1)
      _last_step_initialized = 1;

    _stop = 0;

    if (_t_step > 1 && _t_step != _last_step_initialized)
    {
      _last_step_initialized = _t_step;

      for (unsigned int i = 0; i < _n_step_growth; ++i)
      {
        if (_stop != 1)
        {
          findActiveBoundaryNodes();
          findActiveBoundaryDirection();
          _is_mesh_modified = true;
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
  if (_cfd)
    _crack_front_definition->isCutterModified(_is_mesh_modified);
}

bool
CrackMeshCut3DUserObject::cutElementByGeometry(const Elem * /*elem*/,
                                               std::vector<Xfem::CutEdge> & /*cut_edges*/,
                                               std::vector<Xfem::CutNode> & /*cut_nodes*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
CrackMeshCut3DUserObject::cutElementByGeometry(const Elem * elem,
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
CrackMeshCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_edges*/,
                                                std::vector<Xfem::CutEdge> & /*cut_edges*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
CrackMeshCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_faces*/,
                                                std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  // TODO: Need this for branching in 3D
  mooseError("cutFragmentByGeometry not yet implemented for 3D mesh cutting");
  return false;
}

bool
CrackMeshCut3DUserObject::intersectWithEdge(const Point & p1,
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
CrackMeshCut3DUserObject::findIntersection(const Point & p1,
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
CrackMeshCut3DUserObject::isInsideEdge(const Point & p1, const Point & p2, const Point & p) const
{
  Real dotp1 = (p1 - p) * (p2 - p1);
  Real dotp2 = (p2 - p) * (p2 - p1);
  return (dotp1 * dotp2 <= 0.0);
}

Real
CrackMeshCut3DUserObject::getRelativePosition(const Point & p1,
                                              const Point & p2,
                                              const Point & p) const
{
  Real full_len = (p2 - p1).norm();
  Real len_p1_p = (p - p1).norm();
  return len_p1_p / full_len;
}

bool
CrackMeshCut3DUserObject::isInsideCutPlane(const std::vector<Point> & vertices,
                                           const Point & p) const
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
CrackMeshCut3DUserObject::findBoundaryNodes()
{
  auto boundary_node_ids = MeshTools::find_boundary_nodes(*_cut_mesh);
  for (auto it = boundary_node_ids.cbegin(); it != boundary_node_ids.cend(); it++)
  {
    dof_id_type id = *it;
    std::vector<dof_id_type> neighbors;
    _boundary_map[id] = neighbors;
  }
}

void
CrackMeshCut3DUserObject::findBoundaryEdges()
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
CrackMeshCut3DUserObject::sortBoundaryNodes()
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
CrackMeshCut3DUserObject::findDistance(dof_id_type node1, dof_id_type node2)
{
  Node * n1 = _cut_mesh->node_ptr(node1);
  mooseAssert(n1 != nullptr, "Node is NULL");
  Node * n2 = _cut_mesh->node_ptr(node2);
  mooseAssert(n2 != nullptr, "Node is NULL");
  Real distance = (*n1 - *n2).norm();
  return distance;
}

void
CrackMeshCut3DUserObject::refineBoundary()
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
      std::array<Real, 3> x1;
      std::array<Real, 3> x2;

      Node * n1 = _cut_mesh->node_ptr(node1);
      mooseAssert(n1 != nullptr, "Node is NULL");
      Point & p1 = *n1;
      Node * n2 = _cut_mesh->node_ptr(node2);
      mooseAssert(n2 != nullptr, "Node is NULL");
      Point & p2 = *n2;

      for (unsigned int j = 0; j < 3; ++j)
      {
        x1[j] = p1(j);
        x2[j] = p2(j);
      }

      for (unsigned int j = 0; j < n; ++j)
      {
        Point x;
        for (unsigned int k = 0; k < 3; ++k)
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
CrackMeshCut3DUserObject::findActiveBoundaryNodes()
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
CrackMeshCut3DUserObject::findActiveBoundaryDirection()
{
  mooseAssert(!(_cfd && _active_boundary.size() != 1),
              "crack-front-definition using the cutter mesh only supports one active crack front "
              "segment for now");

  _active_direction.clear();

  for (unsigned int i = 0; i < _active_boundary.size(); ++i)
  {
    std::vector<Point> temp;
    Point dir;

    if (_inactive_boundary_pos.size() != 0)
    {
      for (unsigned int j = 0; j < 3; ++j)
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

    if (_growth_dir_method == GrowthDirectionEnum::FUNCTION)
      // loop over active front points
      for (unsigned int j = i1; j < i2; ++j)
      {
        Node * this_node = _cut_mesh->node_ptr(_active_boundary[i][j]);
        mooseAssert(this_node, "Node is NULL");
        Point & this_point = *this_node;
        dir(0) = _func_x->value(0, this_point);
        dir(1) = _func_y->value(0, this_point);
        dir(2) = _func_z->value(0, this_point);

        temp.push_back(dir);
      }
    // determine growth direction based on KI and KII at the crack front
    else if (_growth_dir_method == GrowthDirectionEnum::MAX_HOOP_STRESS)
    {
      const VectorPostprocessorValue & k1 = getVectorPostprocessorValueByName("II_KI_1", "II_KI_1");
      const VectorPostprocessorValue & k2 =
          getVectorPostprocessorValueByName("II_KII_1", "II_KII_1");
      mooseAssert(k1.size() == k2.size(), "KI and KII VPPs should have the same size");
      mooseAssert(k1.size() == _active_boundary[0].size(),
                  "the number of crack front nodes in the self-similar method should equal to the "
                  "size of VPP defined at the crack front");
      mooseAssert(_crack_front_points.size() == _active_boundary[0].size(),
                  "the number of crack front nodes should be the same in _crack_front_points and "
                  "_active_boundary[0]");

      // the node order in _active_boundary[0] and _crack_front_points may be the same or opposite,
      // their correspondence is needed
      std::vector<int> index = getFrontPointsIndex();

      for (unsigned int j = i1; j < i2; ++j)
      {
        int ind = index[j];
        Real theta = 2 * std::atan((k1[ind] - std::sqrt(k1[ind] * k1[ind] + k2[ind] * k2[ind])) /
                                   (4 * k2[ind]));
        RealVectorValue dir_cfc; // growth direction in crack front coord (cfc) system based on the
                                 // max hoop stress criterion
        RealVectorValue
            dir; // growth direction in global coord system based on the max hoop stress criterion
        dir_cfc(0) = std::cos(theta);
        dir_cfc(1) = std::sin(theta);
        dir_cfc(2) = 0;
        dir = _crack_front_definition->rotateFromCrackFrontCoordsToGlobal(dir_cfc, ind);

        temp.push_back(dir);
      }
    }
    else
      mooseError("This growth_dir_method is not pre-defined!");

    if (_inactive_boundary_pos.size() != 0)
    {
      for (unsigned int j = 0; j < 3; ++j)
        dir(j) = 0;
      temp.push_back(dir);
    }

    _active_direction.push_back(temp);
  }

  // normalize the directional vector
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
CrackMeshCut3DUserObject::growFront()
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

      if (_growth_rate_method == GrowthRateEnum::FUNCTION)
        for (unsigned int k = 0; k < 3; ++k)
        {
          Real velo = _func_v->value(0, Point(0, 0, 0));
          x(k) = this_point(k) + dir(k) * velo;
        }
      else if (_growth_rate_method == GrowthRateEnum::FATIGUE)
      {
        // get the number of loading cycles for this growth increament
        if (j == i1)
        {
          unsigned long int dn = (unsigned long int)_func_v->value(0, Point(0, 0, 0));
          _dn.push_back(dn);
          _n.push_back(_n.size() == 0 ? dn : dn + _n[_n.size() - 1]);
        }

        Real growth_size = _growth_size[j];
        for (unsigned int k = 0; k < 3; ++k)
          x(k) = this_point(k) + dir(k) * growth_size;
      }
      else
        mooseError("This growth_rate_method is not pre-defined!");

      this_node = Node::build(x, _cut_mesh->n_nodes()).release();
      _cut_mesh->add_node(this_node);

      dof_id_type id = _cut_mesh->n_nodes() - 1;
      temp.push_back(id);

      if (_cfd)
      {
        auto it = std::find(_tracked_crack_front_points.begin(),
                            _tracked_crack_front_points.end(),
                            _active_boundary[0][j]);
        if (it != _tracked_crack_front_points.end())
        {
          unsigned int pos = std::distance(_tracked_crack_front_points.begin(), it);
          _tracked_crack_front_points[pos] = id;
        }
      }
    }

    _front.push_back(temp);
  }
}

void
CrackMeshCut3DUserObject::sortFrontNodes()
// TBD; it is not needed for current problems but will be useful for fracture growth
{
}

void
CrackMeshCut3DUserObject::findFrontIntersection()
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

      if (_front[i].size() >= 4)
        node_id = _front[i][2];
      else
        node_id = _front[i][1];

      this_node = _cut_mesh->node_ptr(node_id);
      mooseAssert(this_node, "Node is NULL");
      Point & p1 = *this_node;

      node_id = _front[i].back();
      this_node = _cut_mesh->node_ptr(node_id);
      mooseAssert(this_node, "Node is NULL");
      Point & p4 = *this_node;

      if (_front[i].size() >= 4)
        node_id = _front[i][_front[i].size() - 3];
      else
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

        if (_cfd)
          _tracked_crack_front_points[_tracked_crack_front_points.size() - 1] = n;
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

        if (_cfd)
          _tracked_crack_front_points[0] = n;
      }
    }
  }
}

void
CrackMeshCut3DUserObject::refineFront()
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
        std::array<Real, 3> x1;
        std::array<Real, 3> x2;

        Node * this_node = _cut_mesh->node_ptr(node1);
        mooseAssert(this_node, "Node is NULL");
        Point & p1 = *this_node;
        this_node = _cut_mesh->node_ptr(node2);
        mooseAssert(this_node, "Node is NULL");
        Point & p2 = *this_node;

        for (unsigned int j = 0; j < 3; ++j)
        {
          x1[j] = p1(j);
          x2[j] = p2(j);
        }

        for (unsigned int j = 0; j < n; ++j)
        {
          Point x;
          for (unsigned int k = 0; k < 3; ++k)
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

  if (_cfd)
  {
    if (_front[0][0] == _tracked_crack_front_points[0] &&
        _front[0].back() == _tracked_crack_front_points.back())
      _crack_front_points = _front[0];
    else if (_front[0][0] == _tracked_crack_front_points.back() &&
             _front[0].back() == _tracked_crack_front_points[0])
    {
      _crack_front_points = _front[0];
      std::reverse(_crack_front_points.begin(), _crack_front_points.end());
    }
    else
      mooseError("the crack front and the tracked crack front definition must match in terms of "
                 "their end nodes");

    _num_crack_front_points = _crack_front_points.size();
    _crack_front_definition->updateNumberOfCrackFrontPoints(_num_crack_front_points);
  }
}

void
CrackMeshCut3DUserObject::triangulation()
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
CrackMeshCut3DUserObject::joinBoundary()
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
CrackMeshCut3DUserObject::getCrackFrontPoints(unsigned int number_crack_front_points) const
{
  std::vector<Point> crack_front_points(number_crack_front_points);
  // number_crack_front_points is updated via
  // _crack_front_definition->updateNumberOfCrackFrontPoints(_crack_front_points.size())
  if (number_crack_front_points != _crack_front_points.size())
    mooseError("number_points_from_provider does not match the number of nodes given in "
               "crack_front_nodes");
  for (unsigned int i = 0; i < number_crack_front_points; ++i)
  {
    dof_id_type id = _crack_front_points[i];
    Node * this_node = _cut_mesh->node_ptr(id);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;
    crack_front_points[i] = this_point;
  }
  return crack_front_points;
}

const std::vector<RealVectorValue>
CrackMeshCut3DUserObject::getCrackPlaneNormals(unsigned int number_crack_front_points) const
{
  std::vector<RealVectorValue> crack_plane_normals(number_crack_front_points);

  // build the node-to-elems map
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> node_to_elems_map;
  node_to_elems_map.clear();
  for (const auto & elem : _cut_mesh->element_ptr_range())
    for (auto & node : elem->node_ref_range())
      node_to_elems_map[node.id()].push_back(elem->id());

  // build the elem-to-normal map
  std::unordered_map<dof_id_type, RealVectorValue> elem_to_normal_map;
  elem_to_normal_map.clear();
  for (const auto & elem : _cut_mesh->element_ptr_range())
  {
    Point & p1 = *elem->node_ptr(0);
    Point & p2 = *elem->node_ptr(1);
    Point & p3 = *elem->node_ptr(2);
    Plane elem_plane(p3, p2, p1); // to match the current normal of 0,0,-1;
    RealVectorValue normal = elem_plane.unit_normal(p1);
    elem_to_normal_map[elem->id()] = normal;
  }

  // for any front node, the normal is averaged based on the normals of all elements sharing this
  // node this code may fail when the front node has no element connected to it, e.g. refinement at
  // step 1 has to be disabled
  for (unsigned int i = 0; i < number_crack_front_points; ++i)
  {
    dof_id_type id = _crack_front_points[i];
    std::vector<dof_id_type> elems = node_to_elems_map[id];
    unsigned int n_elem = elems.size();

    RealVectorValue normal_avr = 0;
    for (unsigned int j = 0; j < n_elem; ++j)
      normal_avr += elem_to_normal_map[elems[j]];
    normal_avr = normal_avr / n_elem;
    crack_plane_normals[i] = normal_avr;
  }
  return crack_plane_normals;
}

std::vector<int>
CrackMeshCut3DUserObject::getFrontPointsIndex()
{
  // Crack front definition using the cutter mesh currently only supports one active crack front
  // segment
  unsigned int ibnd = 0;
  unsigned int size_this_segment = _active_boundary[ibnd].size();
  unsigned int n_inactive_nodes = _inactive_boundary_pos.size();

  std::vector<int> index(size_this_segment, -1);

  unsigned int i1 = n_inactive_nodes == 0 ? 0 : 1;
  unsigned int i2 = n_inactive_nodes == 0 ? size_this_segment : size_this_segment - 1;

  // loop over active front points
  for (unsigned int j = i1; j < i2; ++j)
  {
    dof_id_type id = _active_boundary[ibnd][j];
    auto it = std::find(_crack_front_points.begin(), _crack_front_points.end(), id);
    index[j] = std::distance(_crack_front_points.begin(), it);
  }

  return index;
}

void
CrackMeshCut3DUserObject::setSubCriticalGrowthSize(std::vector<Real> & growth_size)
{
  _growth_size = growth_size;
}

unsigned int
CrackMeshCut3DUserObject::getNumberOfCrackFrontPoints() const
{
  return _num_crack_front_points;
}
