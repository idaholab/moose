//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsBetweenSubdomains.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseMesh.h"

#include "libmesh/remote_elem.h"

registerMooseObjectReplaced("MooseApp",
                            SideSetsBetweenSubdomains,
                            "11/30/2019 00:00",
                            SideSetsBetweenSubdomainGenerator);

template <>
InputParameters
validParams<SideSetsBetweenSubdomains>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<SubdomainName>>(
      "primary_block", "The primary set of blocks for which to draw a sideset between");
  params.addRequiredParam<std::vector<SubdomainName>>(
      "paired_block", "The paired set of blocks for which to draw a sideset between");
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The name of the boundary to create");
  return params;
}

SideSetsBetweenSubdomains::SideSetsBetweenSubdomains(const InputParameters & parameters)
  : MeshModifier(parameters)
{
}

void
SideSetsBetweenSubdomains::modify()
{
  MeshBase & mesh = _mesh_ptr->getMesh();

  std::vector<SubdomainID> vec_primary_ids =
      _mesh_ptr->getSubdomainIDs(getParam<std::vector<SubdomainName>>("primary_block"));
  std::set<SubdomainID> primary_ids(vec_primary_ids.begin(), vec_primary_ids.end());

  std::vector<SubdomainID> vec_paired_ids =
      _mesh_ptr->getSubdomainIDs(getParam<std::vector<SubdomainName>>("paired_block"));
  std::set<SubdomainID> paired_ids(vec_paired_ids.begin(), vec_paired_ids.end());

  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Prepare to query about sides adjacent to remote elements if we're
  // on a distributed mesh
  const processor_id_type my_n_proc = mesh.n_processors();
  const processor_id_type my_proc_id = mesh.processor_id();
  typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
  std::vector<vec_type> queries(my_n_proc);

  for (const auto & elem : mesh.active_element_ptr_range())
  {
    SubdomainID curr_subdomain = elem->subdomain_id();

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

  if (!mesh.is_serial())
  {
    Parallel::MessageTag queries_tag = mesh.comm().get_unique_tag(867),
                         replies_tag = mesh.comm().get_unique_tag(5309);

    std::vector<Parallel::Request> side_requests(my_n_proc - 1), reply_requests(my_n_proc - 1);

    // Make all requests
    for (processor_id_type p = 0; p != my_n_proc; ++p)
    {
      if (p == my_proc_id)
        continue;

      Parallel::Request & request = side_requests[p - (p > my_proc_id)];

      mesh.comm().send(p, queries[p], request, queries_tag);
    }

    // Reply to all requests
    std::vector<vec_type> responses(my_n_proc - 1);

    for (processor_id_type p = 1; p != my_n_proc; ++p)
    {
      vec_type query;

      Parallel::Status status(mesh.comm().probe(Parallel::any_source, queries_tag));
      const processor_id_type source_pid = cast_int<processor_id_type>(status.source());

      mesh.comm().receive(source_pid, query, queries_tag);

      Parallel::Request & request = reply_requests[p - 1];

      for (const auto & q : query)
      {
        const Elem * elem = mesh.elem_ptr(q.first);
        const unsigned int side = q.second;
        const Elem * neighbor = elem->neighbor_ptr(side);

        if (neighbor != NULL && paired_ids.count(neighbor->subdomain_id()) > 0)
        {
          responses[p - 1].push_back(std::make_pair(elem->id(), side));
        }
      }

      mesh.comm().send(source_pid, responses[p - 1], request, replies_tag);
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
        const Elem * elem = mesh.elem_ptr(r.first);
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
}
