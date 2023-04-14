//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactUtils.h"

#include <tuple>

namespace Moose
{
namespace Mortar
{
namespace Contact
{
void
communicateGaps(
    std::unordered_map<const DofObject *, std::pair<ADReal, Real>> & dof_to_weighted_gap,
    const MooseMesh & mesh,
    const bool nodal,
    const bool normalize_c,
    const Parallel::Communicator & communicator,
    const bool send_data_back)
{
  libmesh_parallel_only(communicator);
  const auto our_proc_id = communicator.rank();

  // We may have weighted gap information that should go to other processes that own the dofs
  using Datum = std::tuple<dof_id_type, ADReal, Real>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (auto & pr : dof_to_weighted_gap)
  {
    const auto * const dof_object = pr.first;
    const auto proc_id = dof_object->processor_id();
    if (proc_id == our_proc_id)
      continue;

    push_data[proc_id].push_back(
        std::make_tuple(dof_object->id(), std::move(pr.second.first), pr.second.second));
  }

  const auto & lm_mesh = mesh.getMesh();
  std::unordered_map<processor_id_type, std::vector<const DofObject *>>
      pid_to_dof_object_for_sending_back;

  auto action_functor =
      [nodal,
       our_proc_id,
       &lm_mesh,
       &dof_to_weighted_gap,
       &normalize_c,
       &pid_to_dof_object_for_sending_back,
       send_data_back](const processor_id_type pid, const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != our_proc_id, "We do not send messages to ourself here");
    libmesh_ignore(our_proc_id);

    for (auto & [dof_id, weighted_gap, normalization] : sent_data)
    {
      const auto * const dof_object =
          nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");
      if (send_data_back)
        pid_to_dof_object_for_sending_back[pid].push_back(dof_object);
      auto & [our_weighted_gap, our_normalization] = dof_to_weighted_gap[dof_object];
      our_weighted_gap += weighted_gap;
      if (normalize_c)
        our_normalization += normalization;
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
      const auto & [our_weighted_gap, our_normalization] =
          libmesh_map_find(dof_to_weighted_gap, dof_object);
      pid_send_data.push_back(
          std::make_tuple(dof_object->id(), our_weighted_gap, our_normalization));
    }
  }

  auto sent_back_action_functor =
      [nodal, our_proc_id, &lm_mesh, &dof_to_weighted_gap, &normalize_c](
          const processor_id_type libmesh_dbg_var(pid), const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != our_proc_id, "We do not send messages to ourself here");
    libmesh_ignore(our_proc_id);

    for (auto & [dof_id, weighted_gap, normalization] : sent_data)
    {
      const auto * const dof_object =
          nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");
      auto & [our_weighted_gap, our_normalization] = dof_to_weighted_gap[dof_object];
      our_weighted_gap = weighted_gap;
      if (normalize_c)
        our_normalization = normalization;
    }
  };
  TIMPI::push_parallel_vector_data(communicator, push_back_data, sent_back_action_functor);
}
}
}
}
