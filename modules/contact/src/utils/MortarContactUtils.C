//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactUtils.h"

#include <limits>
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

ConstraintState
combineConstraintStates(const ConstraintState a, const ConstraintState b)
{
  if (a != b)
    mooseError("Contradictory constraint state observations for the same degree of freedom: ",
               static_cast<unsigned int>(a),
               " vs ",
               static_cast<unsigned int>(b));
  return a;
}

void
communicateConstraintStates(
    std::unordered_map<dof_id_type, ConstraintState> & dof_to_state,
    const std::unordered_map<dof_id_type, processor_id_type> & owner_of,
    const Parallel::Communicator & communicator,
    const bool send_data_back)
{
  libmesh_parallel_only(communicator);
  const auto our_proc_id = communicator.rank();

  // We may have constraint-state observations that should go to other processes that own the
  // dofs. The wire datum stores the enum as its underlying unsigned char, since TIMPI has no
  // generic StandardType/Packing specialization for an arbitrary enum class.
  using Datum = std::pair<dof_id_type, unsigned char>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (const auto & [dof_id, state] : dof_to_state)
  {
    const auto owner = libmesh_map_find(owner_of, dof_id);
    if (owner == our_proc_id)
      continue;

    push_data[owner].push_back({dof_id, static_cast<unsigned char>(state)});
  }

  std::unordered_map<processor_id_type, std::vector<dof_id_type>> pid_to_dof_id_for_sending_back;

  // A contradiction is recorded rather than thrown immediately: throwing from inside this
  // callback would exit the function on only the rank that happens to own the contradicting
  // dof, before that rank reaches the send-back collective below. Every other rank, having
  // detected no local contradiction, would then block forever in that collective waiting for a
  // participant that already unwound its stack. Instead every rank finishes both communication
  // rounds, agrees via a reduction on whether any rank saw a contradiction, and only then throws
  // identically everywhere.
  bool contradiction_detected = false;

  auto action_functor = [our_proc_id,
                          &dof_to_state,
                          &pid_to_dof_id_for_sending_back,
                          &contradiction_detected,
                          send_data_back](const processor_id_type pid, const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != our_proc_id, "We do not send messages to ourself here");
    libmesh_ignore(our_proc_id);

    for (const auto & [dof_id, byte] : sent_data)
    {
      const auto state = static_cast<ConstraintState>(byte);
      if (send_data_back)
        pid_to_dof_id_for_sending_back[pid].push_back(dof_id);

      const auto it = dof_to_state.find(dof_id);
      if (it == dof_to_state.end())
        dof_to_state.emplace(dof_id, state);
      else if (it->second != state)
        contradiction_detected = true;
    }
  };

  TIMPI::push_parallel_vector_data(communicator, push_data, action_functor);

  communicator.max(contradiction_detected);
  if (contradiction_detected)
    mooseError("Contradictory constraint state observations for the same degree of freedom.");

  // Now send data back if requested
  if (!send_data_back)
    return;

  std::unordered_map<processor_id_type, std::vector<Datum>> push_back_data;

  for (const auto & [pid, dof_ids] : pid_to_dof_id_for_sending_back)
  {
    auto & pid_send_data = push_back_data[pid];
    pid_send_data.reserve(dof_ids.size());
    for (const auto dof_id : dof_ids)
    {
      const auto & canonical_state = libmesh_map_find(dof_to_state, dof_id);
      pid_send_data.push_back({dof_id, static_cast<unsigned char>(canonical_state)});
    }
  }

  auto sent_back_action_functor = [our_proc_id, &dof_to_state](
                                       const processor_id_type libmesh_dbg_var(pid),
                                       const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != our_proc_id, "We do not send messages to ourself here");
    libmesh_ignore(our_proc_id);

    for (const auto & [dof_id, byte] : sent_data)
      dof_to_state[dof_id] = static_cast<ConstraintState>(byte);
  };
  TIMPI::push_parallel_vector_data(communicator, push_back_data, sent_back_action_functor);
}

ConstraintStateDiff
symmetricDifference(const std::unordered_map<dof_id_type, ConstraintState> & prev,
                     const std::unordered_map<dof_id_type, ConstraintState> & curr)
{
  ConstraintStateDiff diff;
  for (const auto & [dof_id, curr_state] : curr)
  {
    const auto prev_state = libmesh_map_find(prev, dof_id);

    if (prev_state == ConstraintState::OPEN && curr_state != ConstraintState::OPEN)
      diff.newly_contact.push_back(dof_id);
    if (prev_state != ConstraintState::OPEN && curr_state == ConstraintState::OPEN)
      diff.newly_open.push_back(dof_id);
    if (curr_state == ConstraintState::CONTACT_STICK &&
        prev_state != ConstraintState::CONTACT_STICK)
      diff.newly_stick.push_back(dof_id);
    if (curr_state == ConstraintState::CONTACT_SLIP && prev_state != ConstraintState::CONTACT_SLIP)
      diff.newly_slip.push_back(dof_id);
  }
  return diff;
}

