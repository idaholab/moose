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
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.renameParam("included_subdomains",
                     "primary_block",
                     "The primary set of blocks for which to draw a sideset between");
  params.makeParamRequired<std::vector<SubdomainName>>("primary_block");
  params.renameParam("included_neighbors",
                     "paired_block",
                     "The paired set of blocks for which to draw a sideset between");
  params.makeParamRequired<std::vector<SubdomainName>>("paired_block");
  params.addClassDescription("MeshGenerator that creates a sideset composed of the nodes located "
                             "between two or more subdomains.");

  // TODO: Implement each of these in the generate() routine using utilities in SidesetGeneratorBase
  params.suppressParameter<bool>("fixed_normal");
  params.suppressParameter<bool>("include_only_external_sides");

  return params;
}

SideSetsBetweenSubdomainsGenerator::SideSetsBetweenSubdomainsGenerator(
    const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters)
{
}

std::unique_ptr<MeshBase>
SideSetsBetweenSubdomainsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // construct the FE object so we can compute normals of faces
  setup(*mesh);

  std::vector<boundary_id_type> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, true);

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Prepare to query about sides adjacent to remote elements if we're
  // on a distributed mesh
  const processor_id_type my_n_proc = mesh->n_processors();
  const processor_id_type my_proc_id = mesh->processor_id();
  typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
  std::vector<vec_type> queries(my_n_proc);

  // Request to compute normal vectors
  const std::vector<Point> & face_normals = _fe_face->get_normals();

  for (const auto & elem : mesh->active_element_ptr_range())
  {
    // We only need to loop over elements in the primary subdomain
    if (_check_subdomains && !elementSubdomainIdInList(elem, _included_subdomain_ids))
      continue;

    for (const auto & side : make_range(elem->n_sides()))
    {
      const Elem * neighbor = elem->neighbor_ptr(side);

      // On a replicated mesh, we add all subdomain sides ourselves.
      // On a distributed mesh, we may have missed sides which
      // neighbor remote elements.  We should query any such cases.
      if (neighbor == remote_elem)
      {
        queries[elem->processor_id()].push_back(std::make_pair(elem->id(), side));
      }
      else if (neighbor != NULL)
      {
        _fe_face->reinit(elem, side);
        // We'll just use the normal of the first qp
        const Point & face_normal = face_normals[0];
        // Add the boundaries, if appropriate
        if (elemSideSatisfiesRequirements(elem, side, *mesh, _normal, face_normal))
        {
          // Add the boundaries
          if (_replace)
            boundary_info.remove_side(elem, side);
          for (const auto & boundary_id : boundary_ids)
            boundary_info.add_side(elem, side, boundary_id);
        }
      }
    }
  }

  if (!mesh->is_serial())
  {
    const auto queries_tag = mesh->comm().get_unique_tag(),
               replies_tag = mesh->comm().get_unique_tag();

    std::vector<Parallel::Request> side_requests(my_n_proc - 1), reply_requests(my_n_proc - 1);

    // Make all requests
    for (const auto & p : make_range(my_n_proc))
    {
      if (p == my_proc_id)
        continue;

      Parallel::Request & request = side_requests[p - (p > my_proc_id)];

      mesh->comm().send(p, queries[p], request, queries_tag);
    }

    // Reply to all requests
    std::vector<vec_type> responses(my_n_proc - 1);

    for (const auto & p : make_range(uint(1), my_n_proc))
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

        if (neighbor != NULL)
        {
          _fe_face->reinit(elem, side);
          // We'll just use the normal of the first qp
          const Point & face_normal = _fe_face->get_normals()[0];
          // Add the boundaries, if appropriate
          if (elemSideSatisfiesRequirements(elem, side, *mesh, _normal, face_normal))
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

        if (_replace)
          boundary_info.remove_side(elem, side);
        for (const auto & boundary_id : boundary_ids)
          boundary_info.add_side(elem, side, boundary_id);
      }
    }

    Parallel::wait(side_requests);
    Parallel::wait(reply_requests);
  }

  for (const auto & i : make_range(boundary_ids.size()))
    boundary_info.sideset_name(boundary_ids[i]) = _boundary_names[i];

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
