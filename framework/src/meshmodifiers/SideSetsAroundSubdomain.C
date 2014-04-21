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

#include "SideSetsAroundSubdomain.h"
#include "InputParameters.h"
#include "MooseTypes.h"

// libMesh includes
#include "libmesh/mesh.h"

template<>
InputParameters validParams<SideSetsAroundSubdomain>()
{
  InputParameters params = validParams<MeshModifier>();
  params += validParams<BlockRestrictable>();
  params.addParam<std::vector<BoundaryName> >("boundary", "The list of boundary IDs from the mesh where this object is to be applied");
  return params;
}

SideSetsAroundSubdomain::SideSetsAroundSubdomain(const std::string & name, InputParameters parameters):
    MeshModifier(name, parameters),
    BlockRestrictable(name, parameters),
    _boundary_names(getParam<std::vector<BoundaryName> >("boundary"))
{
}

SideSetsAroundSubdomain::~SideSetsAroundSubdomain()
{
}

void
SideSetsAroundSubdomain::modify()
{
  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  // Extract the 'first' block ID
  SubdomainID block_id = *blockIDs().begin();

  // Extract the SubdomainID
  if (numBlocks() > 1)
     mooseWarning("SideSetsAroundSubdomain only acts on a single subdomain, but multiple were provided: only the " << block_id << "' subdomain is being used.");

  // Create the boundary IDs from the list of names provided (the true flag creates ids from unknown names)
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(_boundary_names, true);

  // Loop over the elements
  MeshBase::const_element_iterator   el  = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_elements_end();
  for (; el != end_el ; ++el)
  {
    const Elem* elem = *el;
    SubdomainID curr_subdomain = elem->subdomain_id();

    // We only need to loop over elements in the source subdomain
    if (curr_subdomain != block_id)
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      const Elem * neighbor = elem->neighbor(side);
      if (neighbor == NULL ||                   // element on boundary OR
          neighbor->subdomain_id() != block_id) // neighboring element is on a different subdomain

        // Add the boundaries
        for (unsigned int i = 0; i < boundary_ids.size(); ++i)
          mesh.boundary_info->add_side(elem, side, boundary_ids[i]);
    }
  }

  // Assign the supplied names to the newly created side sets
  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    mesh.boundary_info->sideset_name(boundary_ids[i]) = _boundary_names[i];
}
