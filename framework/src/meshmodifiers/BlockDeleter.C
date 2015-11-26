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
#include "MooseMesh.h"

//Moose Includes
#include "DependencyResolver.h"

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
  //Cody's test function
  Point center(0.0043, 0, 0.25);
  Real radius_squared = 0.0012*0.0012;
  //Data Structures
  std::vector< unsigned int > elem_list;
  std::vector< unsigned short int > side_list;
  std::vector< short int > id_list;
  
  //Retrieve the Element Boundary data structres from the mesh
  mesh.boundary_info->build_side_list(elem_list, side_list, id_list);

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
  // Dependency Resolver
  DependencyResolver<unsigned int> resolver;

  MeshBase::const_element_iterator it = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end = mesh.active_elements_end();
  for (; it != end; ++it)
  {
    Elem * elem = *it;
    unsigned int elem_id = elem->id();
    Point p = elem->centroid();

    //Check which elements should be deleted
    if (std::pow(p(0)-center(0), 2) + std::pow(p(1)-center(1), 2) + std::pow(p(2)-center(2), 2) <radius_squared)
    {
      deleteable_elems.insert(elem);
      //Check sidesets
      for (unsigned int i=0; i<elem_list.size(); ++i)
      {
	resolver.insertDependency(neighbour->id(), elem->id());
	info_indices.insert(std::pair<unsigned int, unsigned int>(elem->id(), i));
	std::cout << neighbour->id()+1 << " -> " << elem->id()+1 << " on side " << side_list[i] << " id " << id_list[i] << " (index: " << i << ")\n";
      }
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
