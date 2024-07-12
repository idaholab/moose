//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DUserObjectBase.h"
#include "MeshCut2DNucleationBase.h"
#include "CrackFrontDefinition.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"

InputParameters
MeshCut2DUserObjectBase::validParams()
{
  InputParameters params = GeometricCutUserObject::validParams();
  params.addRequiredParam<MeshFileName>(
      "mesh_file",
      "Mesh file for the XFEM geometric cut; currently only the Exodus type is supported");
  params.addParam<UserObjectName>("nucleate_uo", "The MeshCutNucleation UO for nucleating cracks.");
  params.addParam<UserObjectName>("crack_front_definition",
                                  "crackFrontDefinition",
                                  "The CrackFrontDefinition user object name");
  params.addClassDescription("Creates a UserObject base class for a mesh cutter in 2D problems");
  return params;
}

MeshCut2DUserObjectBase::MeshCut2DUserObjectBase(const InputParameters & parameters)
  : GeometricCutUserObject(parameters, true),
    _mesh(_subproblem.mesh()),
    _nucleate_uo(isParamValid("nucleate_uo")
                     ? &getUserObject<MeshCut2DNucleationBase>("nucleate_uo")
                     : nullptr),
    _is_mesh_modified(false)
{
  _depend_uo.insert(getParam<UserObjectName>("crack_front_definition"));

  // only the Exodus type is currently supported
  MeshFileName cutterMeshFileName = getParam<MeshFileName>("mesh_file");
  _cutter_mesh = std::make_unique<ReplicatedMesh>(_communicator);
  _cutter_mesh->read(cutterMeshFileName);
  // test element type; only line elements are allowed
  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    if (cut_elem->n_nodes() != 2)
      mooseError("The input cut mesh should include EDGE2 elements only!");
    if (cut_elem->dim() != 1)
      mooseError("The input cut mesh should have 1D elements (in a 2D space) only!");
  }

  // find node fronts of the original cutmesh.  This is used to order EVERYTHING.
  findOriginalCrackFrontNodes();
}

void
MeshCut2DUserObjectBase::initialSetup()
{
  const auto uo_name = getParam<UserObjectName>("crack_front_definition");
  _crack_front_definition = &_fe_problem.getUserObject<CrackFrontDefinition>(uo_name);
}

