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

#include "ElementDeleterBase.h"
#include "MooseMesh.h"
#include "DependencyResolver.h"
#include "libmesh/ExodusII_IO.h"

template<>
InputParameters validParams<ElementDeleterBase>()
{
  InputParameters params = validParams<MeshModifier>();
  return params;
}

ElementDeleterBase::ElementDeleterBase(const InputParameters & parameters) :
    MeshModifier(parameters)
{}

void
ElementDeleterBase::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling ElementDeleterBase::modify()");

  MeshBase & mesh = _mesh_ptr->getMesh();
  DependencyResolver<unsigned int> resolver;

  // Elements that the deleter will remove
  std::set<Elem *> deleteable_elems;

  // Nodes that are attched to a deleted elements which may be orphaned
  std::set<dof_id_type> node_ids_to_check;

  // First let's figure out which elements need to be deleted
  const MeshBase::const_element_iterator end = mesh.elements_end();
  for (MeshBase::const_element_iterator elem_it = mesh.elements_begin(); elem_it != end; ++elem_it)
  {
    Elem * elem = *elem_it;
    if (shouldDelete(elem))
    {
      deleteable_elems.insert(elem);

      // Also, we need to make sure we save off all of the nodes to check later
      unsigned int n_nodes = elem->n_vertices();
      for (unsigned int i = 0; i < n_nodes; ++i)
      {
        const Node * node = elem->get_node(i);
        node_ids_to_check.insert(node->id());
      }
    }
  }

  /**
   * Delete all of the elements
   *
   * TODO: We need to sort these not because they have to be deleted in a certain order in libMesh,
   *       but because the order of deletion might impact what happens to any existing sidesets or nodesets.
   */
  for (std::set<Elem *>::const_iterator it = deleteable_elems.begin(); it != deleteable_elems.end(); ++it)
    mesh.delete_elem(*it);

  /**
   * Now go through and see what nodes have been orphaned and clean them up.
   */
  std::map<dof_id_type, std::vector<dof_id_type> > & node_to_elem_map = _mesh_ptr->nodeToElemMap();
  for (std::set<dof_id_type>::const_iterator it = node_ids_to_check.begin(); it != node_ids_to_check.end(); ++it)
  {
    Node * node = mesh.node_ptr(*it);

    std::vector<dof_id_type> & connected_elems = node_to_elem_map[node->id()];

    // If every connected element is deleted, we'll need to remove this node
    bool connected_elem = false;
    for (std::vector<dof_id_type>::const_iterator jt = connected_elems.begin(); jt != connected_elems.end(); ++jt)
    {
      Elem * elem = mesh.elem(*jt);

      if (deleteable_elems.find(elem) == deleteable_elems.end())
      {
        connected_elem = true;
        break;
      }
    }

    // No connected elements, then delete it!
    if (!connected_elem)
      mesh.delete_node(node);
  }

  /**
   * Deleting nodes and elements leaves NULLs in the mesh datastructure. We need to get rid of those.
   * For now, we'll call contract and notify the SetupMeshComplete Action that we need to re-prepare the
   * mesh.
   */
  mesh.contract();
  _mesh_ptr->needsPrepareForUse();
}

bool
ElementDeleterBase::shouldDelete(const Elem * elem)
{
  /**
   * TODO: This method should be removed and this class made pure virtual, for now, we'll just delete a few elements
   *       out of a well defined 4x4 mesh (all of the middle ones). In a 4x4 GeneratedMesh, the middle elements
   *       will be numbered 5, 6, 9, 10. This will trigger at least one node removal as well.
   */

  // Very simple test - do not merge!
  if (elem->id() == 5 || elem->id() == 6 || elem->id() == 9 || elem->id() == 10)
    return true;


  return false;
}
