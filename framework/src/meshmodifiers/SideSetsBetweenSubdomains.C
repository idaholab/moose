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

template<>
InputParameters validParams<SideSetsBetweenSubdomains>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "A list of boudnary names to associate with the subdomain");
  params.addRequiredParam<SubdomainName>("master_block", "The first block for which to draw a sideset between");
  params.addRequiredParam<SubdomainName>("paired_block", "The second block for which to draw a sideset between");
  return params;
}

SideSetsBetweenSubdomains::SideSetsBetweenSubdomains(const std::string & name, InputParameters parameters):
    MeshModifier(name, parameters)
{
}

SideSetsBetweenSubdomains::~SideSetsBetweenSubdomains()
{
}

void
SideSetsBetweenSubdomains::modify()
{
  MeshBase & mesh = _mesh_ptr->_mesh;

  SubdomainID master_id = _mesh_ptr->getSubdomainID(getParam<SubdomainName>("master_block"));
  SubdomainID paired_id = _mesh_ptr->getSubdomainID(getParam<SubdomainName>("paired_block"));
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName> >("boundary");
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);

  MeshBase::const_element_iterator	 el	= mesh.active_elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.active_elements_end();
  for (; el != end_el ; ++el)
  {
    const Elem* elem = *el;
    SubdomainID curr_subdomain = elem->subdomain_id();

    // We only need to loop over elements in the master subdomain
    if (curr_subdomain != master_id)
      continue;

    for (unsigned int side=0; side<elem->n_sides(); side++)
    {
      const Elem * neighbor = elem->neighbor(side);
      if (neighbor != NULL && neighbor->subdomain_id() == paired_id)  // is this side between the two blocks?

        // Add the boundaries
        for (unsigned int i=0; i<boundary_ids.size(); ++i)
          mesh.boundary_info->add_side(elem, side, boundary_ids[i]);
    }
  }

  for (unsigned int i=0; i<boundary_ids.size(); ++i)
    mesh.boundary_info->sideset_name(boundary_ids[i]) = boundary_names[i];
}
