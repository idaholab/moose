/****************************************************************/
/* DO NOT MODIFY THIS HEADER */
/* MOOSE - Multiphysics Object Oriented Simulation Environment */
/* */
/* (c) 2010 Battelle Energy Alliance, LLC */
/* ALL RIGHTS RESERVED */
/* */
/* Prepared by Battelle Energy Alliance, LLC */
/* Under Contract No. DE-AC07-05ID14517 */
/* With the U. S. Department of Energy */
/* */
/* See COPYRIGHT for full restrictions */
/****************************************************************/
#include "BlockDeleter.h"
<<<<<<< HEAD
#include "MooseMesh.h"
=======
>>>>>>> c9e0414... Initial block deleter for #3080. Does not work on two processors. Also leaves removed nodes hanging, but physics do not propogate across these nodes.

template<>
InputParameters validParams<BlockDeleter>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<SubdomainID>("block_id", "The block to be deleted");
  return params;
}

BlockDeleter::BlockDeleter(const InputParameters & parameters) :
  MeshModifier(parameters),
  _block_id(getParam<SubdomainID>("block_id"))
{}

void
BlockDeleter::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling BlockDeleter::modify()");

  // Reference the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  // set of elements to be deleted
  std::set<Elem *> deletable_elems;

  // Loop over the elements, checking if they should be deleted
  for (MeshBase::element_iterator el = mesh.active_elements_begin(); el != mesh.active_elements_end(); ++el)
  {   
    unsigned int elem_id = (*el)->id();
    if ((*el)->subdomain_id() == _block_id)
    {
      deletable_elems.insert(*el);
    }
  }

  // delete the elements
  std::set<Elem *>::iterator it;
  for (it = deletable_elems.begin() ; it != deletable_elems.end(); ++it)
  {
    mesh.boundary_info->remove(*it);
    mesh.delete_elem(*it);
  }
  mesh.prepare_for_use();
}
