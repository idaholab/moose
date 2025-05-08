//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CopyMeshPartitioner.h"

#include "MooseApp.h"
#include "MooseMesh.h"
#include "MooseRandom.h"

#include "libmesh/elem.h"

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"

registerMooseObject("MooseApp", CopyMeshPartitioner);

InputParameters
CopyMeshPartitioner::validParams()
{
  InputParameters params = MoosePartitioner::validParams();
  params.addClassDescription("Assigns element to match the partitioning of another mesh. If in a "
                             "child application, defaults to the parent app mesh if the other mesh "
                             "is not specified programmatically.");
  params.addPrivateParam<MooseMesh *>("source_mesh");

  return params;
}

CopyMeshPartitioner::CopyMeshPartitioner(const InputParameters & params) : MoosePartitioner(params)
{
  if (isParamValid("source_mesh"))
    _base_mesh = getParam<MooseMesh *>("source_mesh");
  else if (!_app.isUltimateMaster() && _app.masterMesh())
    _base_mesh = _app.masterMesh();
  else
    mooseError("Expecting either a parent app with a mesh to copy the partitioning from, a "
               "'source_mesh' (private) parameter to be set programmatically.");
}

std::unique_ptr<Partitioner>
CopyMeshPartitioner::clone() const
{
  return _app.getFactory().clone(*this);
}

