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
    mooseError2("_mesh_ptr must be initialized before calling ElementDeleterBase::modify()");

  MeshBase & mesh = _mesh_ptr->getMesh();

  // Elements that the deleter will remove
  std::set<Elem *> deleteable_elems;

  // First let's figure out which elements need to be deleted
  const MeshBase::const_element_iterator end = mesh.elements_end();
  for (MeshBase::const_element_iterator elem_it = mesh.elements_begin(); elem_it != end; ++elem_it)
  {
    Elem * elem = *elem_it;
    if (shouldDelete(elem))
      deleteable_elems.insert(elem);
  }

  /**
   * Delete all of the elements
   *
   * TODO: We need to sort these not because they have to be deleted in a certain order in libMesh,
   *       but because the order of deletion might impact what happens to any existing sidesets or nodesets.
   */
  for (const auto & elem : deleteable_elems)
    mesh.delete_elem(elem);

  /**
   * Deleting nodes and elements leaves NULLs in the mesh datastructure. We need to get rid of those.
   * For now, we'll call contract and notify the SetupMeshComplete Action that we need to re-prepare the
   * mesh.
   */
  mesh.contract();
  _mesh_ptr->needsPrepareForUse();
}
