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

namespace ContactUtils = Moose::Mortar::Contact;

TEST(EventStepLength, InRangeCrossing)
{
  // q0 = 1, qdot = -2 -> alpha = -1/-2 = 0.5, admissible
  const auto alpha = ContactUtils::eventStepLength(1.0, -2.0);
  ASSERT_TRUE(alpha.has_value());
  EXPECT_DOUBLE_EQ(*alpha, 0.5);
}

TEST(EventStepLength, PastOneIsRejected)
{
  // q0 = 1, qdot = -0.5 -> alpha = 2, outside (0, 1]
  EXPECT_FALSE(ContactUtils::eventStepLength(1.0, -0.5).has_value());
}

TEST(EventStepLength, NonPositiveIsRejected)
{
  // q0 = 1, qdot = 2 -> alpha = -0.5, the direction moves further from the switching surface
  EXPECT_FALSE(ContactUtils::eventStepLength(1.0, 2.0).has_value());
}

TEST(EventStepLength, ZeroDerivativeIsRejected)
{
  // q is locally flat along d_k: no predicted crossing, regardless of q's sign
  EXPECT_FALSE(ContactUtils::eventStepLength(1.0, 0.0).has_value());
  EXPECT_FALSE(ContactUtils::eventStepLength(-1.0, 0.0).has_value());
}

TEST(EventStepLength, BoundaryInclusive)
{
  // alpha == 1 is admissible ("0 < alpha <= 1")
  const auto alpha_one = ContactUtils::eventStepLength(1.0, -1.0);
  ASSERT_TRUE(alpha_one.has_value());
  EXPECT_DOUBLE_EQ(*alpha_one, 1.0);

  // alpha == 0 (q already at the switching surface) is not treated as a future event
  EXPECT_FALSE(ContactUtils::eventStepLength(0.0, -1.0).has_value());
}

TEST(DirectionalDerivative, ContractsMatchingDofsAndIgnoresMissing)
{
  ADReal q = 0;
  Moose::derivInsert(q.derivatives(), 1, 2.0);
  Moose::derivInsert(q.derivatives(), 2, 3.0);
  Moose::derivInsert(q.derivatives(), 3, 5.0);

  // dof 3 is a dof q depends on that the direction vector does not cover (e.g. a primary-side
  // displacement dof outside the caller's Newton direction slice); dof 4 is the opposite case,
  // a direction entry for a dof q does not depend on. Both must contribute nothing.
  const std::unordered_map<dof_id_type, Real> direction = {{1, 4.0}, {2, -1.0}, {4, 10.0}};

  // Only dofs 1 and 2 overlap: 2.0*4.0 + 3.0*(-1.0) = 5.0
  EXPECT_DOUBLE_EQ(ContactUtils::directionalDerivative(q, direction), 5.0);
}

TEST(DirectionalDerivative, EmptyDirectionIsZero)
{
  ADReal q = 0;
  Moose::derivInsert(q.derivatives(), 1, 2.0);
  const std::unordered_map<dof_id_type, Real> direction;
  EXPECT_DOUBLE_EQ(ContactUtils::directionalDerivative(q, direction), 0.0);
}

TEST(FirstEventGroup, EmptyInput)
{
  const std::unordered_map<dof_id_type, Real> predicted_alphas;
  EXPECT_FALSE(ContactUtils::firstEventGroup(predicted_alphas, 1e-6).has_value());
}

TEST(FirstEventGroup, SingleEntry)
{
  const std::unordered_map<dof_id_type, Real> predicted_alphas = {{7, 0.3}};
  const auto group = ContactUtils::firstEventGroup(predicted_alphas, 1e-6);
  ASSERT_TRUE(group.has_value());
  EXPECT_DOUBLE_EQ(group->alpha_min, 0.3);
  EXPECT_EQ(group->members, (std::unordered_set<dof_id_type>{7}));
}

TEST(FirstEventGroup, TiedGroupIndependentOfInsertionOrder)
{
  // dofs 1 and 2 are tied within tau_event of alpha_min; dof 3 is clearly separated and must be
  // excluded even though it is the closest remaining candidate.
  const Real tau_event = 1e-4;
  const std::vector<std::pair<dof_id_type, Real>> entries = {
      {1, 0.20005}, {2, 0.2}, {3, 0.25}};

  for (const bool forward : {true, false})
  {
    std::unordered_map<dof_id_type, Real> predicted_alphas;
    if (forward)
      for (const auto & [dof_id, alpha] : entries)
        predicted_alphas.emplace(dof_id, alpha);
    else
      for (auto it = entries.rbegin(); it != entries.rend(); ++it)
        predicted_alphas.emplace(it->first, it->second);

    const auto group = ContactUtils::firstEventGroup(predicted_alphas, tau_event);
    ASSERT_TRUE(group.has_value());
    EXPECT_DOUBLE_EQ(group->alpha_min, 0.2);
    EXPECT_EQ(group->members, (std::unordered_set<dof_id_type>{1, 2}));
  }
}

TEST(EventPrediction, PredictedCrossingIsSelfConsistent)
{
  // "Predicted vs actual on a known crossing": for an exactly linear switching function
  // q(alpha) = q0 + qdot*alpha, the linearization eventStepLength() relies on is exact, so the
  // predicted crossing must zero the model to within floating point precision.
  const Real q0 = 2.5;
  const Real qdot = -3.0;

  const auto alpha = ContactUtils::eventStepLength(q0, qdot);
  ASSERT_TRUE(alpha.has_value());
  EXPECT_NEAR(q0 + qdot * *alpha, 0.0, 1e-12);
}
