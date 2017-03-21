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

#include "BreakBoundaryOnSubdomain.h"

#include "MooseMesh.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<BreakBoundaryOnSubdomain>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addClassDescription("Break boundaries based on the subdomains to which their sides are "
                             "attached. Naming convention for the new boundaries will be the old "
                             "boundary name plus \"_to_\" plus the subdomain name");
  params.addParam<std::vector<BoundaryName>>(
      "boundaries", "Boundaries to be broken. Default means to break all boundaries");
  return params;
}

BreakBoundaryOnSubdomain::BreakBoundaryOnSubdomain(const InputParameters & parameters)
  : MeshModifier(parameters)
{
}

void
BreakBoundaryOnSubdomain::modify()
{
  // this modifier is not designed for working with distributed mesh
  _mesh_ptr->errorIfDistributedMesh("BreakBoundaryOnSubdomain");

  // get the mesh and boundary info
  auto & mesh = _mesh_ptr->getMesh();
  auto & boundary_info = mesh.get_boundary_info();

  // get IDs of all boundaries to be broken
  std::set<BoundaryID> breaking_boundary_ids;
  if (isParamValid("boundaries"))
  {
    auto & boundary_names = getParam<std::vector<BoundaryName>>("boundaries");
    for (auto i = beginIndex(boundary_names); i < boundary_names.size(); ++i)
      breaking_boundary_ids.insert(_mesh_ptr->getBoundaryID(boundary_names[i]));
  }
  else
    breaking_boundary_ids = boundary_info.get_boundary_ids();

  auto end_el = mesh.active_elements_end();

  // create a list of new boundary names
  std::set<BoundaryName> new_boundary_name_set;
  for (auto el = mesh.active_elements_begin(); el != end_el; ++el)
  {
    auto elem = *el;
    auto subdomain_id = elem->subdomain_id();
    auto subdomain_name = mesh.subdomain_name(subdomain_id);
    if (subdomain_name == "")
      subdomain_name = std::to_string(subdomain_id);
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      auto side_boundary_ids = boundary_info.boundary_ids(elem, side);
      for (auto i = beginIndex(side_boundary_ids); i < side_boundary_ids.size(); ++i)
        if (breaking_boundary_ids.count(side_boundary_ids[i]) > 0)
          new_boundary_name_set.emplace(boundary_info.sideset_name(side_boundary_ids[i]) + "_to_" +
                                        subdomain_name);
    }
  }

  // assign boundary IDs to the boundaries to be added
  std::vector<BoundaryName> new_boundary_names(new_boundary_name_set.begin(),
                                               new_boundary_name_set.end());
  auto new_boundary_ids = _mesh_ptr->getBoundaryIDs(new_boundary_names, true);

  // assign boundary names to the new boundaries
  mooseAssert(new_boundary_ids.size() == new_boundary_names.size(),
              "sizes of boundary names and boundary IDs mismatch");
  for (auto i = beginIndex(new_boundary_ids); i < new_boundary_ids.size(); ++i)
  {
    boundary_info.sideset_name(new_boundary_ids[i]) = new_boundary_names[i];
    boundary_info.nodeset_name(new_boundary_ids[i]) = new_boundary_names[i];
  }

  // add sides into the side sets
  for (auto el = mesh.active_elements_begin(); el != end_el; ++el)
  {
    auto elem = *el;
    auto subdomain_id = elem->subdomain_id();
    auto subdomain_name = mesh.subdomain_name(subdomain_id);
    if (subdomain_name == "")
      subdomain_name = std::to_string(subdomain_id);
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      auto side_boundary_ids = boundary_info.boundary_ids(elem, side);
      for (auto i = beginIndex(side_boundary_ids); i < side_boundary_ids.size(); ++i)
      {
        if (breaking_boundary_ids.count(side_boundary_ids[i]) > 0)
        {
          BoundaryName bname =
              boundary_info.sideset_name(side_boundary_ids[i]) + "_to_" + subdomain_name;
          auto bid = boundary_info.get_id_by_name(bname);
          boundary_info.add_side(elem, side, bid);
        }
      }
    }
  }
}
