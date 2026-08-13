//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MortarContactUtils.h"
#include "MooseUnitUtils.h"

#include "libmesh/parallel.h"

namespace ContactUtils = Moose::Mortar::Contact;
using ContactUtils::ConstraintState;

TEST(ConstraintStateProvider, CombineAgreeingStates)
{
  const std::array<ConstraintState, 4> states = {ConstraintState::OPEN,
                                                  ConstraintState::CONTACT_STICK,
                                                  ConstraintState::CONTACT_SLIP,
                                                  ConstraintState::CONTACT_UNCLASSIFIED};
  for (const auto state : states)
    EXPECT_EQ(ContactUtils::combineConstraintStates(state, state), state);
}

TEST(ConstraintStateProvider, CombineContradictoryStatesThrows)
{
  EXPECT_THROW_MSG_CONTAINS(
      ContactUtils::combineConstraintStates(ConstraintState::OPEN, ConstraintState::CONTACT_STICK),
      MooseRuntimeError,
      "Contradictory constraint state observations");
  EXPECT_THROW_MSG_CONTAINS(ContactUtils::combineConstraintStates(ConstraintState::CONTACT_STICK,
                                                                   ConstraintState::CONTACT_SLIP),
                            MooseRuntimeError,
                            "Contradictory constraint state observations");
  // A frictionless observer and a frictional observer of the same dof can never both be
  // correct: a formulation is either uniformly frictionless (OPEN/CONTACT_UNCLASSIFIED) or
  // uniformly frictional (OPEN/CONTACT_STICK/CONTACT_SLIP). This is a hard contradiction too.
  EXPECT_THROW_MSG_CONTAINS(ContactUtils::combineConstraintStates(
                                ConstraintState::CONTACT_UNCLASSIFIED, ConstraintState::CONTACT_STICK),
                            MooseRuntimeError,
                            "Contradictory constraint state observations");
}

TEST(ConstraintStateProvider, SymmetricDifferenceIdentifiesTransitions)
{
  // id 1: OPEN -> CONTACT_STICK (newly_contact and newly_stick)
  // id 2: CONTACT_STICK -> CONTACT_SLIP (newly_slip only, already closed)
  // id 3: CONTACT_SLIP -> OPEN (newly_open only)
  // id 4: CONTACT_UNCLASSIFIED -> CONTACT_UNCLASSIFIED (no transition)
  const std::unordered_map<dof_id_type, ConstraintState> prev = {
      {1, ConstraintState::OPEN},
      {2, ConstraintState::CONTACT_STICK},
      {3, ConstraintState::CONTACT_SLIP},
      {4, ConstraintState::CONTACT_UNCLASSIFIED}};
  const std::unordered_map<dof_id_type, ConstraintState> curr = {
      {1, ConstraintState::CONTACT_STICK},
      {2, ConstraintState::CONTACT_SLIP},
      {3, ConstraintState::OPEN},
      {4, ConstraintState::CONTACT_UNCLASSIFIED}};

  const auto diff = ContactUtils::symmetricDifference(prev, curr);

  EXPECT_EQ(diff.newly_contact, std::vector<dof_id_type>{1});
  EXPECT_EQ(diff.newly_open, std::vector<dof_id_type>{3});
  EXPECT_EQ(diff.newly_stick, std::vector<dof_id_type>{1});
  EXPECT_EQ(diff.newly_slip, std::vector<dof_id_type>{2});
}

