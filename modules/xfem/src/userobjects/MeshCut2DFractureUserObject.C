//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DFractureUserObject.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"

#include "CrackFrontDefinition.h"

registerMooseObject("XFEMApp", MeshCut2DFractureUserObject);

InputParameters
MeshCut2DFractureUserObject::validParams()
{
  InputParameters params = MeshCut2DUserObjectBase::validParams();
  params.addClassDescription("Creates a UserObject for a mesh cutter in 2D problems that will use "
                             "fracture integrals to determine growth");
  params.addRequiredParam<Real>("k_critical", "Critical fracture toughness.");
  params.addRequiredParam<Real>("growth_length_per_timestep",
                                "Length to grow crack per timestep if k>k_critical");
  return params;
}

MeshCut2DFractureUserObject::MeshCut2DFractureUserObject(const InputParameters & parameters)
  : MeshCut2DUserObjectBase(parameters),
    _k_critical_squared((getParam<Real>("k_critical")) * (getParam<Real>("k_critical"))),
    _growth_per_timestep(getParam<Real>("growth_length_per_timestep"))
{
}

void
MeshCut2DFractureUserObject::initialSetup()
{
  _crack_front_definition =
      &_fe_problem.getUserObject<CrackFrontDefinition>("crackFrontDefinition");
  _time_of_previous_call_to_UO = _t;
  findOriginalCrackFrontNodes();
}

void
MeshCut2DFractureUserObject::initialize()
{
  bool is_mesh_modified = false;
  // following logic only calls crack growth function if time changed.
  // This deals with max_xfem_update > 1.
  if ((_t - _time_of_previous_call_to_UO) > libMesh::TOLERANCE)
  {
    if (_time_of_previous_call_to_UO > 0)
    {
      growFront();
      is_mesh_modified = true;
    }
    _crack_front_definition->isCutterModified(is_mesh_modified);
  }
  _time_of_previous_call_to_UO = _t;
}

void
MeshCut2DFractureUserObject::findOriginalCrackFrontNodes()
{
  if (_original_and_current_front_node_ids.empty())
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
  else
    mooseError("MeshCut2DFractureUserObject::findOriginalCrackFrontNodes() should never be called "
               "more than once.  This fills the _original_and_current_front_node_ids data "
               "structure and the order in this data structure must match the node order used to "
               "set-up the CrackFrontDefinition.");
}

std::vector<Point>
MeshCut2DFractureUserObject::findActiveBoundaryDirection(const std::vector<Real> & k1,
                                                         const std::vector<Real> & k2) const
{
  std::vector<Point> active_node_id_direction(k2.size());
  for (unsigned int i = 0; i < active_node_id_direction.size(); ++i)
  {
    // growth direction in crack front coord (cfc) system based on the  max hoop stress criterion
    Real theta = 2 * atan((k1[i] - sqrt(k1[i] * k1[i] + k2[i] * k2[i])) / (4 * k2[i]));
    RealVectorValue dir_cfc;
    dir_cfc(0) = cos(theta);
    dir_cfc(1) = sin(theta);
    dir_cfc(2) = 0;

    // growth direction in global coord system based on the max hoop stress criterion
    RealVectorValue dir_global;
    dir_global = _crack_front_definition->rotateFromCrackFrontCoordsToGlobal(dir_cfc, i);
    active_node_id_direction[i] = Point{dir_global(0), dir_global(1), dir_global(2)};
  }

  return active_node_id_direction;
}

std::vector<Real>
MeshCut2DFractureUserObject::getKSquared(const std::vector<Real> & k1,
                                         const std::vector<Real> & k2) const
{
  std::vector<Real> k_squared(k1.size());
  for (unsigned int i = 0; i < k_squared.size(); ++i)
    k_squared[i] = k1[i] * k1[i] + k2[i] * k2[i];

  return k_squared;
}

void
MeshCut2DFractureUserObject::growFront()
{
  const VectorPostprocessorValue & k1 = getVectorPostprocessorValueByName("II_KI_1", "II_KI_1");
  const VectorPostprocessorValue & k2 = getVectorPostprocessorValueByName("II_KII_1", "II_KII_1");

  mooseAssert(k1.size() == k2.size(), "KI and KII VPPs should have the same size");
  mooseAssert(k1.size() == _original_and_current_front_node_ids.size(),
              "the number of crack front nodes in the should equal to the "
              "size of VPP defined at the crack front");

  std::vector<Point> current_front_node_id_directions(findActiveBoundaryDirection(k1, k2));
  std::vector<Real> k_squared = getKSquared(k1, k2);

  std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();

  for (std::size_t i = 0; i < _original_and_current_front_node_ids.size(); ++i)
  {
    if (k_squared[i] > _k_critical_squared)
    {
      Node * this_node = _cutter_mesh->node_ptr(_original_and_current_front_node_ids[i].second);
      mooseAssert(this_node, "Node is NULL");
      Point & this_point = *this_node;

      Point dir = current_front_node_id_directions[i];
      Point x = this_point + dir * _growth_per_timestep;

      // add node to front
      this_node = Node::build(x, _cutter_mesh->n_nodes()).release();
      _cutter_mesh->add_node(this_node);
      dof_id_type new_front_node_id = _cutter_mesh->n_nodes() - 1;

      // add element to front
      std::vector<dof_id_type> elem;
      elem.push_back(_original_and_current_front_node_ids[i].second);
      elem.push_back(new_front_node_id);
      Elem * new_elem = Elem::build(EDGE2).release();
      for (unsigned int i = 0; i < new_elem->n_nodes(); ++i)
      {
        mooseAssert(_cutter_mesh->node_ptr(elem[i]) != nullptr, "Node is NULL");
        new_elem->set_node(i) = _cutter_mesh->node_ptr(elem[i]);
      }
      _cutter_mesh->add_elem(new_elem);
      // now update active boundary to new front node ids
      _original_and_current_front_node_ids[i].second = new_front_node_id;
    }
  }
  _cutter_mesh->prepare_for_use();
}

const std::vector<Point>
MeshCut2DFractureUserObject::getCrackFrontPoints(unsigned int number_crack_front_points) const
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

// CrackFrontDefinition wants the normal so this gives it the normal for a line element with an
// assumed tangent direction in the [001] direction.
const std::vector<RealVectorValue>
MeshCut2DFractureUserObject::getCrackPlaneNormals(unsigned int number_crack_front_points) const
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
    if (found_it0 || found_it1)
    {

      Point end_pt, connecting_pt;
      // make sure normal crossed with the crack face points in the [0,0,1] direction
      if (found_it0)
      {
        end_pt = elem->node_ref(0);
        connecting_pt = elem->node_ref(1);
        id = it0->first; // sort by original crack front node ids
      }
      else
      {
        end_pt = elem->node_ref(1);
        connecting_pt = elem->node_ref(0);
        id = it1->first; // sort by original crack front node ids
      }

      Point fracture_dir = end_pt - connecting_pt;
      // crack normal is fracture direction cross tangent direction
      // In 2D, tangent_direction is always [001] in CrackFrontDefinition.C so
      // need to make sure [001] x normal_direction gives fracture_dir
      // SO... fracture_dir x [001] gives the correct normal_direction
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
