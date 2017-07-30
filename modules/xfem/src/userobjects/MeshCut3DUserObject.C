/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MeshCut3DUserObject.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "MooseMesh.h"
#include "libmesh/face_tri3.h"
#include "libmesh/edge_edge2.h"

template <>
InputParameters
validParams<MeshCut3DUserObject>()
{
  InputParameters params = validParams<GeometricCutUserObject>();
  params.addRequiredParam<std::string>("mesh_file", "Mesh file for XFEM geometric cuts");
  params.addParam<Real>("size_control", "Criterion for refining elements while growing the crack");
  params.addParam<unsigned int>("n_step_growth", "Number of steps for crack growth");
  params.addClassDescription("Creates a UserObject for a mesh cutter in 3D problems");
  return params;
}

// currently, this code does not allow predefined crack growth
// all inital cracks have to be defined at t_start = t_end = 0
MeshCut3DUserObject::MeshCut3DUserObject(const InputParameters & parameters)
  : GeometricCutUserObject(parameters), _mesh(_subproblem.mesh())
{
  if (isParamValid("n_step_growth"))
    _n_step_growth = getParam<unsigned int>("n_step_growth");
  else
    _n_step_growth = 0;

  if (_n_step_growth == 0)
    _grow = 0;
  else
    _grow = 1;

  if (_grow && !isParamValid("size_control"))
    mooseError("Crack growth needs size control");

  if (isParamValid("size_control"))
    _size_control = getParam<Real>("size_control");

  std::string xfem_cut_mesh_file = getParam<std::string>("mesh_file");
  _cut_mesh = libmesh_make_unique<ReplicatedMesh>(_communicator);
  _cut_mesh->read(xfem_cut_mesh_file);

  // test element type
  auto elem_it = _cut_mesh->elements_begin();
  auto elem_end = _cut_mesh->elements_end();
  for (; elem_it != elem_end; ++elem_it)
  {
    Elem * cut_elem = *elem_it;
    unsigned int n_node = cut_elem->n_nodes();
    if (n_node != 3)
      mooseError("The input cut mesh should include tri elements only!");
  }

  // this is a test of the growth methods
  // will be moved to other places in a later version
  if (_grow)
  {
    _stop = 0;

    findBoundaryNodes();
    findBoundaryEdges();
    sortBoundaryNodes();
    refineBoundary();

    // visualize crack growth through an output file
    // will be updated to other formats, e.g. *.e in a later version
    std::ofstream myfile;
    myfile.open("mesh_grow.out");

    for (unsigned int i = 0; i < _n_step_growth; ++i)
    {
      findActiveBoundaryNodes();

      if (_stop != 1)
      {
        findActiveBoundaryDirection();
        growFront();
        sortFrontNodes();

        if (_inactive_boundary.size() != 0)
          findFrontIntersection();

        refineFront();
        triangulation();
        join();
      }

      myfile << _cut_mesh->n_nodes() << std::endl;
      myfile << _cut_mesh->n_elem() << std::endl;
      auto node_begin = _cut_mesh->nodes_begin();
      auto node_end = _cut_mesh->nodes_end();
      auto node_it = node_begin;
      unsigned int n = 0;
      for (; node_it != node_end; ++node_it)
      {
        Node * cutnode = *node_it;
        Point & this_point = *cutnode;
        myfile << this_point(0) << " " << this_point(1) << " " << this_point(2) << " " << std::endl;
      }
      auto elem_begin2 = _cut_mesh->elements_begin();
      auto elem_end2 = _cut_mesh->elements_end();
      auto elem_it2 = elem_begin2;
      n = 0;
      for (; elem_it2 != elem_end2; ++elem_it2)
      {
        Elem * cut_elem = *elem_it2;
        std::vector<unsigned int> tri;
        for (unsigned int i = 0; i < cut_elem->n_nodes(); ++i)
        {
          tri.push_back(cut_elem->node(i));
        }
        myfile << tri[0] << " " << tri[1] << " " << tri[2] << std::endl;
      }
    }
    myfile.close();
  }
}

MeshCut3DUserObject::~MeshCut3DUserObject() {}

bool
MeshCut3DUserObject::active(Real time) const
{
  // TBD
  return time >= 0;
}