TEST(ConstraintStateProvider, ConstraintStateSetsPartitionsCorrectly)
{
  const std::unordered_map<dof_id_type, ConstraintState> canonical = {
      {1, ConstraintState::OPEN},
      {2, ConstraintState::CONTACT_STICK},
      {3, ConstraintState::CONTACT_SLIP},
      {4, ConstraintState::CONTACT_UNCLASSIFIED}};

  const auto sets = ContactUtils::constraintStateSets(canonical);

  EXPECT_EQ(sets.A_open, std::unordered_set<dof_id_type>{1});
  EXPECT_EQ(sets.A_stick, std::unordered_set<dof_id_type>{2});
  EXPECT_EQ(sets.A_slip, std::unordered_set<dof_id_type>{3});
  EXPECT_EQ(sets.A_contact, (std::unordered_set<dof_id_type>{2, 3, 4}));

  // A_contact is exactly the disjoint union of A_stick, A_slip, and the CONTACT_UNCLASSIFIED ids
  for (const auto dof_id : sets.A_stick)
    EXPECT_TRUE(sets.A_contact.count(dof_id));
  for (const auto dof_id : sets.A_slip)
    EXPECT_TRUE(sets.A_contact.count(dof_id));
  for (const auto dof_id : sets.A_open)
    EXPECT_FALSE(sets.A_contact.count(dof_id));
}

// Genuine cross-rank communication is only exercised with more than one rank:
// mpiexec -n 2 modules/contact/unit/contact-unit-opt --gtest_filter=ConstraintStateProvider.*
// At the default 1-rank invocation, this test degenerates to confirming the local map is
// unchanged - still meaningful since every id is locally owned in that case.
TEST(ConstraintStateProvider, ReconciliationAgreement)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_WORLD);
  const auto n = comm.size();

  // Every rank observes the same two global ids with agreeing states; id 1 is always owned by
  // rank 0, id 2 by the last rank.
  std::unordered_map<dof_id_type, ConstraintState> dof_to_state = {
      {1, ConstraintState::OPEN}, {2, ConstraintState::CONTACT_STICK}};
  const std::unordered_map<dof_id_type, processor_id_type> owner_of = {
      {1, 0}, {2, static_cast<processor_id_type>(n - 1)}};

  ContactUtils::communicateConstraintStates(dof_to_state, owner_of, comm, /*send_data_back=*/true);

  EXPECT_EQ(libmesh_map_find(dof_to_state, 1), ConstraintState::OPEN);
  EXPECT_EQ(libmesh_map_find(dof_to_state, 2), ConstraintState::CONTACT_STICK);
}

// See ReconciliationAgreement for the cross-rank invocation required to exercise genuine
// cross-rank communication.
TEST(ConstraintStateProvider, ContradictoryAssignmentDetected)
{
  libMesh::Parallel::Communicator comm(MPI_COMM_WORLD);
  const auto rank = comm.rank();
  const auto n = comm.size();

  // Rank 0 observes id 1 as OPEN; every other rank (or, at n==1, rank 0 itself via a second
  // local entry) observes id 1 as CONTACT_STICK. Owned by rank 0 so the contradiction is
  // detected either locally (n==1) or after the wire round-trip (n>1).
  std::unordered_map<dof_id_type, ConstraintState> dof_to_state;
  if (n == 1)
    dof_to_state = {{1, ConstraintState::OPEN}};
  else if (rank == 0)
    dof_to_state = {{1, ConstraintState::OPEN}};
  else
    dof_to_state = {{1, ConstraintState::CONTACT_STICK}};

  if (n == 1)
  {
    // Degenerate case: combine two of this rank's own contradictory local observations
    // directly, still exercising the combineConstraintStates mooseError path.
    EXPECT_THROW_MSG_CONTAINS(
        ContactUtils::combineConstraintStates(libmesh_map_find(dof_to_state, 1),
                                               ConstraintState::CONTACT_STICK),
        MooseRuntimeError,
        "Contradictory constraint state observations");
    return;
  }

  const std::unordered_map<dof_id_type, processor_id_type> owner_of = {{1, 0}};
  EXPECT_THROW_MSG_CONTAINS(
      ContactUtils::communicateConstraintStates(dof_to_state, owner_of, comm, /*send_data_back=*/true),
      MooseRuntimeError,
      "Contradictory constraint state observations");
}
