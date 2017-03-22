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
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh.h"
#include "libmesh/remote_elem.h"

template <>
InputParameters
validParams<SideSetsAroundSubdomain>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  params += validParams<BlockRestrictable>();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "new_boundary", "The list of boundary IDs to create on the supplied subdomain");
  params.addParam<Point>("normal",
                         "If supplied, only faces with normal equal to this, up to "
                         "normal_tol, will be added to the sidesets specified");
  params.addRangeCheckedParam<Real>("normal_tol",
                                    0.1,
                                    "normal_tol>=0 & normal_tol<=2",
                                    "If normal is supplied then faces are "
                                    "only added if face_normal.normal_hat >= "
                                    "1 - normal_tol, where normal_hat = "
                                    "normal/|normal|");

  // We can't perform block/boundary restrictable checks on construction for MeshModifiers
  params.set<bool>("delay_initialization") = true;

  params.addClassDescription(
      "Adds element faces that are on the exterior of the given block to the sidesets specified");
  return params;
}

SideSetsAroundSubdomain::SideSetsAroundSubdomain(const InputParameters & parameters)
  : AddSideSetsBase(parameters),
    BlockRestrictable(parameters),
    _boundary_names(getParam<std::vector<BoundaryName>>("new_boundary")),
    _using_normal(isParamValid("normal")),
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

void
SideSetsAroundSubdomain::initialize()
{
  // Initialize the BlockRestrictable parent
  initializeBlockRestrictable(_pars);
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
    mooseWarning("SideSetsAroundSubdomain only acts on a single subdomain, but multiple were "
                 "provided: only the ",
                 block_id,
                 "' subdomain is being used.");

  // Create the boundary IDs from the list of names provided (the true flag creates ids from unknown
  // names)
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(_boundary_names, true);

  // construct the FE object so we can compute normals of faces
  setup();
  Point face_normal;
  bool add_to_bdy = true;

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // Prepare to query about sides adjacent to remote elements if we're
  // on a distributed mesh
  const processor_id_type my_n_proc = mesh.n_processors();
  const processor_id_type my_proc_id = mesh.processor_id();
  typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
  std::vector<vec_type> queries(my_n_proc);

  // Loop over the elements
  for (MeshBase::const_element_iterator el = mesh.active_elements_begin(),
                                        end_el = mesh.active_elements_end();
       el != end_el;
       ++el)
  {
    const Elem * elem = *el;
    SubdomainID curr_subdomain = elem->subdomain_id();

    // We only need to loop over elements in the source subdomain
    if (curr_subdomain != block_id)
      continue;

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      const Elem * neighbor = elem->neighbor(side);

      // On a replicated mesh, we add all subdomain sides ourselves.
      // On a distributed mesh, we may have missed sides which
      // neighbor remote elements.  We should query any such cases.
      if (neighbor == remote_elem)
      {
        queries[elem->processor_id()].push_back(std::make_pair(elem->id(), side));
      }
      else if (neighbor == NULL || // element on boundary OR
               neighbor->subdomain_id() !=
                   block_id) // neighboring element is on a different subdomain
      {
        if (_using_normal)
        {
          _fe_face->reinit(elem, side);
          face_normal = _fe_face->get_normals()[0];
          add_to_bdy = (_normal * face_normal >= 1.0 - _normal_tol);
        }

        // Add the boundaries, if appropriate
        if (add_to_bdy)
          for (const auto & boundary_id : boundary_ids)
            boundary_info.add_side(elem, side, boundary_id);
      }
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
        const Elem * neighbor = elem->neighbor(side);

        if (neighbor == NULL ||                   // element on boundary OR
            neighbor->subdomain_id() != block_id) // neighboring element is on a different subdomain
        {
          if (_using_normal)
          {
            _fe_face->reinit(elem, side);
            face_normal = _fe_face->get_normals()[0];
            add_to_bdy = (_normal * face_normal >= 1.0 - _normal_tol);
          }

          // Add the boundaries, if appropriate
          if (add_to_bdy)
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

  finalize();

  // Assign the supplied names to the newly created side sets
  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.sideset_name(boundary_ids[i]) = _boundary_names[i];
}
