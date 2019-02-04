//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakMeshByBlockManual_3Blocks.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"

registerMooseObject("MooseTestApp", BreakMeshByBlockManual_3Blocks);

template <>
InputParameters
validParams<BreakMeshByBlockManual_3Blocks>()
{
  InputParameters params = validParams<BreakMeshByBlockManualBase>();
  params.addClassDescription("Manually split the mesh in coh3D_3blocks.e");
  return params;
}

BreakMeshByBlockManual_3Blocks::BreakMeshByBlockManual_3Blocks(const InputParameters & parameters)
  : BreakMeshByBlockManualBase(parameters)

{
}

void
BreakMeshByBlockManual_3Blocks::modify()
{

  // if distributeb mesh raise an error
  _mesh_ptr->errorIfDistributedMesh(
      "BreakMeshByBlockManual_3Blocks only works on a REPLICATED mesh");

  checkInputParameter();

  updateElements();

  addInterfaceBoundary();
}

void
BreakMeshByBlockManual_3Blocks::updateElements()
{
  // specyfing nodes to duplciate
  dof_id_type curr_elem, local_node, global_node;

  // node 0
  curr_elem = 4;
  local_node = 1; // new node 27
  duplicateAndSetLocalNode(curr_elem, local_node);
  // node 3
  curr_elem = 4;
  local_node = 2; // new node 28
  duplicateAndSetLocalNode(curr_elem, local_node);
  curr_elem = 7;
  local_node = 1;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 29

  // node 4
  curr_elem = 4;
  local_node = 5;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 30
  curr_elem = 5;
  local_node = 1;
  global_node = 30;
  setElemNode(curr_elem, local_node, global_node);

  // node 7
  curr_elem = 4;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 31
  curr_elem = 5;
  local_node = 2;
  global_node = 31;
  setElemNode(curr_elem, local_node, global_node);
  curr_elem = 6;
  local_node = 1;
  global_node = 31;
  setElemNode(curr_elem, local_node, global_node);

  curr_elem = 7;
  local_node = 5;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 32

  // node 9
  curr_elem = 7;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 33

  // node 11
  curr_elem = 6;
  local_node = 2;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 34
  curr_elem = 7;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 35

  // node 12
  curr_elem = 5;
  local_node = 5;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 36

  // node 15
  curr_elem = 5;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 37
  curr_elem = 6;
  local_node = 5;
  global_node = 37;
  setElemNode(curr_elem, local_node, global_node);

  // node 17
  curr_elem = 6;
  local_node = 6;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 38

  // node 19
  curr_elem = 7;
  local_node = 0;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 39

  // node 21
  curr_elem = 7;
  local_node = 4;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 40

  // node 24
  curr_elem = 7;
  local_node = 7;
  duplicateAndSetLocalNode(curr_elem, local_node); // new node 41
}

void
BreakMeshByBlockManual_3Blocks::addInterfaceBoundary()
{
  // construct boundary 100, which will contain the cohesive interface
  Elem * elem;
  BoundaryInfo & boundary_info = _mesh_ptr->getMesh().get_boundary_info();
  SubdomainID interface_id = 100;
  if (!_split_interface)
  {
    elem = _mesh_ptr->getMesh().elem_ptr(0);
    boundary_info.add_side(elem->id(), 4, interface_id);

    elem = _mesh_ptr->getMesh().elem_ptr(1);
    boundary_info.add_side(elem->id(), 4, interface_id);

    elem = _mesh_ptr->getMesh().elem_ptr(2);
    boundary_info.add_side(elem->id(), 4, interface_id);

    elem = _mesh_ptr->getMesh().elem_ptr(3);
    boundary_info.add_side(elem->id(), 4, interface_id);

    elem = _mesh_ptr->getMesh().elem_ptr(4);
    boundary_info.add_side(elem->id(), 3, interface_id);

    elem = _mesh_ptr->getMesh().elem_ptr(6);
    boundary_info.add_side(elem->id(), 0, interface_id);

    // rename the boundary
    boundary_info.sideset_name(interface_id) = _interface_name;
  }
  else
  {
    elem = _mesh_ptr->getMesh().elem_ptr(0);
    boundary_info.add_side(elem->id(), 4, 0);

    elem = _mesh_ptr->getMesh().elem_ptr(2);
    boundary_info.add_side(elem->id(), 4, 0);

    elem = _mesh_ptr->getMesh().elem_ptr(3);
    boundary_info.add_side(elem->id(), 4, 0);

    // rename the boundary
    boundary_info.sideset_name(0) = "Block1_Block2";

    elem = _mesh_ptr->getMesh().elem_ptr(1);
    boundary_info.add_side(elem->id(), 4, 4);

    // rename the boundary
    boundary_info.sideset_name(4) = "Block1_Block3";

    elem = _mesh_ptr->getMesh().elem_ptr(4);
    boundary_info.add_side(elem->id(), 3, 5);

    elem = _mesh_ptr->getMesh().elem_ptr(6);
    boundary_info.add_side(elem->id(), 0, 5);

    // rename the boundary
    boundary_info.sideset_name(5) = "Block2_Block3";
  }
}
