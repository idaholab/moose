//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "SBMUtils.h"

namespace
{
constexpr SubdomainID INSIDE = 1;
constexpr SubdomainID OUTSIDE = 2;
constexpr SubdomainID INTERCEPTED = 3;
constexpr SBMUtils::InterceptedSubdomainPolicy MARK_INTERCEPTED{true, INTERCEPTED};
constexpr SBMUtils::InterceptedSubdomainPolicy DO_NOT_MARK_INTERCEPTED{false, INTERCEPTED};

SubdomainID
classify(const bool all_nodes_in_domain,
         const bool all_nodes_outside_domain,
         const Real domain_fraction,
         const Real lambda,
         const bool mark_intercepted)
{
  const SBMUtils::ElementDomainOccupancy occupancy{
      all_nodes_in_domain, all_nodes_outside_domain, domain_fraction};
  const SBMUtils::ClassificationSubdomains subdomain_id_settings{INSIDE, OUTSIDE, INTERCEPTED};
  return SBMUtils::classifySubdomainFromOccupancy(
      occupancy, subdomain_id_settings, mark_intercepted, lambda);
}
}

// A fully active element is inside; a fully inactive element is outside. The downstream
// inputs do not override these: neither mark_intercepted nor the lambda endpoints (which
// otherwise force intercepted/outside/inside) change the result.
TEST(ClassifySubdomainFromOccupancyTest, FullyInsideOrOutside)
{
  EXPECT_EQ(classify(true, false, 1.0, 0.5, true), INSIDE);
  EXPECT_EQ(classify(true, false, 1.0, 0.0, false), INSIDE); // lambda 0 rejects a partial elem
  EXPECT_EQ(classify(true, false, 1.0, 1.0, false), INSIDE);

  EXPECT_EQ(classify(false, true, 0.0, 0.5, true), OUTSIDE);
  EXPECT_EQ(classify(false, true, 0.0, 1.0, false), OUTSIDE); // lambda 1 accepts a partial elem
  EXPECT_EQ(classify(false, true, 0.0, 0.0, false), OUTSIDE);
}

// A partial element with mark_intercepted gets the intercepted subdomain, ahead of
// the lambda-threshold decision (checked here at both lambda extremes).
TEST(ClassifySubdomainFromOccupancyTest, MarkIntercepted)
{
  EXPECT_EQ(classify(false, false, 0.5, 0.0, true), INTERCEPTED);
  EXPECT_EQ(classify(false, false, 0.5, 1.0, true), INTERCEPTED);
}

// Without mark_intercepted, a partial element is resolved by the lambda threshold:
// inactive fraction above lambda is outside, below is inside.
TEST(ClassifySubdomainFromOccupancyTest, LambdaThreshold)
{
  EXPECT_EQ(classify(false, false, 0.2, 0.5, false), OUTSIDE); // inactive fraction 0.8 > 0.5
  EXPECT_EQ(classify(false, false, 0.8, 0.5, false), INSIDE);  // inactive fraction 0.2 < 0.5
}

// All nodes on one side does not force the fully-inside/outside result unless the
// active fraction is exactly one/zero: an enclosed surface (all nodes active but a
// partial interior fraction) still routes through the intercepted / lambda logic.
TEST(ClassifySubdomainFromOccupancyTest, EnclosedSurfaceEndpoints)
{
  EXPECT_EQ(classify(true, false, 0.5, 0.5, false), INSIDE);  // inactive fraction 0.5 not > 0.5
  EXPECT_EQ(classify(true, false, 0.5, 0.3, false), OUTSIDE); // inactive fraction 0.5 > 0.3
  EXPECT_EQ(classify(true, false, 0.5, 0.5, true), INTERCEPTED);
}

// isInactive endpoints: lambda zero rejects even a fully active element, lambda one
// accepts even a fully inactive one, and in between it compares the inactive fraction.
TEST(ClassifySubdomainFromOccupancyTest, IsInactive)
{
  EXPECT_TRUE(SBMUtils::isInactive(1.0, 0.0));
  EXPECT_FALSE(SBMUtils::isInactive(0.0, 1.0));
  EXPECT_TRUE(SBMUtils::isInactive(0.2, 0.5));
  EXPECT_FALSE(SBMUtils::isInactive(0.8, 0.5));
}

TEST(SelectSubdomainFromOccupanciesTest, FullyOccupiedTakesPrecedence)
{
  const std::vector<SBMUtils::SubdomainOccupancy> candidates{
      {4, {false, false, 0.9}}, {7, {true, false, 1.0}}, {3, {true, false, 1.0}}};

  EXPECT_EQ(SBMUtils::selectSubdomainFromOccupancies(candidates, MARK_INTERCEPTED, 0.5), 3);
}

TEST(SelectSubdomainFromOccupanciesTest, PartialSelection)
{
  // This perturbation is small enough for absoluteFuzzyEqual, so the lower ID must win the tie.
  const std::vector<SBMUtils::SubdomainOccupancy> candidates{
      {7, {false, false, 0.8 + 1e-14}}, {3, {false, false, 0.8}}, {4, {false, false, 0.6}}};

  EXPECT_EQ(SBMUtils::selectSubdomainFromOccupancies(candidates, DO_NOT_MARK_INTERCEPTED, 0.5), 3);
  EXPECT_EQ(SBMUtils::selectSubdomainFromOccupancies(candidates, MARK_INTERCEPTED, 0.5),
            INTERCEPTED);
  EXPECT_FALSE(SBMUtils::selectSubdomainFromOccupancies(candidates, DO_NOT_MARK_INTERCEPTED, 0.0));
  EXPECT_EQ(SBMUtils::selectSubdomainFromOccupancies(candidates, DO_NOT_MARK_INTERCEPTED, 1.0), 3);
}

TEST(SelectSubdomainFromOccupanciesTest, NoOccupiedCandidate)
{
  const std::vector<SBMUtils::SubdomainOccupancy> candidates{{3, {false, true, 0.0}},
                                                             {7, {false, true, 0.0}}};

  EXPECT_FALSE(SBMUtils::selectSubdomainFromOccupancies(candidates, DO_NOT_MARK_INTERCEPTED, 0.5));
  EXPECT_FALSE(SBMUtils::selectSubdomainFromOccupancies({}, MARK_INTERCEPTED, 0.5));
}
