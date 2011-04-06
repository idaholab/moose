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

#include "ElementDeleter.h"

//Moose Includes
#include "DependencyResolver.h"

//libMesh
#include "elem.h"
#include "boundary_info.h"


ElementDeleter::ElementDeleter(const std::string & name, InputParameters parameters) :
    MeshModifier(name, parameters)

// TODO: Make this work
//  FunctionInterface(parameters)
{}

void
ElementDeleter::modifyMesh(Mesh & mesh)
{
  // TODO: Move the initialization of the function to the contructor - this will require having
  // the problem and functions parsed earlier
  // Function & func = getFunction("function");
  
  // For now lets just make up a function
  // We'll start with a small sphere located at 0.0043 0 0.025 with radius 0.0012

  Point center(0.0043, 0, 0.025);
  Real radius_squard = 0.0012*0.0012;

  unsigned int cube_map[6] = {5, 3, 4, 1, 2, 0};
  unsigned int quad_map[4] = {2, 3, 0, 1};

  std::multimap< unsigned int, unsigned int > info_indices;
  std::set<Elem *> deleteable_elems;

  // Data structures to hold the element boundary information
  std::vector< unsigned int > elem_list;
  std::vector< unsigned short int > side_list;
  std::vector< short int > id_list;

  // Retrieve the Element Boundary data structures from the mesh
  mesh.boundary_info->build_side_list(elem_list, side_list, id_list);

  DependencyResolver<unsigned int> resolver;
  
  MeshBase::const_element_iterator it = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end = mesh.active_elements_end();
  for (; it != end; ++it)
  {
    Elem * elem = *it;
    unsigned int elem_id = elem->id();
    Point p = elem->centroid();

    // Check the function to see which elements should be deleted
    // TODO: Get functions to work so this can be passed in!
    if (std::pow(p(0)-center(0), 2) + std::pow(p(1)-center(1), 2) + std::pow(p(2)-center(2), 2) < radius_squard) 
    {
      deleteable_elems.insert(elem);
      // Check the sidesets
      for (unsigned int i=0; i<elem_list.size(); ++i)
      {
        // TODO: Make this work for arbitrary elements
        Elem * neighbor = elem->neighbor(cube_map[side_list[i]]);
        if (elem_list[i] == elem_id && neighbor != NULL)
        {
          resolver.insertDependency(neighbor->id(), elem->id());
          info_indices.insert(std::pair<unsigned int, unsigned int>(elem->id(), i));
          std::cout << neighbor->id()+1 << " -> " << elem->id()+1 << " on side " << side_list[i] << " id " << id_list[i] << " (index: " << i << ")\n";
        }
      }  
    }
  }

  const std::vector<unsigned int> & sorted = resolver.getSortedValues();
  std::pair<std::multimap<unsigned int, unsigned int>::iterator, std::multimap<unsigned int, unsigned int>::iterator> ret;
  std::multimap<unsigned int, unsigned int>::iterator j;
  
  for (unsigned int i=0; i<sorted.size(); ++i)
  {
    std::cout << sorted[i]+1 << "\n";
    Elem * elem = mesh.elem(sorted[i]);

    for (unsigned int j=0; j<elem->n_sides(); ++j)
    {
      Elem * neighbor = NULL;
      neighbor = elem->neighbor(cube_map[j]);
      if (neighbor != NULL && deleteable_elems.find(elem) != deleteable_elems.end())
      {
        std::vector<short int> ids = mesh.boundary_info->boundary_ids(elem, j);
        for (unsigned int k=0; k<ids.size(); ++k) 
        {
          mesh.boundary_info->add_side(neighbor, j, ids[k]);
          std::cout << "Neighbor " << neighbor->id()+1 << " side " << j << " id " << ids[k] << "\n";
        }
      }
    }
  }
  
  // Now go through the list and delete the elements that were originally marked
  for (unsigned int i=0; i<sorted.size(); ++i)
  {
    Elem * elem = mesh.elem(sorted[i]);
    if (deleteable_elems.find(elem) != deleteable_elems.end())
    {
      removeAllElemBCs(mesh, elem);
      mesh.delete_elem(elem);
    }
    
  }

  mesh.prepare_for_use();
}

void
ElementDeleter::removeAllElemBCs(Mesh & mesh, Elem * elem)
{
  for (unsigned int i=0; i<elem->n_sides(); ++i)
    mesh.boundary_info->remove_side(elem, i);
}
