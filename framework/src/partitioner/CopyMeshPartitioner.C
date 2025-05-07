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

registerMooseObject("MooseApp", CopyMeshPartitioner);

InputParameters
CopyMeshPartitioner::validParams()
{
  InputParameters params = MoosePartitioner::validParams();
  params.addClassDescription("Assigns element to match the partitioning of another mesh. If in a "
                             "subapp, defaults to the parent app mesh.");
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

CopyMeshPartitioner::~CopyMeshPartitioner() {}

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
  // If we use the same number of procs, we are matching regions. Makes sense
  // If we use more procs, this will leave some processors with no elements. It is not ideal, let's
  // just give a warning. This does not happen in practice with MultiApps
  // If we use less procs (N_source > N_current), then either:
  // - the elements on our mesh always get matched to elements from only N_current processes among
  //   the N_source processes. We can accomodate that
  // - the elements on our mesh get matched to the full spectrum of N_source processes. We would
  //   need a heuristic to accomodate that. We won't support it
  std::set<unsigned int> pids_used;

  // Get point locator
  auto pl = _base_mesh->getPointLocator();
  pl->enable_out_of_mesh_mode();

  if (dynamic_cast<DistributedMesh *>(&mesh))
    mooseError("Distributed meshes not supported");

  // Assign processor ids to all elements
  for (auto & elem_ptr : mesh.active_element_ptr_range())
  {
    // Get the processor id
    // TODO: support multiapp mesh transformations
    const auto & elem_found = (*pl)(elem_ptr->vertex_average());
    unsigned int tentative_pid = std::numeric_limits<unsigned int>::max();
    if (elem_found)
      tentative_pid = elem_found->processor_id();
    unsigned int found_pid = tentative_pid;

    // Synchronize across all processes
    mesh.comm().min(tentative_pid);
    if (found_pid != std::numeric_limits<unsigned int>::max() && found_pid != tentative_pid)
      mooseWarning("Element: " + elem_ptr->get_info() +
                   "\n was found in two different source partitions: " + std::to_string(found_pid) +
                   " & " + std::to_string(tentative_pid));
    if (tentative_pid == std::numeric_limits<unsigned int>::max())
      mooseError("Element: " + elem_ptr->get_info() + "\n was not located in source mesh");

    elem_ptr->processor_id() = tentative_pid;

    // Keep track of processor ids used in case we need to do a pass at re-assigning
    pids_used.insert(tentative_pid);
  }

  // Check the pids used
  // We cannot use more process ids to partition the mesh than the current app is using
  const auto max_pid = mesh.comm().size();
  if (pids_used.size() > max_pid)
    mooseError("Partitioning copy used more regions (" + std::to_string(pids_used.size()) +
               ") than the number of parallel processes (" + std::to_string(mesh.comm().size()) +
               ")");

  // The pids are not too many, but their numbering is not contiguous, renumber the process id
  // assignment
  if (*pids_used.rbegin() > max_pid)
  {
    std::unordered_map<unsigned int, unsigned int> source_to_current_pid_map;

    // TODO: This is a naive remapping. There might be remapping that have optimal locality. E.g.
    // the mpi ranks are on the same node, limiting communications
    unsigned int i = 0;
    for (const auto pid_set : pids_used)
      source_to_current_pid_map[pid_set] = i++;

    for (auto & elem_ptr : mesh.active_element_ptr_range())
      elem_ptr->processor_id() = source_to_current_pid_map[elem_ptr->processor_id()];
  }
}