bool
MeshCut3DUserObject::cutElementByGeometry(const Elem * /*elem*/,
                                          std::vector<CutEdge> & /*cut_edges*/,
                                          Real /*time*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
MeshCut3DUserObject::cutElementByGeometry(const Elem * elem,
                                          std::vector<CutFace> & cut_faces,
                                          Real /*time*/) const
// TODO: Time evolving cuts not yet supported in 3D (hence the lack of use of the time variable)
{
  bool elem_cut = false;

  const MeshBase::element_iterator elem_begin = _cut_mesh->elements_begin();
  MeshBase::element_iterator elem_end = _cut_mesh->elements_end();
  MeshBase::element_iterator elem_it = _cut_mesh->elements_begin();

  for (unsigned int i = 0; i < elem->n_sides(); ++i)
  {
    // This returns the lowest-order type of side.
    std::unique_ptr<Elem> curr_side = elem->side(i);
    if (curr_side->dim() != 2)
      mooseError("In cutElementByGeometry dimension of side must be 2, but it is ",
                 curr_side->dim());
    unsigned int n_edges = curr_side->n_sides();

    std::vector<unsigned int> cut_edges;
    std::vector<Real> cut_pos;

    for (unsigned int j = 0; j < n_edges; j++)
    {
      // This returns the lowest-order type of side.
      std::unique_ptr<Elem> curr_edge = curr_side->side(j);
      if (curr_edge->type() != EDGE2)
        mooseError("In cutElementByGeometry face edge must be EDGE2, but type is: ",
                   libMesh::Utility::enum_to_string(curr_edge->type()),
                   " base element type is: ",
                   libMesh::Utility::enum_to_string(elem->type()));
      Node * node1 = curr_edge->get_node(0);
      Node * node2 = curr_edge->get_node(1);

      elem_it = elem_begin;
      for (; elem_it != elem_end; ++elem_it)
      {
        std::vector<Point> vertices;
        Elem * cut_elem = *elem_it;

        for (unsigned int i = 0; i < cut_elem->n_nodes(); i++)
        {
          Node * node = cut_elem->get_node(i);
          // Point * pt = dynamic_cast<Point *>(node);
          Point & this_point = *node;
          vertices.push_back(this_point);
        }

        Point intersection;
        if (intersectWithEdge(*node1, *node2, vertices, intersection))
        {
          cut_edges.push_back(j);
          cut_pos.push_back(getRelativePosition(*node1, *node2, intersection));
        }
      }
    }

    if (cut_edges.size() == 2)
    {
      elem_cut = true;
      CutFace mycut;
      mycut.face_id = i;
      mycut.face_edge.push_back(cut_edges[0]);
      mycut.face_edge.push_back(cut_edges[1]);
      mycut.position.push_back(cut_pos[0]);
      mycut.position.push_back(cut_pos[1]);
      cut_faces.push_back(mycut);
    }
  }
  return elem_cut;
}

bool
MeshCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_edges*/,
                                           std::vector<CutEdge> & /*cut_edges*/,
                                           Real /*time*/) const
{
  mooseError("invalid method for 3D mesh cutting");
  return false;
}

bool
MeshCut3DUserObject::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_faces*/,
                                           std::vector<CutFace> & /*cut_faces*/,
                                           Real /*time*/) const
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

  Point normal;
  Point center;
  findElemNormal(vertices, normal, center);

  double plane_point[3] = {center(0), center(1), center(2)};
  double planenormal[3] = {normal(0), normal(1), normal(2)};
  double edge_point1[3] = {p1(0), p1(1), p1(2)};
  double edge_point2[3] = {p2(0), p2(1), p2(2)};

  double cut_point[3] = {0.0, 0.0, 0.0};

  if (Xfem::plane_normal_line_exp_int_3d(
          plane_point, planenormal, edge_point1, edge_point2, cut_point) == 1)
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
  // find directional intersection
  // along the positive extension of the vector from p1 to p2
  bool has_intersection = false;

  Point normal;
  Point center;
  findElemNormal(vertices, normal, center);

  double plane_point[3] = {center(0), center(1), center(2)};
  double planenormal[3] = {normal(0), normal(1), normal(2)};
  double p_begin[3] = {p1(0), p1(1), p1(2)};
  double p_end[3] = {p2(0), p2(1), p2(2)};

  double cut_point[3] = {0.0, 0.0, 0.0};

  if (Xfem::plane_normal_line_exp_int_3d(plane_point, planenormal, p_begin, p_end, cut_point) == 1)
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
  // get the relative position of p from p1
  Real full_len = (p2 - p1).norm();
  Real len_p1_p = (p - p1).norm();
  return len_p1_p / full_len;
}

