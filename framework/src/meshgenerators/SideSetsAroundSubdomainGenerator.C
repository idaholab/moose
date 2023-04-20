//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsAroundSubdomainGenerator.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh.h"
#include "libmesh/remote_elem.h"
#include "libmesh/point.h"
#include "libmesh/fe_base.h"

registerMooseObject("MooseApp", SideSetsAroundSubdomainGenerator);

InputParameters
SideSetsAroundSubdomainGenerator::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "new_boundary", "The list of boundary IDs to create on the supplied subdomain");
  params.addRequiredParam<std::vector<SubdomainName>>("block",
                                                      "The blocks around which to create sidesets");
  params.addParam<Point>("normal",
                         "If supplied, only faces with normal equal to this, up to "
                         "normal_tol, will be added to the sidesets specified");
  params.addParam<bool>("external_only", false, "Only apply the sideset to external boundaries");
  params.addRangeCheckedParam<Real>("normal_tol",
                                    0.1,
                                    "normal_tol>=0 & normal_tol<=2",
                                    "If normal is supplied then faces are "
                                    "only added if face_normal.normal_hat >= "
                                    "1 - normal_tol, where normal_hat = "
                                    "normal/|normal|");

  params.addClassDescription(
      "Adds element faces that are on the exterior of the given block to the sidesets specified");

  return params;
}

SideSetsAroundSubdomainGenerator::SideSetsAroundSubdomainGenerator(
    const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    _input(getMesh("input")),
    _boundary_names(getParam<std::vector<BoundaryName>>("new_boundary")),
    _using_normal(isParamValid("normal")),
    _external_only(getParam<bool>("external_only")),
    _normal_tol(getParam<Real>("normal_tol")),
    _normal(_using_normal ? getParam<Point>("normal") : Point())
{
  if (_using_normal)
  {
    // normalize
    mooseAssert(_normal.norm() >= 1E-5, "Normal is zero");
    _normal /= _normal.norm();
  }
}

std::unique_ptr<MeshBase>
SideSetsAroundSubdomainGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  std::vector<SubdomainName> block_names = getParam<std::vector<SubdomainName>>("block");
  // check that the blocks exist in the mesh
  for (const auto & name : block_names)
    if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
      paramError("block", "The block '", name, "' was not found in the mesh");

  auto blocks = MooseMeshUtils::getSubdomainIDs(*mesh, block_names);
  std::set<subdomain_id_type> block_ids(blocks.begin(), blocks.end());

  // Create the boundary IDs from the list of names provided (the true flag creates ids from unknown
  // names)
  std::vector<boundary_id_type> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, true);

  // construct the FE object so we can compute normals of faces
  setup(*mesh);
  Point face_normal;
  bool add_to_bdy = true;

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Prepare to query about sides adjacent to remote elements if we're
  // on a distributed mesh
  const processor_id_type my_n_proc = mesh->n_processors();
  const processor_id_type my_proc_id = mesh->processor_id();
  typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
  std::vector<vec_type> queries(my_n_proc);

  // Loop over the elements
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    subdomain_id_type curr_subdomain = elem->subdomain_id();

    // We only need to loop over elements in the source subdomain
    if (block_ids.count(curr_subdomain) == 0)
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      const Elem * neighbor = elem->neighbor_ptr(side);

      // On a replicated mesh, we add all subdomain sides ourselves.
      // On a distributed mesh, we may have missed sides which
      // neighbor remote elements.  We should query any such cases.
      if (neighbor == remote_elem)
      {
        queries[elem->processor_id()].push_back(std::make_pair(elem->id(), side));
      }
      else if (neighbor == nullptr || // element on boundary OR
               (!_external_only && block_ids.count(neighbor->subdomain_id()) ==
                                       0)) // neighboring element is on a different subdomain
      {
        if (_using_normal)
        {
          const std::vector<Point> & normals = _fe_face->get_normals();
          _fe_face->reinit(elem, side);
          face_normal = normals[0];
          add_to_bdy = (_normal * face_normal >= 1.0 - _normal_tol);
        }

        // Add the boundaries, if appropriate
        if (add_to_bdy)
          for (const auto & boundary_id : boundary_ids)
            boundary_info.add_side(elem, side, boundary_id);
      }
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

        if (neighbor == nullptr || // element on boundary OR
            block_ids.count(neighbor->subdomain_id()) ==
                0) // neighboring element is on a different subdomain
        {
          if (_using_normal)
          {
            const std::vector<Point> & normals = _fe_face->get_normals();
            _fe_face->reinit(elem, side);
            face_normal = normals[0];
            add_to_bdy = (_normal * face_normal >= 1.0 - _normal_tol);
          }

          // Add the boundaries, if appropriate
          if (add_to_bdy)
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

  // Assign the supplied names to the newly created side sets
  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.sideset_name(boundary_ids[i]) = _boundary_names[i];

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
