//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementDeletionGeneratorBase.h"
#include "libmesh/remote_elem.h"

#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

InputParameters
ElementDeletionGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<bool>("delete_exteriors",
                        true,
                        "Whether to delete lower-d elements whose interior parents are deleted");
  params.addParam<BoundaryName>("new_boundary",
                                "optional boundary name to assign to the cut surface");

  return params;
}

ElementDeletionGeneratorBase::ElementDeletionGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _assign_boundary(isParamValid("new_boundary")),
    _delete_exteriors(getParam<bool>("delete_exteriors")),
    _boundary_name(_assign_boundary ? getParam<BoundaryName>("new_boundary") : "")
{
}

std::unique_ptr<MeshBase>
ElementDeletionGeneratorBase::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Make sure that the mesh is prepared
  if (!mesh->is_prepared())
    mesh->prepare_for_use();

  // Elements that the deleter will remove
  std::set<Elem *> deleteable_elems;

  // First let's figure out which elements need to be deleted
  for (auto & elem : mesh->element_ptr_range())
  {
    if (shouldDelete(elem))
      deleteable_elems.insert(elem);
  }

  // check for dangling interior parents
  for (auto & elem : mesh->element_ptr_range())
    if (elem->dim() < mesh->mesh_dimension() && deleteable_elems.count(elem->interior_parent()) > 0)
    {
      if (_delete_exteriors)
        deleteable_elems.insert(elem);
      else
        elem->set_interior_parent(nullptr);
    }

    /**
     * If we are in parallel we'd better have a consistent idea of what
     * should be deleted.  This can't be checked cheaply.
     */
#ifdef DEBUG
  dof_id_type pmax_elem_id = mesh->max_elem_id();
  mesh->comm().max(pmax_elem_id);

  for (dof_id_type i = 0; i != pmax_elem_id; ++i)
  {
    Elem * elem = mesh->query_elem_ptr(i);
    bool is_deleteable = elem && deleteable_elems.count(elem);

    libmesh_assert(mesh->comm().semiverify(elem ? &is_deleteable : libmesh_nullptr));
  }
#endif

  // Get the BoundaryID from the mesh
  boundary_id_type boundary_id = 0;
  if (_assign_boundary)
    boundary_id = MooseMeshUtils::getBoundaryIDs(*mesh, {_boundary_name}, true)[0];

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  /**
   * Delete all of the elements
   *
   * TODO: We need to sort these not because they have to be deleted in a certain order in libMesh,
   *       but because the order of deletion might impact what happens to any existing sidesets or
   * nodesets.
   */
  for (auto & elem : deleteable_elems)
  {
    // On distributed meshes, we'll need neighbor links to be useable
    // shortly, so we can't just leave dangling pointers.
    //
    // FIXME - this could be made AMR-aware and refactored into
    // libMesh - roystgnr
    unsigned int n_sides = elem->n_sides();
    for (unsigned int n = 0; n != n_sides; ++n)
    {
      Elem * neighbor = elem->neighbor_ptr(n);
      if (!neighbor || neighbor == remote_elem)
        continue;

      const unsigned int return_side = neighbor->which_neighbor_am_i(elem);

      if (neighbor->neighbor_ptr(return_side) == elem)
      {
        neighbor->set_neighbor(return_side, nullptr);

        // assign cut surface boundary
        if (_assign_boundary)
          boundary_info.add_side(neighbor, return_side, boundary_id);
      }
    }

    mesh->delete_elem(elem);
  }

  /**
   * If we are on a distributed mesh, we may have deleted elements
   * which are remote_elem links on other processors, and we need
   * to make neighbor and interior parent links into NULL pointers
   * (i.e. domain boundaries in the former case) instead.
   */
  if (!mesh->is_serial())
  {
    const processor_id_type my_n_proc = mesh->n_processors();
    const processor_id_type my_proc_id = mesh->processor_id();
    typedef std::vector<std::pair<dof_id_type, unsigned int>> vec_type;
    std::vector<vec_type> queries(my_n_proc);

    // Loop over the elements looking for those with remote neighbors
    // or interior parents.
    // The ghost_elements iterators in libMesh need to be updated
    // before we can use them safely here, so we'll test for
    // ghost-vs-local manually.
    for (const auto & elem : mesh->element_ptr_range())
    {
      const processor_id_type pid = elem->processor_id();
      if (pid == my_proc_id)
        continue;

      const unsigned int n_sides = elem->n_sides();
      for (unsigned int n = 0; n != n_sides; ++n)
        if (elem->neighbor_ptr(n) == remote_elem)
          queries[pid].push_back(std::make_pair(elem->id(), n));

      // Use an OOB side index to encode "interior_parent". We will use this OOB index later
      if (elem->interior_parent() == remote_elem)
        queries[pid].push_back(std::make_pair(elem->id(), n_sides));
    }

    const auto queries_tag = mesh->comm().get_unique_tag(),
               replies_tag = mesh->comm().get_unique_tag();

    std::vector<Parallel::Request> query_requests(my_n_proc - 1), reply_requests(my_n_proc - 1);

    // Make all requests
    for (processor_id_type p = 0; p != my_n_proc; ++p)
    {
      if (p == my_proc_id)
        continue;

      Parallel::Request & request = query_requests[p - (p > my_proc_id)];

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
        const Elem * target =
            (side >= elem->n_sides()) ? elem->interior_parent() : elem->neighbor_ptr(side);

        if (target == nullptr) // linked element was deleted!
          responses[p - 1].push_back(std::make_pair(elem->id(), side));
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
        Elem * elem = mesh->elem_ptr(r.first);
        const unsigned int side = r.second;

        if (side < elem->n_sides())
        {
          mooseAssert(elem->neighbor_ptr(side) == remote_elem, "element neighbor != remote_elem");

          elem->set_neighbor(side, nullptr);

          // assign cut surface boundary
          if (_assign_boundary)
            boundary_info.add_side(elem, side, boundary_id);
        }
        else
        {
          mooseAssert(side == elem->n_sides(), "internal communication error");
          mooseAssert(elem->interior_parent() == remote_elem, "interior parent != remote_elem");

          elem->set_interior_parent(nullptr);
        }
      }
    }

    Parallel::wait(query_requests);
    Parallel::wait(reply_requests);
  }

  if (_assign_boundary)
  {
    boundary_info.sideset_name(boundary_id) = _boundary_name;
    boundary_info.nodeset_name(boundary_id) = _boundary_name;
  }

  /**
   * If we are on a ReplicatedMesh, deleting nodes and elements leaves
   * NULLs in the mesh datastructure. We ought to get rid of those.
   * For now, we'll call contract and notify the SetupMeshComplete
   * Action that we need to re-prepare the mesh.
   */
  mesh->contract();
  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
