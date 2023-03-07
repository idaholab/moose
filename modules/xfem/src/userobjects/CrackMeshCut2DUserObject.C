//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackMeshCut2DUserObject.h"

#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "MooseMesh.h"
#include "MooseEnum.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"
#include "Function.h"

registerMooseObject("XFEMApp", CrackMeshCut2DUserObject);

InputParameters
CrackMeshCut2DUserObject::validParams()
{
  InputParameters params = CrackMeshCut2DUserObjectBase::validParams();
  MooseEnum growthDirection("FUNCTION", "FUNCTION");
  params.addParam<MooseEnum>("growth_dir_method", growthDirection, "choose from FUNCTION");

  MooseEnum growthSpeed("FUNCTION", "FUNCTION");
  params.addParam<MooseEnum>("growth_speed_method", growthSpeed, "choose from FUNCTION");

  params.addRequiredParam<FunctionName>("function_x", "Growth function for x direction");
  params.addRequiredParam<FunctionName>("function_y", "Growth function for y direction");

  params.addRequiredParam<FunctionName>("function_v", "Growth speed function");

  params.addClassDescription("Creates a UserObject for a mesh cutter in 2D problems");
  return params;
}

CrackMeshCut2DUserObject::CrackMeshCut2DUserObject(const InputParameters & parameters)
  : CrackMeshCut2DUserObjectBase(parameters),
    _growth_dir_method(getParam<MooseEnum>("growth_dir_method").getEnum<GrowthDirectionEnum>()),
    _growth_speed_method(getParam<MooseEnum>("growth_speed_method").getEnum<GrowthSpeedEnum>()),
    _func_x(&getFunction("function_x")),
    _func_y(&getFunction("function_y")),
    _func_v(&getFunction("function_v"))
{
}

void
CrackMeshCut2DUserObject::initialSetup()
{
  // setting _time_of_previous_call_to_UO to current time
  _time_of_previous_call_to_UO = _t;
}

void
CrackMeshCut2DUserObject::initialize()
{
  // following logic only calls crack growth function if time changed.
  // This deals with max_xfem_update > 1.  Error if the timestep drops below this
  if ((_t - _time_of_previous_call_to_UO) > libMesh::TOLERANCE)
  {
    findBoundaryNodes();
    findActiveBoundaryNodes();
    findActiveBoundaryDirection();
    growFront();
  }
  else if (_dt < libMesh::TOLERANCE && _t_step != 0)
    mooseError("timestep size must be greater than " + Moose::stringify(libMesh::TOLERANCE) +
               ", for crack to grow. dt=" + Moose::stringify(_dt));

  _time_of_previous_call_to_UO = _t;
}

void
CrackMeshCut2DUserObject::findBoundaryNodes()
{
  _boundary_node_ids.clear();
  std::unordered_set boundary_nodes = MeshTools::find_boundary_nodes(*_cutter_mesh);
  std::copy(boundary_nodes.begin(), boundary_nodes.end(), std::back_inserter(_boundary_node_ids));
}

void
CrackMeshCut2DUserObject::findActiveBoundaryNodes()
{
  _active_boundary_node_ids.clear();
  _inactive_boundary_node_ids.clear();

  std::unique_ptr<PointLocatorBase> pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();

  // if the node is outside of the structural model, store its id to inactive_boundary_node_ids
  for (unsigned int j = 0; j < _boundary_node_ids.size(); ++j)
  {
    auto node_id = _boundary_node_ids[j];
    Node * this_node = _cutter_mesh->node_ptr(node_id);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;

    const Elem * elem = (*pl)(this_point);
    if (elem == NULL)
      _inactive_boundary_node_ids.push_back(node_id);
    else
      _active_boundary_node_ids.push_back(node_id);
  }
}

void
CrackMeshCut2DUserObject::findActiveBoundaryDirection()
{
  _active_node_id_direction.clear();
  for (unsigned int i = 0; i < _active_boundary_node_ids.size(); ++i)
  {
    std::vector<Point> temp;
    Point dir;
    if (_growth_dir_method == GrowthDirectionEnum::FUNCTION)
    {
      // loop over active front points
      Node * this_node = _cutter_mesh->node_ptr(_active_boundary_node_ids[i]);
      mooseAssert(this_node, "Node is NULL");
      Point & this_point = *this_node;
      dir(0) = _func_x->value(_t, this_point);
      dir(1) = _func_y->value(_t, this_point);
      _active_node_id_direction.push_back(dir / dir.norm());
    }
    else
      mooseError("This growth_dir_method is not pre-defined!");
  }
}

void
CrackMeshCut2DUserObject::growFront()
{
  _active_boundary_node_front_ids.clear();
  for (unsigned int i = 0; i < _active_boundary_node_ids.size(); ++i)
  {
    Node * this_node = _cutter_mesh->node_ptr(_active_boundary_node_ids[i]);
    mooseAssert(this_node, "Node is NULL");
    Point & this_point = *this_node;
    Point dir = _active_node_id_direction[i];
    Point x;
    if (_growth_speed_method == GrowthSpeedEnum::FUNCTION)
      for (unsigned int k = 0; k < 3; ++k)
      {
        Real velo = _func_v->value(_t, Point(0, 0, 0));
        x(k) = this_point(k) + dir(k) * velo * _dt;
      }
    else
      mooseError("This growth_speed_method is not pre-defined!");

    // add node to front
    this_node = Node::build(x, _cutter_mesh->n_nodes()).release();
    _cutter_mesh->add_node(this_node);
    dof_id_type id = _cutter_mesh->n_nodes() - 1;
    _active_boundary_node_front_ids.push_back(id);

    // add element to front
    std::vector<dof_id_type> elem;
    elem.push_back(_active_boundary_node_ids[i]);
    elem.push_back(_active_boundary_node_front_ids.back());
    Elem * new_elem = Elem::build(EDGE2).release();
    for (unsigned int i = 0; i < new_elem->n_nodes(); ++i)
    {
      mooseAssert(_cutter_mesh->node_ptr(elem[i]) != nullptr, "Node is NULL");
      new_elem->set_node(i) = _cutter_mesh->node_ptr(elem[i]);
    }
    _cutter_mesh->add_elem(new_elem);
  }
  // rebuild connectivity so that find_boundary_nodes will return the new front node
  _cutter_mesh->prepare_for_use();
}

const std::vector<Point>
CrackMeshCut2DUserObject::getCrackFrontPoints(unsigned int /*number_crack_front_points*/) const
{
  mooseError("getCrackFrontPoints() is not implemented for CrackMeshCut2DUserObject.");
}

const std::vector<RealVectorValue>
CrackMeshCut2DUserObject::getCrackPlaneNormals(unsigned int /*number_crack_front_points*/) const
{
  mooseError("getCrackPlaneNormals() is not implemented for CrackMeshCut2DUserObject.");
}
