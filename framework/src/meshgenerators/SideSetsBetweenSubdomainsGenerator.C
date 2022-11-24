//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsBetweenSubdomainsGenerator.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/remote_elem.h"

registerMooseObject("MooseApp", SideSetsBetweenSubdomainsGenerator);

InputParameters
SideSetsBetweenSubdomainsGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<SubdomainName>>(
      "primary_block", "The primary set of blocks for which to draw a sideset between");
  params.addDeprecatedParam<std::vector<SubdomainName>>(
      "master_block",
      "The primary set of blocks for which to draw a sideset between",
      "The 'master_block' param is deprecated and will be removed on January 1, 2021. Please use "
      "the 'primary_block' parameter instead.");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "paired_block", "The paired set of blocks for which to draw a sideset between");
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The name of the boundary to create");
  params.addClassDescription("MeshGenerator that creates a sideset composed of the nodes located "
                             "between two or more subdomains.");

  return params;
}

SideSetsBetweenSubdomainsGenerator::SideSetsBetweenSubdomainsGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
SideSetsBetweenSubdomainsGenerator::generate()
{
  auto primary_block = isParamValid("primary_block")
                           ? getParam<std::vector<SubdomainName>>("primary_block")
                           : getParam<std::vector<SubdomainName>>("master_block");

  auto paired_block = getParam<std::vector<SubdomainName>>("paired_block");

  // Check that the block ids/names exist in the mesh
  for (const auto & b : primary_block)
    if (!MooseMeshUtils::hasSubdomainName(*_input, b))
      paramError("primary_block", "The block '", b, "' was not found within the mesh");

  for (const auto & b : paired_block)
    if (!MooseMeshUtils::hasSubdomainName(*_input, b))
      paramError("paired_block", "The block '", b, "' was not found within the mesh");

  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Make sure that the mesh is prepared
  if (!mesh->is_prepared())
    mesh->find_neighbors();

  std::vector<subdomain_id_type> vec_primary_ids =
      MooseMeshUtils::getSubdomainIDs(*mesh, primary_block);
  std::set<subdomain_id_type> primary_ids(vec_primary_ids.begin(), vec_primary_ids.end());

  std::vector<subdomain_id_type> vec_paired_ids =
      MooseMeshUtils::getSubdomainIDs(*mesh, paired_block);
  std::set<subdomain_id_type> paired_ids(vec_paired_ids.begin(), vec_paired_ids.end());

  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");
  std::vector<boundary_id_type> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, boundary_names, true);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Prepare to query about sides adjacent to remote elements if we're
  // on a distributed mesh
  const processor_id_type my_n_proc = mesh->n_processors();
  const processor_id_type my_proc_id = mesh->processor_id();
  typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
  std::vector<vec_type> queries(my_n_proc);

  for (const auto & elem : mesh->active_element_ptr_range())
  {
    subdomain_id_type curr_subdomain = elem->subdomain_id();

    // We only need to loop over elements in the primary subdomain
    if (primary_ids.count(curr_subdomain) == 0)
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); side++)
    {
      const Elem * neighbor = elem->neighbor_ptr(side);

      // On a replicated mesh, we add all subdomain sides ourselves.
      // On a distributed mesh, we may have missed sides which
      // neighbor remote elements.  We should query any such cases.
      if (neighbor == remote_elem)
      {
        queries[elem->processor_id()].push_back(std::make_pair(elem->id(), side));
      }
      else if (neighbor != NULL && paired_ids.count(neighbor->subdomain_id()) > 0)

        // Add the boundaries
        for (const auto & boundary_id : boundary_ids)
          boundary_info.add_side(elem, side, boundary_id);
    }
  }

  if (!mesh->is_serial())
  {
    const auto queries_tag = mesh->comm().get_unique_tag(),
               replies_tag = mesh->comm().get_unique_tag();

    std::vector<Parallel::Request> side_requests(my_n_proc - 1), reply_requests(my_n_proc - 1);

    // Make all requests
    for (processor_id_type p = 0; p != my_n_proc; ++p)
    {
      if (p == my_proc_id)
        continue;

      Parallel::Request & request = side_requests[p - (p > my_proc_id)];

      mesh->comm().send(p, queries[p], request, queries_tag);
    }

    // Reply to all requests
    std::vector<vec_type> responses(my_n_proc - 1);

    for (processor_id_type p = 1; p != my_n_proc; ++p)
    {
      vec_type query;

      Parallel::Status status(mesh->comm().probe(Parallel::any_source, queries_tag));
      const processor_id_type source_pid = cast_int<processor_id_type>(status.source());

      mesh->comm().receive(source_pid, query, queries_tag);

      Parallel::Request & request = reply_requests[p - 1];

      for (const auto & q : query)
      {
        const Elem * elem = mesh->elem_ptr(q.first);
        const unsigned int side = q.second;
        const Elem * neighbor = elem->neighbor_ptr(side);

        if (neighbor != NULL && paired_ids.count(neighbor->subdomain_id()) > 0)
        {
          responses[p - 1].push_back(std::make_pair(elem->id(), side));
        }
      }

      mesh->comm().send(source_pid, responses[p - 1], request, replies_tag);
    }

    // Process all incoming replies
    for (processor_id_type p = 1; p != my_n_proc; ++p)
    {
      Parallel::Status status(this->comm().probe(Parallel::any_source, replies_tag));
      const processor_id_type source_pid = cast_int<processor_id_type>(status.source());

      vec_type response;

      this->comm().receive(source_pid, response, replies_tag);

      for (const auto & r : response)
      {
        const Elem * elem = mesh->elem_ptr(r.first);
        const unsigned int side = r.second;

        for (const auto & boundary_id : boundary_ids)
          boundary_info.add_side(elem, side, boundary_id);
      }
    }

    Parallel::wait(side_requests);
    Parallel::wait(reply_requests);
  }

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.sideset_name(boundary_ids[i]) = boundary_names[i];

  return dynamic_pointer_cast<MeshBase>(mesh);
}
