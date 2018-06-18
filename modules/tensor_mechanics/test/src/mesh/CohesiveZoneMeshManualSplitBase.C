/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "CohesiveZoneMeshManualSplitBase.h"
#include "Parser.h"
#include "MooseUtils.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseError.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"

template <>
InputParameters
validParams<CohesiveZoneMeshManualSplitBase>()
{
  InputParameters params = validParams<CohesiveZoneMeshSplitBase>();
  params.addClassDescription("This is the base class used to manually split a "
                             "monolitick mesh and produce an exodus file to  "
                             "comapre the resutl of a mesh split algorithm");
  return params;
}

CohesiveZoneMeshManualSplitBase::CohesiveZoneMeshManualSplitBase(const InputParameters & parameters)
  : CohesiveZoneMeshSplitBase(parameters)
{
  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

CohesiveZoneMeshManualSplitBase::CohesiveZoneMeshManualSplitBase(
    const CohesiveZoneMeshManualSplitBase & other_mesh)
  : CohesiveZoneMeshSplitBase(other_mesh)
{
}

CohesiveZoneMeshManualSplitBase::~CohesiveZoneMeshManualSplitBase() {}

std::unique_ptr<MooseMesh>
CohesiveZoneMeshManualSplitBase::safeClone() const
{
  return libmesh_make_unique<CohesiveZoneMeshManualSplitBase>(*this);
}

void
CohesiveZoneMeshManualSplitBase::init()
{
  mooseError("CohesiveZoneMeshManualSplitBase should never be called directly!"
             "Always use one of its the derived classes");
}

// void
// CohesiveZoneMeshManualSplitBase::displayMeshInfo()
// {
//   // // this method plot mesh informations element by element, usefull to check that mesh splitting is perform correclty
//   // for (MeshBase::element_iterator it = getMesh().elements_begin(); it !=
//   // getMesh().elements_end();
//   //      ++it)
//   // {
//   //   const Elem * elem = *it;
//   //       std::ccoouutt << "elem id = " << elem->id() <<
//   //       std::endl; std::ccoouutt << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>ELEM" << std::endl;
//   //       std::ccoouutt << *elem << std::endl; std::ccoouutt <<
//   //       ">>>>>>>>>>>>>>>>>>>>>>>>>>>>NEIGHBOR_ELEM" << std::endl;
//   //   }
// }

void
CohesiveZoneMeshManualSplitBase::duplicateAndSetLocalNode(dof_id_type element_id,
                                                          dof_id_type local_node)
{

  // method used to duplicate existing nodes given an element and a local node.
  // The new node is added to the mesh and assigned to specified element.

  Elem * elem = getMesh().elem(element_id);              // retrieve element
  dof_id_type original_node_id = elem->node(local_node); // find original node id

  // duplicate node
  Node * new_node = Node::build(getMesh().node(original_node_id), getMesh().n_nodes()).release();
  // assign node to the correct process (i.e. the same process the orignal node had)
  new_node->processor_id() = getMesh().node(original_node_id).processor_id();
  // add node to the mesh
  getMesh().add_node(new_node);

  // set new node
  elem->set_node(local_node) = getMesh().node_ptr(new_node->id());
}

void
CohesiveZoneMeshManualSplitBase::setElemNode(dof_id_type element_id,
                                             dof_id_type local_node,
                                             dof_id_type global_node)
{

  // method used to set a local node given element and global node id
  Elem * elem = getMesh().elem(element_id);          // retrieve element
  Node * new_node = getMesh().node_ptr(global_node); // retrieve node
  elem->set_node(local_node) = getMesh().node_ptr(new_node->id());
}
