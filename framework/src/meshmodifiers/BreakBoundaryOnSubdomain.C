//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  // get the mesh and boundary info
  auto & mesh = _mesh_ptr->getMesh();
  auto & boundary_info = mesh.get_boundary_info();

  // get IDs of all boundaries to be broken
  std::set<BoundaryID> breaking_boundary_ids;
  if (isParamValid("boundaries"))
  {
    auto & boundary_names = getParam<std::vector<BoundaryName>>("boundaries");
    for (auto & boundary_name : boundary_names)
      breaking_boundary_ids.insert(_mesh_ptr->getBoundaryID(boundary_name));
  }
  else
  {
    breaking_boundary_ids = boundary_info.get_boundary_ids();

    // We might be on a distributed mesh with remote boundary ids
    if (!mesh.is_replicated())
      this->comm().set_union(breaking_boundary_ids);
  }

  // create a list of new boundary names
  std::set<std::string> new_boundary_name_set;
  for (const auto & elem : mesh.active_element_ptr_range())
  {
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

  // We might be on a distributed mesh with remote elements that had
  // new boundary ids added
  if (!mesh.is_replicated())
    this->comm().set_union(new_boundary_name_set);

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
  for (const auto & elem : mesh.active_element_ptr_range())
  {
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
