//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakMeshByBlockManualBase.h"
#include "MooseMesh.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"

template <>
InputParameters
validParams<BreakMeshByBlockManualBase>()
{
  InputParameters params = validParams<BreakMeshByBlockBase>();
  params.addClassDescription("This is the base class used to manually split a "
                             "monolitick mesh and produce an exodus file to  "
                             "comapre the resutl of a mesh split algorithm");
  return params;
}

BreakMeshByBlockManualBase::BreakMeshByBlockManualBase(const InputParameters & parameters)
  : BreakMeshByBlockBase(parameters)
{
}

void
BreakMeshByBlockManualBase::modify()
{
  mooseError("BreakMeshByBlockManualBase should never be called directly!"
             "Always use one of its the derived classes");
}

void
BreakMeshByBlockManualBase::duplicateAndSetLocalNode(dof_id_type element_id, dof_id_type local_node)
{

  // method used to duplicate existing nodes given an element and a local node.
  // The new node is added to the mesh and assigned to specified element.

  Elem * elem = _mesh_ptr->getMesh().elem_ptr(element_id);  // retrieve element
  dof_id_type original_node_id = elem->node_id(local_node); // find original node id

  // duplicate node
  Node * new_node =
      Node::build(_mesh_ptr->getMesh().node_ref(original_node_id), _mesh_ptr->getMesh().n_nodes())
          .release();
  // assign node to the correct process (i.e. the same process the orignal node had)
  new_node->processor_id() = _mesh_ptr->getMesh().node_ref(original_node_id).processor_id();
  // add node to the mesh
  _mesh_ptr->getMesh().add_node(new_node);

  // set new node
  elem->set_node(local_node) = _mesh_ptr->getMesh().node_ptr(new_node->id());
}

void
BreakMeshByBlockManualBase::setElemNode(dof_id_type element_id,
                                        dof_id_type local_node,
                                        dof_id_type global_node)
{

  // method used to set a local node given element and global node id
  Elem * elem = _mesh_ptr->getMesh().elem_ptr(element_id);      // retrieve element
  Node * new_node = _mesh_ptr->getMesh().node_ptr(global_node); // retrieve node
  elem->set_node(local_node) = _mesh_ptr->getMesh().node_ptr(new_node->id());
}