bool
MeshCut3DUserObject::isInsideCutPlane(const std::vector<Point> & vertices, Point p) const
{
  unsigned int n_node = vertices.size();

  Point normal;
  Point center;

  findElemNormal(vertices, normal, center);

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

bool
MeshCut3DUserObject::findElemNormal(const std::vector<Point> & vertices,
                                    Point & normal,
                                    Point & center) const
{
  unsigned int n_node = vertices.size();

  center(0) = center(1) = center(2) = 0;
  normal(0) = normal(1) = normal(2) = 0;

  for (unsigned int i = 0; i < n_node; ++i)
    center += vertices[i];
  center /= n_node;

  for (unsigned int i = 0; i < n_node; ++i)
  {
    unsigned int iplus1(i < n_node - 1 ? i + 1 : 0);
    Point ray1 = vertices[i] - center;
    Point ray2 = vertices[iplus1] - center;
    normal += ray1.cross(ray2);
  }
  normal /= n_node;
  Xfem::normalizePoint(normal);
  return true; // to be done
}

void
MeshCut3DUserObject::findBoundaryNodes()
// this is a simple algorithm simply based on the added angle = 360 degrees
// works fine for planar cutting surface
// for curved cutting surface, need to re-work this subroutine to make it more general
{
  _boundary_map.clear();

  unsigned int n_nodes = _cut_mesh->n_nodes();
  Real angle[n_nodes];
  unsigned int node_id[3];
  const Real PI = 3.14159265358979323846264338327950288419716939937510582;

  for (unsigned int i = 0; i < n_nodes; ++i)
    angle[i] = 0;

  const MeshBase::element_iterator elem_begin = _cut_mesh->elements_begin();
  MeshBase::element_iterator elem_end = _cut_mesh->elements_end();
  MeshBase::element_iterator elem_it = elem_begin;

  for (; elem_it != elem_end; ++elem_it)
  {
    Elem * cut_elem = *elem_it;
    double vertices[3][3];

    for (unsigned int i = 0; i < 3; ++i)
    {
      Node * this_node = cut_elem->get_node(i);
      Point & this_point = *this_node;
      for (unsigned int j = 0; j < 3; ++j)
        vertices[i][j] = this_point(j);
      node_id[i] = this_node->id();
    }

    angle[node_id[0]] += Xfem::angle_rad_3d(&vertices[2][0], &vertices[0][0], &vertices[1][0]);
    angle[node_id[1]] += Xfem::angle_rad_3d(&vertices[0][0], &vertices[1][0], &vertices[2][0]);
    angle[node_id[2]] += Xfem::angle_rad_3d(&vertices[1][0], &vertices[2][0], &vertices[0][0]);
  }

  for (unsigned int i = 0; i < n_nodes; ++i)
  {
    if (abs(angle[i] - PI * 2) > PI * 2 * 0.001)
    {
      _boundary_map[i] = new CutPoint;
      _boundary_map.at(i)->id = i;
      Node * this_node = _cut_mesh->node_ptr(i);
      Point & this_point = *this_node;
      for (unsigned int j = 0; j < 3; ++j)
        _boundary_map.at(i)->coord[j] = this_point(j);
    }
  }
}

void
MeshCut3DUserObject::findBoundaryEdges()
{
  _boundary_edges.clear();

  std::vector<unsigned int> special_boundary_elem_id;
  const MeshBase::element_iterator elem_begin = _cut_mesh->elements_begin();
  MeshBase::element_iterator elem_end = _cut_mesh->elements_end();
  MeshBase::element_iterator elem_it = elem_begin;
  unsigned int counter = -1;

  for (; elem_it != elem_end; ++elem_it)
  {
    counter++;
    Elem * cut_elem = *elem_it;
    unsigned int node_id[3];
    bool is_node_on_boundary[3];

    for (unsigned int i = 0; i < 3; ++i)
    {
      node_id[i] = cut_elem->get_node(i)->id();
      is_node_on_boundary[i] = (_boundary_map.find(node_id[i]) != _boundary_map.end());
    }

    if (is_node_on_boundary[0] && is_node_on_boundary[1] && is_node_on_boundary[2])
    {
      // store special boundary elements
      special_boundary_elem_id.push_back(counter);
    }
    else
    {
      // for other elements, store boundary edges
      for (unsigned int i = 0; i < 3; ++i)
      {
        if (is_node_on_boundary[i] && is_node_on_boundary[(i + 1 <= 2) ? i + 1 : 0])
        {
          unsigned int node1 = node_id[i];
          unsigned int node2 = node_id[(i + 1 <= 2) ? i + 1 : 0];
          if (node1 > node2)
            std::swap(node1, node2);
          _boundary_edges.push_back(new CutEdge);
          _boundary_edges[_boundary_edges.size() - 1]->id1 = node1;
          _boundary_edges[_boundary_edges.size() - 1]->id2 = node2;
        }
      }
    }
  }

  // loop over edges in special boundary elements
  for (unsigned int i = 0; i < special_boundary_elem_id.size(); ++i)
  {
    elem_it = elem_begin;
    for (unsigned int j = 0; j < special_boundary_elem_id[i]; ++j)
      ++elem_it;
    Elem * cut_elem = *elem_it;

    for (unsigned int j = 0; j < 3; ++j)
    {
      bool is_edge_inside = 0;

      unsigned int node1 = cut_elem->get_node(j)->id();
      unsigned int node2 = cut_elem->get_node((j + 1 <= 2) ? j + 1 : 0)->id();
      if (node1 > node2)
        std::swap(node1, node2);

      MeshBase::element_iterator elem_it2 = elem_begin;
      unsigned int counter = -1;
      for (; elem_it2 != elem_end; ++elem_it2)
      {
        ++counter;

        if (counter != special_boundary_elem_id[i])
        {
          Elem * cut_elem2 = *elem_it2;
          for (unsigned int k = 0; k < 3; ++k)
          {
            unsigned int node3 = cut_elem2->get_node(k)->id();
            unsigned int node4 = cut_elem2->get_node((k + 1 <= 2) ? k + 1 : 0)->id();
            if (node3 > node4)
              std::swap(node3, node4);

            if (node1 == node3 && node2 == node4)
            {
              is_edge_inside = 1;
              goto endloop;
            }
          }
        }
      }
    endloop:
      if (is_edge_inside == 0)
      {
        // store good edges
        _boundary_edges.push_back(new CutEdge);
        _boundary_edges[_boundary_edges.size() - 1]->id1 = node1;
        _boundary_edges[_boundary_edges.size() - 1]->id2 = node2;
      }
      else
      {
        // this is a bad edge; remove it from existing edge list
        for (unsigned int k = 0; k < _boundary_edges.size(); ++k)
        {
          if (_boundary_edges[k]->id1 == node1 && _boundary_edges[k]->id2 == node2)
          {
            _boundary_edges.erase(_boundary_edges.begin() + k);
            break;
          }
        }
      }
    }
  }
}

void
MeshCut3DUserObject::sortBoundaryNodes()
{
  _boundary.clear();

  for (unsigned int i = 0; i < _boundary_edges.size(); ++i)
  {
    unsigned int node1 = _boundary_edges[i]->id1;
    unsigned int node2 = _boundary_edges[i]->id2;
    _boundary_map.at(node1)->neighbor.push_back(node2);
    _boundary_map.at(node2)->neighbor.push_back(node1);
  }

  std::map<int, CutPoint *>::iterator it = _boundary_map.begin();
  while (it != _boundary_map.end())
  {
    if (it->second->neighbor.size() != 2)
      mooseError("Boundary points in cutter mesh must have exactly two neighbors");
    ++it;
  }

  unsigned int node1 = _boundary_edges[0]->id1;
  unsigned int node2 = _boundary_edges[0]->id2;
  _boundary.push_back(node1);
  _boundary.push_back(node2);
  for (unsigned int i = 0; i < _boundary_edges.size() - 1; ++i)
  {
    unsigned int node3 = _boundary_map.at(node2)->neighbor[0];
    unsigned int node4 = _boundary_map.at(node2)->neighbor[1];
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
MeshCut3DUserObject::findDistance(unsigned int node1, unsigned int node2)
{
  Node * n1 = _cut_mesh->node_ptr(node1);
  Point & p1 = *n1;
  Node * n2 = _cut_mesh->node_ptr(node2);
  Point & p2 = *n2;
  Real distance = std::sqrt(std::pow(p1(0) - p2(0), 2) + std::pow(p1(1) - p2(1), 2) +
                            std::pow(p1(2) - p2(2), 2));
  return distance;
}

void
MeshCut3DUserObject::refineBoundary()
{
  std::vector<int> new_boundary_order(_boundary.begin(), _boundary.end());

  for (int i = _boundary.size() - 2; i >= 0; --i)
  {
    unsigned int node1 = _boundary[i];
    unsigned int node2 = _boundary[i + 1];

    // Real distance = findDistance(node1, node2);
    Real distance = findDistance(node1, node2);

    if (distance > _size_control)
    {
      unsigned int n = static_cast<int>(distance / _size_control);
      Real x1[3];
      Real x2[3];

      Node * n1 = _cut_mesh->node_ptr(node1);
      Point & p1 = *n1;
      Node * n2 = _cut_mesh->node_ptr(node2);
      Point & p2 = *n2;

      for (unsigned int j = 0; j < 3; ++j)
      {
        x1[j] = p1(j);
        x2[j] = p2(j);
      }

      for (unsigned int j = 0; j < n; ++j)
      {
        Real x[3];
        for (unsigned int k = 0; k < 3; ++k)
          x[k] = x2[k] - (x2[k] - x1[k]) * (j + 1) / (n + 1);

        Node * this_node = new Node(x[0], x[1], x[2]);
        _cut_mesh->add_node(this_node);

        int id = _cut_mesh->n_nodes() - 1;
        auto it = new_boundary_order.begin();
        new_boundary_order.insert(it + i + 1, id);
      }
    }
  }

  _boundary.swap(new_boundary_order);
  new_boundary_order.clear();
  _boundary.pop_back();
}

void
MeshCut3DUserObject::findActiveBoundaryNodes()
{
  _active_boundary.clear();
  _inactive_boundary.clear();

  std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();

  for (unsigned int j = 0; j < _boundary.size(); ++j)
  {
    Node * this_node = _cut_mesh->node_ptr(_boundary[j]);
    Point & this_point = *this_node;

    const Elem * elem = (*pl)(this_point);
    if (elem == NULL)
      _inactive_boundary.push_back(j);
  }

  int n_inactive_boundary = _inactive_boundary.size();
  int n_boundary_order = _boundary.size();
  if (n_inactive_boundary == n_boundary_order)
    _stop = 1;

  if (n_inactive_boundary == 0)
    _active_boundary.push_back(_boundary);
  else
  {
    for (unsigned int i = 0; i < n_inactive_boundary - 1; ++i)
    {
      if (_inactive_boundary[i + 1] - _inactive_boundary[i] != 1)
      {
        std::vector<int> temp;
        for (unsigned int j = _inactive_boundary[i]; j <= _inactive_boundary[i + 1]; ++j)
        {
          temp.push_back(_boundary[j]);
        }
        _active_boundary.push_back(temp);
      }
    }
    if (_inactive_boundary[n_inactive_boundary - 1] - _inactive_boundary[0] < n_boundary_order - 1)
    {
      std::vector<int> temp;
      for (unsigned int j = _inactive_boundary[n_inactive_boundary - 1]; j < n_boundary_order; ++j)
        temp.push_back(_boundary[j]);
      for (unsigned int j = 0; j <= _inactive_boundary[0]; ++j)
        temp.push_back(_boundary[j]);
      _active_boundary.push_back(temp);
    }
  }
}

void
MeshCut3DUserObject::findActiveBoundaryDirection()
{
  _active_direction.clear();

  for (unsigned int i = 0; i < _active_boundary.size(); ++i)
  {
    std::vector<Point> temp;
    Point dir;

    if (_inactive_boundary.size() != 0)
    {
      for (unsigned int i = 0; i < 3; ++i)
        dir(i) = 0;
      temp.push_back(dir);
    }

    unsigned int i1 = 1;
    unsigned int i2 = _active_boundary[i].size() - 1;
    if (_inactive_boundary.size() == 0)
    {
      i1 = 0;
      i2 = _active_boundary[i].size();
    }

    for (unsigned int j = i1; j < i2; ++j)
    {
      Node * this_node = _cut_mesh->node_ptr(_active_boundary[i][j]);
      Point & this_point = *this_node;
      Real x[3];
      for (unsigned int j = 0; j < 3; ++j)
        x[j] = this_point(j);

      // Here is hard code of predefined crack growth direction
      // Will be replaced by directions calculated from SIFs, or from parsed functions
      dir(0) = 5 * (x[0] - 0.3) + x[2];
      dir(1) = 5 * (x[1] - 0.5) + (x[2] + x[0]) / 2;
      dir(2) = 5 * (x[2] - 0.1) + x[0];

      temp.push_back(dir);
    }

    if (_inactive_boundary.size() != 0)
    {
      for (unsigned int i = 0; i < 3; ++i)
        dir(i) = 0;
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
    std::vector<int> temp;

    unsigned int i1 = 1;
    unsigned int i2 = _active_boundary[i].size() - 1;
    if (_inactive_boundary.size() == 0)
    {
      i1 = 0;
      i2 = _active_boundary[i].size();
    }

    for (unsigned int j = i1; j < i2; ++j)
    {
      Node * this_node = _cut_mesh->node_ptr(_active_boundary[i][j]);
      Point & this_point = *this_node;
      Point dir = _active_direction[i][j];
      Real x[3];
      for (unsigned int j = 0; j < 3; ++j)
        x[j] = this_point(j) + dir(j) * _size_control;

      this_node = new Node(x[0], x[1], x[2]);
      _cut_mesh->add_node(this_node);

      int id = _cut_mesh->n_nodes() - 1;
      temp.push_back(id);
    }

    _front.push_back(temp);
  }
}

void
MeshCut3DUserObject::sortFrontNodes()
{
  // to be done
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
      Point & p2 = *this_node;

      node_id = _front[i][1];
      this_node = _cut_mesh->node_ptr(node_id);
      Point & p1 = *this_node;

      node_id = _front[i][_front[i].size() - 1];
      this_node = _cut_mesh->node_ptr(node_id);
      Point & p4 = *this_node;

      node_id = _front[i][_front[i].size() - 2];
      this_node = _cut_mesh->node_ptr(node_id);
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
        std::unique_ptr<Elem> curr_side = elem->side(belem->_side);
        for (unsigned int j = 0; j < curr_side->n_nodes(); ++j)
        {
          Node * node = curr_side->get_node(j);
          Point & this_point = *node;
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
        inter1 += (inter1 - p1) * 0.01;

        this_node = new Node(inter1(0), inter1(1), inter1(2));
        _cut_mesh->add_node(this_node);
        unsigned int n = _cut_mesh->n_nodes() - 1;

        auto it = _front[i].begin();
        _front[i].insert(it, n);
      }

      if (length2.size() != 0 && do_inter2)
      {
        auto it2 = std::min_element(length2.begin(), length2.end());
        Point inter2 = pint2[std::distance(length2.begin(), it2)];
        inter2 += (inter2 - p2) * 0.01;

        this_node = new Node(inter2(0), inter2(1), inter2(2));
        _cut_mesh->add_node(this_node);
        unsigned int n = _cut_mesh->n_nodes() - 1;

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
  std::vector<std::vector<int>> new_front_order(_front.begin(), _front.end());

  for (unsigned int k = 0; k < _front.size(); ++k)
  {
    int i1 = _front[k].size() - 2;
    if (_inactive_boundary.size() == 0)
      i1 = _front[k].size() - 1;

    for (int i = i1; i >= 0; --i)
    {
      unsigned int i2 = i + 1;
      if (_inactive_boundary.size() == 0)
        i2 = (i + 1 <= _front[k].size() - 1 ? i + 1 : 0);

      unsigned int node1 = _front[k][i];
      unsigned int node2 = _front[k][i2];
      Real distance = findDistance(node1, node2);

      if (distance > _size_control)
      {
        unsigned int n = static_cast<int>(distance / _size_control);
        Real x1[3];
        Real x2[3];

        Node * this_node = _cut_mesh->node_ptr(node1);
        Point & p1 = *this_node;
        this_node = _cut_mesh->node_ptr(node2);
        Point & p2 = *this_node;

        for (unsigned int j = 0; j < 3; ++j)
        {
          x1[j] = p1(j);
          x2[j] = p2(j);
        }

        for (unsigned int j = 0; j < n; ++j)
        {
          Real x[3];
          for (unsigned int k = 0; k < 3; ++k)
            x[k] = x2[k] - (x2[k] - x1[k]) * (j + 1) / (n + 1);

          Node * this_node = new Node(x[0], x[1], x[2]);
          _cut_mesh->add_node(this_node);
          int id = _cut_mesh->n_nodes() - 1;

          auto it = new_front_order[k].begin();
          new_front_order[k].insert(it + i + 1, id);
        }
      }
    }
  }

  _front.swap(new_front_order);
  new_front_order.clear();
}

void
MeshCut3DUserObject::triangulation()
{
  if (_inactive_boundary.size() == 0)
  {
    _active_boundary[0].push_back(_active_boundary[0][0]);
    _front[0].push_back(_front[0][0]);
  }

  for (unsigned int k = 0; k < _front.size(); ++k)
  {
    unsigned int n1 = _active_boundary[k].size();
    unsigned int n2 = _front[k].size();

    unsigned int i1 = 0;
    unsigned int i2 = 0;

    while (!(i1 == n1 - 1 && i2 == n2 - 1))
    {
      std::vector<int> elem;
      std::vector<int> type;

      if (i1 != n1 - 1 && i2 != n2 - 1)
      {
        elem.push_back(i1);
        type.push_back(0);
        elem.push_back(i2);
        type.push_back(1);

        int p1 = _active_boundary[k][i1];
        int p2 = _front[k][i2];
        int p3 = _active_boundary[k][i1 + 1];
        int p4 = _front[k][i2 + 1];
        Real d1 = findDistance(p1, p4);
        Real d2 = findDistance(p3, p2);

        if (d1 < d2)
        {
          elem.push_back(i2 + 1);
          type.push_back(1);
          i2++;
        }

        else
        {
          elem.push_back(i1 + 1);
          type.push_back(0);
          i1++;
        }
      }

      else if (i1 == n1 - 1)
      {
        elem.push_back(i1);
        type.push_back(0);
        elem.push_back(i2);
        type.push_back(1);
        elem.push_back(i2 + 1);
        type.push_back(1);
        i2++;
      }

      else if (i2 == n2 - 1)
      {
        elem.push_back(i1);
        type.push_back(0);
        elem.push_back(i2);
        type.push_back(1);
        elem.push_back(i1 + 1);
        type.push_back(0);
        i1++;
      }

      Elem * new_elem = new Tri3;

      for (unsigned int i = 0; i < 3; ++i)
      {
        if (type[i] == 0)
          new_elem->set_node(i) = _cut_mesh->node_ptr(_active_boundary[k][elem[i]]);
        else
          new_elem->set_node(i) = _cut_mesh->node_ptr(_front[k][elem[i]]);
      }

      _cut_mesh->add_elem(new_elem);
    }
  }
}

void
MeshCut3DUserObject::join()
{
  if (_inactive_boundary.size() == 0)
  {
    _front[0].pop_back();
    _boundary.swap(_front[0]);
    return;
  }

  std::vector<int> full_front;

  if (_active_boundary.size() != _front.size())
    mooseError("_active_boundary and _front do not have same size!");

  unsigned int size1 = _active_boundary.size();

  for (unsigned int i = 0; i < size1; ++i)
  {
    unsigned int size2 = _active_boundary[i].size();

    unsigned int bd1 = _active_boundary[i][size2 - 1];
    unsigned int bd2 = _active_boundary[i + 1 < size1 ? i + 1 : 0][0];

    full_front.insert(full_front.end(), _front[i].begin(), _front[i].end());

    auto it1 = std::find(_boundary.begin(), _boundary.end(), bd1);
    int pos1 = std::distance(_boundary.begin(), it1);
    auto it2 = std::find(_boundary.begin(), _boundary.end(), bd2);
    int pos2 = std::distance(_boundary.begin(), it2);

    if (pos1 <= pos2)
      full_front.insert(full_front.end(), _boundary.begin() + pos1, _boundary.begin() + pos2 + 1);
    else
    {
      full_front.insert(full_front.end(), _boundary.begin() + pos1, _boundary.end());
      full_front.insert(full_front.end(), _boundary.begin(), _boundary.begin() + pos2 + 1);
    }
  }
  _boundary.swap(full_front);

  return;
}
