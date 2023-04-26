//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseConfig.h"

#include "FEProblemBase.h"
#include "ADReal.h"
#include "MooseMesh.h"

#include "libmesh/dof_object.h"

#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"

#include "timpi/parallel_sync.h"
#include "timpi/communicator.h"

#include <utility>
#include <array>
#include <unordered_map>
#include <vector>

namespace Moose
{
namespace Mortar
{
namespace Contact
{

/**
 * This function is used to communicate velocities across processes
 * @param dof_to_weighted_gap Map from degree of freedom to weighted (weak) gap
 * @param mesh Mesh used to locate nodes or elements
 * @param nodal Whether the element has Lagrange interpolation
 * @param communicator Process communicator
 * @param send_data_back Whether to send back data to a distributed constraint
 */
template <typename T>
inline void
communicateVelocities(std::unordered_map<const DofObject *, std::array<T, 2>> & dof_map,
                      const MooseMesh & mesh,
                      const bool nodal,
                      const Parallel::Communicator & communicator,
                      const bool send_data_back)
{
  const auto our_proc_id = communicator.rank();

  // We may have weighted gap information that should go to other processes that own the dofs
  using Datum = std::pair<dof_id_type, std::array<T, 2>>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (auto & pr : dof_map)
  {
    const auto * const dof_object = pr.first;
    const auto proc_id = dof_object->processor_id();
    if (proc_id == our_proc_id)
      continue;

    push_data[proc_id].push_back(std::make_pair(dof_object->id(), std::move(pr.second)));
  }

  const auto & lm_mesh = mesh.getMesh();
  std::unordered_map<processor_id_type, std::vector<const DofObject *>>
      pid_to_dof_object_for_sending_back;

  auto action_functor =
      [nodal, our_proc_id, &lm_mesh, &dof_map, &pid_to_dof_object_for_sending_back, send_data_back](
          const processor_id_type pid, const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != our_proc_id, "We do not send messages to ourself here");
    libmesh_ignore(our_proc_id);

    for (auto & pr : sent_data)
    {
      const auto dof_id = pr.first;
      const auto * const dof_object =
          nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");

      if (send_data_back)
        pid_to_dof_object_for_sending_back[pid].push_back(dof_object);

      dof_map[dof_object][0] += pr.second[0];
      dof_map[dof_object][1] += pr.second[1];
    }
  };

  TIMPI::push_parallel_vector_data(communicator, push_data, action_functor);

  // Now send data back if requested
  if (!send_data_back)
    return;

  std::unordered_map<processor_id_type, std::vector<Datum>> push_back_data;

  for (const auto & [pid, dof_objects] : pid_to_dof_object_for_sending_back)
  {
    auto & pid_send_data = push_back_data[pid];
    pid_send_data.reserve(dof_objects.size());
    for (const DofObject * const dof_object : dof_objects)
    {
      const auto & [tangent_one, tangent_two] = libmesh_map_find(dof_map, dof_object);
      pid_send_data.push_back({dof_object->id(), {tangent_one, tangent_two}});
    }
  }

  auto sent_back_action_functor =
      [nodal, our_proc_id, &lm_mesh, &dof_map](const processor_id_type libmesh_dbg_var(pid),
                                               const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != our_proc_id, "We do not send messages to ourself here");
    libmesh_ignore(our_proc_id);

    for (auto & [dof_id, tangents] : sent_data)
    {
      const auto * const dof_object =
          nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");
      auto & [our_tangent_one, our_tangent_two] = dof_map[dof_object];
      our_tangent_one = tangents[0];
      our_tangent_two = tangents[1];
    }
  };

  TIMPI::push_parallel_vector_data(communicator, push_back_data, sent_back_action_functor);
}

/**
 * This function is used to communicate gaps across processes
 * @param dof_to_weighted_gap Map from degree of freedom to weighted (weak) gap
 * @param mesh Mesh used to locate nodes or elements
 * @param nodal Whether the element has Lagrange interpolation
 * @param normalize_c Whether to normalize with size the c coefficient in contact constraint
 * @param communicator Process communicator
 * @param send_data_back After aggregating data on the owning process, whether to send the aggregate
 * back to senders. This can be necessary for things like penalty contact in which the constraint is
 * not enforced by the owner but in a weighted way by the displacement constraints
 */
void communicateGaps(
    std::unordered_map<const DofObject *, std::pair<ADReal, Real>> & dof_to_weighted_gap,
    const MooseMesh & mesh,
    bool nodal,
    bool normalize_c,
    const Parallel::Communicator & communicator,
    bool send_data_back);

/**
 * This function is used to communicate real quantities across processes
 * @param dof_to_adreal Map from degree of freedom to ADReal
 * @param mesh Mesh used to locate nodes or elements
 * @param nodal Whether the element has Lagrange interpolation
 * @param communicator Process communicator
 * @param send_data_back After aggregating data on the owning process, whether to send the aggregate
 * back to senders. This can be necessary for things like penalty contact in which the constraint is
 * not enforced by the owner but in a weighted way by the displacement constraints
 */
void communicateReals(std::unordered_map<const DofObject *, ADReal> & dof_to_adreal,
                      const MooseMesh & mesh,
                      bool nodal,
                      const Parallel::Communicator & communicator,
                      bool send_data_back);
}
}
}
