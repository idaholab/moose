//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakMeshByBlockManual_2DJunction.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"

registerMooseObject("MooseTestApp", BreakMeshByBlockManual_2DJunction);

template <>
InputParameters
validParams<BreakMeshByBlockManual_2DJunction>()
{
  InputParameters params = validParams<BreakMeshByBlockManualBase>();
  params.addClassDescription("Manually split the mesh in 4ElementJunction.e."
                             "only works with REPLCIATED mesh");
  return params;
}

BreakMeshByBlockManual_2DJunction::BreakMeshByBlockManual_2DJunction(
    const InputParameters & parameters)
  : BreakMeshByBlockManualBase(parameters)
{
}

void
BreakMeshByBlockManual_2DJunction::modify()
{
  // if distributeb mesh raise an error
  _mesh_ptr->errorIfDistributedMesh(
      "BreakMeshByBlockManual_2DJunction only works on a REPLICATED mesh");

  checkInputParameter();

  updateElements();

  addInterfaceBoundary();
}

void
BreakMeshByBlockManual_2DJunction::updateElements()
{
  // specyfing nodes to duplciate

  dof_id_type curr_elem, local_node;

  curr_elem = 1;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 2;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 3;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 1;
  local_node = 1;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 2;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 3;
  local_node = 1;
  duplicateAndSetLocalNode(curr_elem, local_node);

  curr_elem = 3;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node);
}

void
BreakMeshByBlockManual_2DJunction::addInterfaceBoundary()
{
  // construct boundary 100, which will contain the cohesive interface
  Elem * elem;
  BoundaryInfo & boundary_info = _mesh_ptr->getMesh().get_boundary_info();
  SubdomainID interface_id = 100;
  if (!_split_interface)
  {
    elem = _mesh_ptr->getMesh().elem_ptr(0);
    boundary_info.add_side(elem->id(), 0, interface_id);
    boundary_info.add_side(elem->id(), 2, interface_id);

    elem = _mesh_ptr->getMesh().elem_ptr(1);
    boundary_info.add_side(elem->id(), 2, interface_id);

    elem = _mesh_ptr->getMesh().elem_ptr(2);
    boundary_info.add_side(elem->id(), 0, interface_id);

    // rename the boundary
    boundary_info.sideset_name(interface_id) = _interface_name;
  }
  else
  {
    elem = _mesh_ptr->getMesh().elem_ptr(0);
    boundary_info.add_side(elem->id(), 0, 0);
    boundary_info.sideset_name(0) = "Block1_Block2";

    boundary_info.add_side(elem->id(), 2, 5);
    boundary_info.sideset_name(5) = "Block1_Block3";

    elem = _mesh_ptr->getMesh().elem_ptr(1);
    boundary_info.add_side(elem->id(), 2, 6);
    boundary_info.sideset_name(6) = "Block2_Block4";

    elem = _mesh_ptr->getMesh().elem_ptr(2);
    boundary_info.add_side(elem->id(), 0, 7);
    boundary_info.sideset_name(7) = "Block3_Block4";
  }
}