void
CopyMeshPartitioner::_do_partition(MeshBase & mesh, const unsigned int /*n*/)
{
  mooseAssert(_base_mesh, "Should have a base mesh to copy the partitioning from");

  // First check that it makes sense to copy the partitioning
  // If we use the same number of procs to partition this mesh than to partition the source mesh, we
  // are effectively matching regions to processes the same way in both meshes. Makes sense.
  // If we use more procs, this will leave some processes with no elements. It is not ideal, let's
  // just give a warning. This does not happen in practice with MultiApps.
  // If we use less procs (N_source > N_current), we error. If we could avoid erroring, then either:
  // - the elements on our mesh always get matched to elements from only N_current processes among
  //   the N_source processes. We can accomodate that
  // - the elements on our mesh get matched to more processes than we have. We would
  //   need a heuristic (for example a nested partitioner) to re-distribute these elements. We won't
  //   support it
  std::set<unsigned int> pids_used;

  // We cannot get the point locator only on some ranks of the base mesh, so we must error here
  if (_base_mesh->comm().size() > mesh.comm().size())
    mooseError("This partitioner does not support using less ranks to partition the mesh than are "
               "used to partition the source mesh (the mesh we are copying the partitioning from)");

  // Get point locator
  auto pl = _base_mesh->getPointLocator();
  pl->enable_out_of_mesh_mode();

  // We use the pull_parallel_vector_data algorithm.
  // This code is very similar to partitioner::assign_partitioning in libMesh partitioner.C
  // We need to define three things:
  // - the list of requests: the elements in our local mesh we are trying to assign pids to
  // - how to gather results: not all processors know which element belongs where, they will each
  //   process the requests sent to them and send back the elem pids if they know them.
  // - how to fill the requests: from the data gathered and sent back, we need to decide which data
  //   is good and save it into the output map

  // For each processor id, all the elements in our mesh that will request to know their new
  // processor id. If the target mesh is replicated, that's all the elements from that mesh, but the
  // source mesh may be distributed so we still need the communication. If a distributed mesh, then
  // that's only the local and ghosted elements.
  // We send the target element vertex average, enough to find it in the source mesh.
  // We send the target element pid, to know where to save the result in the filled requests
  // We send an index for the ordering of the requests to facilitate retrieving the results
  // NOTE: in partitioner.C they manage to unpack the filled requests in the same order that
  //       the requests are sent, thus saving the need for that last index. Possible rework..
  std::map<processor_id_type,
           std::vector<std::tuple<Real, Real, Real, processor_id_type, unsigned int>>>
      requested_elements;
  const bool distributed_mesh = dynamic_cast<const DistributedMesh *>(&_base_mesh->getMesh());
  unsigned int request_i = 0;
  for (auto & elem : mesh.active_element_ptr_range())
  {
    // we don't know which processor owns this point.
    // As a first try, we add this to every processor id
    // TODO: a simple bounding box heuristic would do fine!
    const auto elem_pt = elem->vertex_average();
    if (distributed_mesh)
      for (const auto pid : make_range(mesh.comm().size()))
        requested_elements[pid].push_back(
            {elem_pt(0), elem_pt(1), elem_pt(2), elem->processor_id(), request_i});
    // source mesh is replicated, let's just ask the sending processor what the processor id is
    else
      requested_elements[processor_id()].push_back(
          {elem_pt(0), elem_pt(1), elem_pt(2), processor_id(), request_i});
    request_i++;
  }

  // For each requests, find which process is able to fill the request
  // Every processor fills these requests as best it can.
  auto gather_functor =
      [&pl](processor_id_type /*pid*/,
            const std::vector<std::tuple<Real, Real, Real, processor_id_type, unsigned int>> &
                incoming_elements,
            std::vector<processor_id_type> & outgoing_pids)
  {
    outgoing_pids.resize(incoming_elements.size(), libMesh::invalid_uint);

    // Try the pl on the incoming element
    for (const auto i : index_range(incoming_elements))
    {
      const auto & elem = (*pl)(Point(std::get<0>(incoming_elements[i]),
                                      std::get<1>(incoming_elements[i]),
                                      std::get<2>(incoming_elements[i])));
      if (elem)
        outgoing_pids[i] = elem->processor_id();
    }
  };

  // Results to gather from each processor
  // For each processor id, we have a vector indexed by element with the new processor id
  // Note that the filled_requests should match the ordering of the requested_elements
  std::map<processor_id_type, std::vector<processor_id_type>> filled_request;
  for (const auto i : make_range(mesh.comm().size()))
    filled_request[i].resize(requested_elements.count(i) ? requested_elements[i].size() : 0);

  // How to act on the exchanged data, and fill the filled_request (output we sought)
  auto action_functor =
      [&filled_request](
          processor_id_type,
          const std::vector<std::tuple<Real, Real, Real, processor_id_type, unsigned int>> & elems,
          const std::vector<processor_id_type> & new_procids)
  {
    for (const auto i : index_range(new_procids))
      if (new_procids[i] != libMesh::invalid_uint)
      {
        const auto elem_pid = std::get<3>(elems[i]);
        const auto request_i = std::get<4>(elems[i]);
        filled_request[elem_pid][request_i] = new_procids[i];
      }
  };

  // Trade requests with other processors
  // NOTE: We could try to use base mesh communicator because that's where we gather the information
  // However, if that mesh communicator has more processes than we do, that would be trouble.
  const processor_id_type * ex = nullptr;
  Parallel::pull_parallel_vector_data(
      mesh.comm(), requested_elements, gather_functor, action_functor, ex);

  // Assign the partitioning.
  request_i = 0;
  for (auto & elem : mesh.active_element_ptr_range())
  {
    const processor_id_type current_pid = elem->processor_id();
    const auto lookup_pid = distributed_mesh ? current_pid : processor_id();
    const processor_id_type elem_procid = filled_request[lookup_pid][request_i++];

    elem->processor_id() = elem_procid;
    // Keep track of processor ids used in case we need to do a pass at re-assigning
    pids_used.insert(elem_procid);
  }

  // Synchronize the pids used across all ranks
  // NOTE: we could have gathered this earlier
  std::vector<unsigned int> pids_used_vec(pids_used.begin(), pids_used.end());
  mesh.comm().allgather(pids_used_vec);
  pids_used.insert(pids_used_vec.begin(), pids_used_vec.end());

  // Check the pids used
  // We cannot use more process ids to partition the mesh than the current app is using
  const auto max_pid = mesh.comm().size();
  if (pids_used.size() > max_pid)
    mooseError("Partitioning copy used more regions (" + std::to_string(pids_used.size()) +
               ") than the number of parallel processes (" + std::to_string(mesh.comm().size()) +
               ")");
  if (pids_used.size() < max_pid)
    mooseWarning(
        "Some parallel (MPI) processes were not assigned any element during partitioning. These "
        "processes will not perform any work.");

  // The pids are not too many, but their numbering is not contiguous, renumber the process id
  // assignment
  if (*pids_used.rbegin() > max_pid)
  {
    std::unordered_map<unsigned int, unsigned int> source_to_current_pid_map;

    // TODO: This is a naive remapping. There might be remapping that have optimal locality. E.g.
    // the mpi ranks are on the same node, limiting communications. Once we can have multiple
    // partitioners, we should look into having a nested partitioner handle the outliers
    unsigned int i = 0;
    for (const auto pid_set : pids_used)
      source_to_current_pid_map[pid_set] = i++;

    for (auto & elem_ptr : mesh.active_element_ptr_range())
      elem_ptr->processor_id() = source_to_current_pid_map[elem_ptr->processor_id()];
  }
}
