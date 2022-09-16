//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BreakBoundaryOnSubdomainGenerator.h"
#include "CastUniquePointer.h"

#include "MooseUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", BreakBoundaryOnSubdomainGenerator);

InputParameters
BreakBoundaryOnSubdomainGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Break boundaries based on the subdomains to which their sides are "
                             "attached. Naming convention for the new boundaries will be the old "
                             "boundary name plus \"_to_\" plus the subdomain name");
  params.addParam<std::vector<BoundaryName>>(
      "boundaries", "Boundaries to be broken. Default means to break all boundaries");

  return params;
}

BreakBoundaryOnSubdomainGenerator::BreakBoundaryOnSubdomainGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
BreakBoundaryOnSubdomainGenerator::generate()
{
  // get the mesh and boundary info
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  auto & boundary_info = mesh->get_boundary_info();

  // get IDs of all boundaries to be broken
  std::set<boundary_id_type> breaking_boundary_ids;
  if (isParamValid("boundaries"))
  {
    auto & boundary_names = getParam<std::vector<BoundaryName>>("boundaries");
    for (auto & boundary_name : boundary_names)
    {
      // check that the boundary exists in the mesh
      if (!MooseMeshUtils::hasBoundaryName(*mesh, boundary_name))
        paramError("boundaries", "The boundary '", boundary_name, "' was not found in the mesh");

      breaking_boundary_ids.insert(boundary_info.get_id_by_name(boundary_name));
    }
  }
  else
  {
    breaking_boundary_ids = boundary_info.get_boundary_ids();

    // We might be on a distributed mesh with remote boundary ids
    if (!mesh->is_replicated())
      this->comm().set_union(breaking_boundary_ids);
  }

  // create a list of new boundary names
  std::set<std::string> new_boundary_name_set;
  std::vector<boundary_id_type> side_boundary_ids;
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    auto subdomain_id = elem->subdomain_id();
    auto subdomain_name = mesh->subdomain_name(subdomain_id);
    if (subdomain_name == "")
      subdomain_name = std::to_string(subdomain_id);
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      boundary_info.boundary_ids(elem, side, side_boundary_ids);
      for (auto boundary_id : side_boundary_ids)
        if (breaking_boundary_ids.count(boundary_id) > 0)
          new_boundary_name_set.emplace(boundary_info.sideset_name(boundary_id) + "_to_" +
                                        subdomain_name);
    }
  }

  // We might be on a distributed mesh with remote elements that had
  // new boundary ids added
  if (!mesh->is_replicated())
    this->comm().set_union(new_boundary_name_set);

  // assign boundary IDs to the boundaries to be added
  std::vector<BoundaryName> new_boundary_names(new_boundary_name_set.begin(),
                                               new_boundary_name_set.end());
  auto new_boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, new_boundary_names, true);

  // assign boundary names to the new boundaries
  mooseAssert(new_boundary_ids.size() == new_boundary_names.size(),
              "sizes of boundary names and boundary IDs mismatch");
  for (MooseIndex(new_boundary_ids) i = 0; i < new_boundary_ids.size(); ++i)
  {
    boundary_info.sideset_name(new_boundary_ids[i]) = new_boundary_names[i];
    boundary_info.nodeset_name(new_boundary_ids[i]) = new_boundary_names[i];
  }

  // add sides into the side sets
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    auto subdomain_id = elem->subdomain_id();
    auto subdomain_name = mesh->subdomain_name(subdomain_id);
    if (subdomain_name == "")
      subdomain_name = std::to_string(subdomain_id);
    for (MooseIndex(elem->n_sides()) side = 0; side < elem->n_sides(); ++side)
    {
      std::vector<boundary_id_type> side_boundary_ids;
      boundary_info.boundary_ids(elem, side, side_boundary_ids);
      for (auto boundary_id : side_boundary_ids)
      {
        if (breaking_boundary_ids.count(boundary_id) > 0)
        {
          BoundaryName bname = boundary_info.sideset_name(boundary_id) + "_to_" + subdomain_name;
          auto bid = boundary_info.get_id_by_name(bname);
          boundary_info.add_side(elem, side, bid);
        }
      }
    }
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
