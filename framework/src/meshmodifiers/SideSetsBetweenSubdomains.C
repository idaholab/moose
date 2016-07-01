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

#include "SideSetsBetweenSubdomains.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<SideSetsBetweenSubdomains>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<SubdomainName> >("master_block", "The master set of blocks for which to draw a sideset between");
  params.addRequiredParam<std::vector<SubdomainName> >("paired_block", "The paired set of blocks for which to draw a sideset between");
  params.addRequiredParam<std::vector<BoundaryName> >("new_boundary", "The name of the boundary to create");
  return params;
}

SideSetsBetweenSubdomains::SideSetsBetweenSubdomains(const InputParameters & parameters) :
    MeshModifier(parameters)
{
}

SideSetsBetweenSubdomains::~SideSetsBetweenSubdomains()
{
}

void
SideSetsBetweenSubdomains::modify()
{
  MeshBase & mesh = _mesh_ptr->getMesh();

  std::vector<SubdomainID> vec_master_ids = _mesh_ptr->getSubdomainIDs(getParam<std::vector<SubdomainName> >("master_block"));
  std::set<SubdomainID> master_ids(vec_master_ids.begin(), vec_master_ids.end());

  std::vector<SubdomainID> vec_paired_ids = _mesh_ptr->getSubdomainIDs(getParam<std::vector<SubdomainName> >("paired_block"));
  std::set<SubdomainID> paired_ids(vec_paired_ids.begin(), vec_paired_ids.end());

  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName> >("new_boundary");
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  MeshBase::const_element_iterator   el  = mesh.active_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_elements_end();

  for (; el != end_el ; ++el)
  {
    const Elem* elem = *el;
    SubdomainID curr_subdomain = elem->subdomain_id();

    // We only need to loop over elements in the master subdomain
    if (master_ids.count(curr_subdomain) == 0)
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      const Elem * neighbor = elem->neighbor(side);
      if (neighbor != NULL && paired_ids.count(neighbor->subdomain_id()) > 0)

        // Add the boundaries
        for (const auto & boundary_id : boundary_ids)
          boundary_info.add_side(elem, side, boundary_id);
    }
  }

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.sideset_name(boundary_ids[i]) = boundary_names[i];
}