ConstraintStateSets
constraintStateSets(const std::unordered_map<dof_id_type, ConstraintState> & canonical)
{
  ConstraintStateSets sets;
  for (const auto & [dof_id, state] : canonical)
  {
    switch (state)
    {
      case ConstraintState::OPEN:
        sets.A_open.insert(dof_id);
        break;
      case ConstraintState::CONTACT_STICK:
        sets.A_contact.insert(dof_id);
        sets.A_stick.insert(dof_id);
        break;
      case ConstraintState::CONTACT_SLIP:
        sets.A_contact.insert(dof_id);
        sets.A_slip.insert(dof_id);
        break;
      case ConstraintState::CONTACT_UNCLASSIFIED:
        sets.A_contact.insert(dof_id);
        break;
    }
  }
  return sets;
}

Real
directionalDerivative(const ADReal & q, const std::unordered_map<dof_id_type, Real> & direction)
{
  const auto & indices = q.derivatives().nude_indices();
  const auto & data = q.derivatives().nude_data();

  Real qdot = 0;
  for (const auto i : index_range(indices))
  {
    const auto it = direction.find(indices[i]);
    if (it != direction.end())
      qdot += data[i] * it->second;
  }
  return qdot;
}

std::optional<Real>
eventStepLength(const Real q, const Real qdot)
{
  if (qdot == 0)
    return std::nullopt;

  const Real alpha = -q / qdot;
  if (alpha > 0 && alpha <= 1)
    return alpha;
  return std::nullopt;
}

std::optional<FirstEventGroup>
firstEventGroup(const std::unordered_map<dof_id_type, Real> & predicted_alphas,
                 const Real tau_event)
{
  if (predicted_alphas.empty())
    return std::nullopt;

  Real alpha_min = std::numeric_limits<Real>::max();
  for (const auto & [dof_id, alpha] : predicted_alphas)
  {
    libmesh_ignore(dof_id);
    alpha_min = std::min(alpha_min, alpha);
  }

  FirstEventGroup group;
  group.alpha_min = alpha_min;
  for (const auto & [dof_id, alpha] : predicted_alphas)
    if (std::abs(alpha - alpha_min) <= tau_event)
      group.members.insert(dof_id);
  return group;
}

Real
meritFunction(const Real fnorm)
{
  return 0.5 * fnorm * fnorm;
}

bool
watchdogBoundPermits(const Real phi_candidate, const Real phi_ke, const Real gamma)
{
  return phi_candidate <= gamma * phi_ke;
}

bool
watchdogRecovered(const Real phi_committed, const Real phi_ke)
{
  return phi_committed <= phi_ke;
}

ConstraintState
classifyNormalState(const ADReal & lm_value, const ADReal & weighted_gap, const Real c)
{
  return (lm_value - c * weighted_gap) >= 0 ? ConstraintState::CONTACT_UNCLASSIFIED
                                            : ConstraintState::OPEN;
}

ConstraintState
classifyFrictionalState(const ADReal & augmented_tangential_pressure,
                        const ADReal & radius,
                        const ADReal & normal_pressure,
                        const ADReal & epsilon)
{
  if (normal_pressure < epsilon)
    return ConstraintState::OPEN;
  return tangentialNorm(std::array<ADReal, 1>{{augmented_tangential_pressure}}) <= radius
             ? ConstraintState::CONTACT_STICK
             : ConstraintState::CONTACT_SLIP;
}

ConstraintState
classifyFrictionalState3d(const std::array<ADReal, 2> & augmented_tangential_pressure,
                          const ADReal & radius,
                          const ADReal & normal_pressure,
                          const ADReal & epsilon)
{
  if (normal_pressure < epsilon)
    return ConstraintState::OPEN;
  return tangentialNorm(augmented_tangential_pressure) <= radius ? ConstraintState::CONTACT_STICK
                                                                  : ConstraintState::CONTACT_SLIP;
}

ConstraintState
applyTangentialHysteresis(const ConstraintState raw,
                          const ConstraintState previous,
                          const Real switch_value,
                          const Real tau)
{
  if (tau <= 0)
    return raw;

  const auto is_stick_or_slip = [](const ConstraintState state)
  { return state == ConstraintState::CONTACT_STICK || state == ConstraintState::CONTACT_SLIP; };

  if (is_stick_or_slip(raw) && is_stick_or_slip(previous) && std::abs(switch_value) <= tau)
    return previous;
  return raw;
}
}
}
}
