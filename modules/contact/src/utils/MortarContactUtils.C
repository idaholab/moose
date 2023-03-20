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
    const processor_id_type this_id,
    const MooseMesh & mesh,
    const bool nodal,
    const bool normalize_c,
    const Parallel::Communicator & communicator)
{
  // We may have weighted gap information that should go to other processes that own the dofs
  using Datum = std::tuple<dof_id_type, ADReal, Real>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (auto & pr : dof_to_weighted_gap)
  {
    const auto * const dof_object = pr.first;
    const auto proc_id = dof_object->processor_id();
    if (proc_id == this_id)
      continue;

    push_data[proc_id].push_back(
        std::make_tuple(dof_object->id(), std::move(pr.second.first), pr.second.second));
  }

  const auto & lm_mesh = mesh.getMesh();

  auto action_functor =
      [nodal, this_id, &lm_mesh, &dof_to_weighted_gap, &normalize_c](
          const processor_id_type libmesh_dbg_var(pid), const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != this_id, "We do not send messages to ourself here");
    libmesh_ignore(this_id);

    for (auto & tup : sent_data)
    {
      const auto dof_id = std::get<0>(tup);
      const auto * const dof_object =
          nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");
      auto & weighted_gap_pr = dof_to_weighted_gap[dof_object];
      weighted_gap_pr.first += std::move(std::get<1>(tup));
      if (normalize_c)
        weighted_gap_pr.second += std::get<2>(tup);
    }
  };
  TIMPI::push_parallel_vector_data(communicator, push_data, action_functor);
}
}
}
}