bool
MeshCut2DUserObjectBase::cutElementByGeometry(const Elem * elem,
                                              std::vector<Xfem::CutEdge> & cut_edges,
                                              std::vector<Xfem::CutNode> & cut_nodes) const
{
  // With the crack defined by a line, this method cuts a 2D elements by a line
  // Fixme lynn Copy and paste from InterfaceMeshCut2DUserObject::cutElementByGeometry
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
MeshCut2DUserObjectBase::cutElementByGeometry(const Elem * /*elem*/,
                                              std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("Invalid method for 2D mesh cutting.");
  return false;
}

bool
MeshCut2DUserObjectBase::cutFragmentByGeometry(std::vector<std::vector<Point>> & frag_edges,
                                               std::vector<Xfem::CutEdge> & cut_edges) const
{
  bool cut_frag = false;

  for (const auto & cut_elem : _cutter_mesh->element_ptr_range())
  {
    const std::pair<Point, Point> elem_endpoints(cut_elem->node_ref(0), cut_elem->node_ref(1));
    unsigned int n_sides = frag_edges.size();
    for (unsigned int i = 0; i < n_sides; ++i)
    {
      Real seg_int_frac = 0.0;
      if (Xfem::intersectSegmentWithCutLine(
              frag_edges[i][0], frag_edges[i][1], elem_endpoints, 1, seg_int_frac))
      {
        cut_frag = true;
        Xfem::CutEdge mycut;
        mycut._id1 = i;
        mycut._id2 = (i < (n_sides - 1) ? (i + 1) : 0);
        mycut._distance = seg_int_frac;
        mycut._host_side_id = i;
        cut_edges.push_back(mycut);
      }
    }
  }
  return cut_frag;
}

bool
MeshCut2DUserObjectBase::cutFragmentByGeometry(std::vector<std::vector<Point>> & /*frag_faces*/,
                                               std::vector<Xfem::CutFace> & /*cut_faces*/) const
{
  mooseError("Invalid method for 2D mesh fragment cutting.");
  return false;
}

MeshBase &
MeshCut2DUserObjectBase::getCutterMesh() const
{
  mooseAssert(_cutter_mesh, "MeshCut2DUserObjectBase::getCutterMesh _cutter_mesh is nullptr");
  return *_cutter_mesh;
}

const std::vector<Point>
MeshCut2DUserObjectBase::getCrackFrontPoints(unsigned int number_crack_front_points) const
{
  std::vector<Point> crack_front_points(number_crack_front_points);
  // number_crack_front_points is updated via
  // _crack_front_definition->updateNumberOfCrackFrontPoints(_crack_front_points.size())
  if (number_crack_front_points != _original_and_current_front_node_ids.size())
    mooseError("MeshCut2DFractureUserObject::getCrackFrontPoints:  number_crack_front_points=" +
               Moose::stringify(number_crack_front_points) +
               " does not match the number of nodes given in "
               "_original_and_current_front_node_ids=" +
               Moose::stringify(_original_and_current_front_node_ids.size()));

  for (unsigned int i = 0; i < number_crack_front_points; ++i)
  {
    dof_id_type id = _original_and_current_front_node_ids[i].second;
    Node * this_node = _cutter_mesh->node_ptr(id);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;
    crack_front_points[i] = this_point;
  }
  return crack_front_points;
}

const std::vector<RealVectorValue>
MeshCut2DUserObjectBase::getCrackPlaneNormals(unsigned int number_crack_front_points) const
{
  if (number_crack_front_points != _original_and_current_front_node_ids.size())
    mooseError("MeshCut2DFractureUserObject::getCrackPlaneNormals: number_crack_front_points=" +
               Moose::stringify(number_crack_front_points) +
               " does not match the number of nodes given in "
               "_original_and_current_front_node_ids=" +
               Moose::stringify(_original_and_current_front_node_ids.size()) +
               ".  This will happen if a crack front exits the boundary because the number of "
               "points in the CrackFrontDefinition is never updated.");

  std::vector<std::pair<dof_id_type, RealVectorValue>> crack_plane_normals;
  for (const auto & elem : _cutter_mesh->element_ptr_range())
  {
    dof_id_type id0 = elem->node_id(0);
    dof_id_type id1 = elem->node_id(1);
    dof_id_type id;

    auto it0 = std::find_if(_original_and_current_front_node_ids.begin(),
                            _original_and_current_front_node_ids.end(),
                            [&id0](const std::pair<dof_id_type, dof_id_type> & element)
                            { return element.second == id0; });
    auto it1 = std::find_if(_original_and_current_front_node_ids.begin(),
                            _original_and_current_front_node_ids.end(),
                            [&id1](const std::pair<dof_id_type, dof_id_type> & element)
                            { return element.second == id1; });

    bool found_it0 = (it0 != _original_and_current_front_node_ids.end());
    bool found_it1 = (it1 != _original_and_current_front_node_ids.end());

    // Newly nucleated crack elements can have one normal if they are on the edge OR
    // two normals if they are in the bulk.
    if (found_it0)
    {
      Point end_pt, connecting_pt;

      end_pt = elem->node_ref(0);
      connecting_pt = elem->node_ref(1);
      id = it0->first; // sort by original crack front node ids

      Point fracture_dir = end_pt - connecting_pt;
      // The crack normal is orthogonal to the crack extension direction (fracture_dir),
      // and is defined in this implementation as the cross product of the direction of crack
      // extension with the tangent direction, which is always (0, 0, 1) in 2D.
      RealVectorValue normal_dir{fracture_dir(1), -fracture_dir(0), 0};
      normal_dir /= normal_dir.norm();
      crack_plane_normals.push_back(std::make_pair(id, normal_dir));
    }

    if (found_it1)
    {
      Point end_pt, connecting_pt;

      end_pt = elem->node_ref(1);
      connecting_pt = elem->node_ref(0);
      id = it1->first; // sort by original crack front node ids

      Point fracture_dir = end_pt - connecting_pt;
      // The crack normal is orthogonal to the crack extension direction (fracture_dir),
      // and is defined in this implementation as the cross product of the direction of crack
      // extension with the tangent direction, which is always (0, 0, 1) in 2D.
      RealVectorValue normal_dir{fracture_dir(1), -fracture_dir(0), 0};
      normal_dir /= normal_dir.norm();
      crack_plane_normals.push_back(std::make_pair(id, normal_dir));
    }
  }
  mooseAssert(
      _original_and_current_front_node_ids.size() == crack_plane_normals.size(),
      "Boundary nodes are attached to more than one element.  This should not happen for a 1D "
      "cutter mesh."
      "\n    Number of _original_and_current_front_node_ids=" +
          Moose::stringify(_original_and_current_front_node_ids.size()) +
          "\n    Number of crack_plane_normals=" + Moose::stringify(crack_plane_normals.size()));

  // the crack_plane_normals are now sorted by the ORIGINAL crack front ids
  std::sort(crack_plane_normals.begin(), crack_plane_normals.end());
  std::vector<RealVectorValue> sorted_crack_plane_normals;
  for (auto & crack : crack_plane_normals)
    sorted_crack_plane_normals.push_back(crack.second);

  return sorted_crack_plane_normals;
}

void
MeshCut2DUserObjectBase::findOriginalCrackFrontNodes()
{
  std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();
  std::unordered_set boundary_nodes = MeshTools::find_boundary_nodes(*_cutter_mesh);
  for (const auto & node : boundary_nodes)
  {
    auto node_id = node;
    Node * this_node = _cutter_mesh->node_ptr(node_id);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;

    const Elem * elem = (*pl)(this_point);
    if (elem != NULL)
      _original_and_current_front_node_ids.push_back(std::make_pair(node, node));
  }
  std::sort(_original_and_current_front_node_ids.begin(),
            _original_and_current_front_node_ids.end());
}

void
MeshCut2DUserObjectBase::growFront()
{
  dof_id_type current_front_node_id;
  for (std::size_t i = 0; i < _original_and_current_front_node_ids.size(); ++i)
  {
    current_front_node_id = _original_and_current_front_node_ids[i].second;
    // check if node front node id is active
    auto direction_iter =
        std::find_if(_active_front_node_growth_vectors.begin(),
                     _active_front_node_growth_vectors.end(),
                     [&current_front_node_id](const std::pair<dof_id_type, Point> & element)
                     { return element.first == current_front_node_id; });
    // only add an element for active node front ids
    if (direction_iter != _active_front_node_growth_vectors.end())
    {
      Node * this_node = _cutter_mesh->node_ptr(current_front_node_id);
      mooseAssert(this_node, "Node is NULL");
      Point & this_point = *this_node;

      Point new_node_offset = direction_iter->second;
      Point x = this_point + new_node_offset;

      // TODO:  Should check if cut line segment created between "this_point" and "x" crosses
      // another line element in the cutter mesh or solid mesh boundary.
      // Crossing another line element would be a special case that still needs to be handled,
      // however, it doesnot cause an error, it will just ignore the other line segment and recut
      // the solid mesh element.
      // Crossing a solid mesh boundary would be for aesthetics reasons so
      // that element was trimmed close to the boundary but would have not effect on the simulation.
      // Crossing a solid mesh boundary should be handled by something like
      // MeshCut2DRankTwoTensorNucleation::lineLineIntersect2D

      // add node to front
      this_node = Node::build(x, _cutter_mesh->n_nodes()).release();
      _cutter_mesh->add_node(this_node);
      dof_id_type new_front_node_id = _cutter_mesh->n_nodes() - 1;

      // add element to front
      std::vector<dof_id_type> elem;
      elem.push_back(current_front_node_id);
      elem.push_back(new_front_node_id);
      Elem * new_elem = Elem::build(EDGE2).release();
      for (unsigned int i = 0; i < new_elem->n_nodes(); ++i)
      {
        mooseAssert(_cutter_mesh->node_ptr(elem[i]) != nullptr, "Node is NULL");
        new_elem->set_node(i) = _cutter_mesh->node_ptr(elem[i]);
      }
      _cutter_mesh->add_elem(new_elem);
      // now push to the end of _original_and_current_front_node_ids for tracking and fracture
      // integrals
      _original_and_current_front_node_ids[i].second = new_front_node_id;
      _is_mesh_modified = true;
    }
  }
  _cutter_mesh->prepare_for_use();
}

void
MeshCut2DUserObjectBase::addNucleatedCracksToMesh()
{
  if (_nucleate_uo)
  {
    std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>> nucleated_elems_map =
        _nucleate_uo->getNucleatedElemsMap();
    const Real nucleationRadius = _nucleate_uo->getNucleationRadius();

    removeNucleatedCracksTooCloseToEachOther(nucleated_elems_map, nucleationRadius);
    removeNucleatedCracksTooCloseToExistingCracks(nucleated_elems_map, nucleationRadius);

    std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
    pl->enable_out_of_mesh_mode();
    for (const auto & elem_nodes : nucleated_elems_map)
    {
      std::pair<RealVectorValue, RealVectorValue> nodes = elem_nodes.second;
      // add nodes for the elements that define the nucleated cracks
      Node * node_0 = Node::build(nodes.first, _cutter_mesh->n_nodes()).release();
      _cutter_mesh->add_node(node_0);
      dof_id_type node_id_0 = _cutter_mesh->n_nodes() - 1;
      Node * node_1 = Node::build(nodes.second, _cutter_mesh->n_nodes()).release();
      _cutter_mesh->add_node(node_1);
      dof_id_type node_id_1 = _cutter_mesh->n_nodes() - 1;
      // add elements that define nucleated cracks
      std::vector<dof_id_type> elem;
      elem.push_back(node_id_0);
      elem.push_back(node_id_1);
      Elem * new_elem = Elem::build(EDGE2).release();
      for (unsigned int i = 0; i < new_elem->n_nodes(); ++i)
      {
        mooseAssert(_cutter_mesh->node_ptr(elem[i]) != nullptr, "Node is NULL");
        new_elem->set_node(i) = _cutter_mesh->node_ptr(elem[i]);
      }
      _cutter_mesh->add_elem(new_elem);
      // now add the nucleated nodes to the crack id data struct
      // edge nucleated cracks will add one node to _original_and_current_front_node_ids
      // bulk nucleated cracks will add two nodes to _original_and_current_front_node_ids
      Point & point_0 = *node_0;
      const Elem * crack_front_elem_0 = (*pl)(point_0);
      if (crack_front_elem_0 != NULL)
        _original_and_current_front_node_ids.push_back(std::make_pair(node_id_0, node_id_0));

      Point & point_1 = *node_1;
      const Elem * crack_front_elem_1 = (*pl)(point_1);
      if (crack_front_elem_1 != NULL)
        _original_and_current_front_node_ids.push_back(std::make_pair(node_id_1, node_id_1));

      _is_mesh_modified = true;
    }
    _cutter_mesh->prepare_for_use();
  }
}

void
MeshCut2DUserObjectBase::removeNucleatedCracksTooCloseToEachOther(
    std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>> & nucleated_elems_map,
    const Real nucleationRadius)
{
  // remove nucleated elements that are too close too each other.  Lowest key wins
  for (auto it1 = nucleated_elems_map.begin(); it1 != nucleated_elems_map.end(); ++it1)
  {
    std::pair<RealVectorValue, RealVectorValue> nodes = it1->second;
    Point p2 = nodes.first;
    Point p1 = nodes.second;
    Point p = p1 + (p2 - p1) / 2;
    for (auto it2 = nucleated_elems_map.begin(); it2 != nucleated_elems_map.end();)
    {
      if (it1 == it2)
      {
        ++it2;
        continue;
      }

      nodes = it2->second;
      p2 = nodes.first;
      p1 = nodes.second;
      Point q = p1 + (p2 - p1) / 2;
      Point pq = q - p;
      if (pq.norm() <= nucleationRadius)
        it2 = nucleated_elems_map.erase(it2);
      else
        ++it2;
    }
  }
}

void
MeshCut2DUserObjectBase::removeNucleatedCracksTooCloseToExistingCracks(
    std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>> & nucleated_elems_map,
    const Real nucleationRadius)
{
  for (auto it = nucleated_elems_map.begin(); it != nucleated_elems_map.end();)
  {
    std::pair<RealVectorValue, RealVectorValue> nodes = it->second;
    Point p2 = nodes.first;
    Point p1 = nodes.second;
    Point p = p1 + (p2 - p1) / 2;
    bool removeNucleatedElem = false;
    for (const auto & cutter_elem :
         as_range(_cutter_mesh->active_elements_begin(), _cutter_mesh->active_elements_end()))
    {
      const Node * const * cutter_elem_nodes = cutter_elem->get_nodes();
      Point m = *cutter_elem_nodes[1] - *cutter_elem_nodes[0];
      Real t = m * (p - *cutter_elem_nodes[0]) / m.norm_sq();
      Real d = std::numeric_limits<Real>::max();
      if (t <= 0)
      {
        Point j = p - *cutter_elem_nodes[0];
        d = j.norm();
      }
      else if (t >= 0)
      {
        Point j = p - *cutter_elem_nodes[1];
        d = j.norm();
      }
      else
      {
        Point j = p - (*cutter_elem_nodes[0] + t * m);
        d = j.norm();
      }
      if (d <= nucleationRadius)
      {
        removeNucleatedElem = true;
        break;
      }
    }
    if (removeNucleatedElem)
      it = nucleated_elems_map.erase(it);
    else
      ++it;
  }
}
