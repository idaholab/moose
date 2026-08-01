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

SubdomainID
classify(const bool all_nodes_active,
         const bool all_nodes_inactive,
         const Real active_fraction,
         const Real lambda,
         const bool mark_intercepted)
{
  const SBMUtils::ElementActivity activity{all_nodes_active, all_nodes_inactive, active_fraction};
  const SBMUtils::ClassificationSubdomains subdomains{INSIDE, OUTSIDE, INTERCEPTED};
  return SBMUtils::classifyPartialElement(activity, subdomains, mark_intercepted, lambda);
}
}

// A fully active element is inside; a fully inactive element is outside. The
// downstream inputs (lambda, mark_intercepted) do not override these.
TEST(ClassifyPartialElementTest, FullyInsideOrOutside)
{
  EXPECT_EQ(classify(true, false, 1.0, 0.5, true), INSIDE);
  EXPECT_EQ(classify(false, true, 0.0, 0.5, true), OUTSIDE);
}

// A partial element with mark_intercepted gets the intercepted subdomain, ahead of
// the lambda-threshold decision (checked here at both lambda extremes).
TEST(ClassifyPartialElementTest, MarkIntercepted)
{
  EXPECT_EQ(classify(false, false, 0.5, 0.0, true), INTERCEPTED);
  EXPECT_EQ(classify(false, false, 0.5, 1.0, true), INTERCEPTED);
}

// Without mark_intercepted, a partial element is resolved by the lambda threshold:
// inactive fraction above lambda is outside, below is inside.
TEST(ClassifyPartialElementTest, LambdaThreshold)
{
  EXPECT_EQ(classify(false, false, 0.2, 0.5, false), OUTSIDE); // inactive fraction 0.8 > 0.5
  EXPECT_EQ(classify(false, false, 0.8, 0.5, false), INSIDE);  // inactive fraction 0.2 < 0.5
}

// All nodes on one side does not force the fully-inside/outside result unless the
// active fraction is exactly one/zero: an enclosed surface (all nodes active but a
// partial interior fraction) still routes through the intercepted / lambda logic.
TEST(ClassifyPartialElementTest, EnclosedSurfaceEndpoints)
{
  EXPECT_EQ(classify(true, false, 0.5, 0.5, false), INSIDE);  // inactive fraction 0.5 not > 0.5
  EXPECT_EQ(classify(true, false, 0.5, 0.3, false), OUTSIDE); // inactive fraction 0.5 > 0.3
  EXPECT_EQ(classify(true, false, 0.5, 0.5, true), INTERCEPTED);
}

// isInactive endpoints: lambda zero rejects even a fully active element, lambda one
// accepts even a fully inactive one, and in between it compares the inactive fraction.
TEST(ClassifyPartialElementTest, IsInactive)
{
  EXPECT_TRUE(SBMUtils::isInactive(1.0, 0.0));
  EXPECT_FALSE(SBMUtils::isInactive(0.0, 1.0));
  EXPECT_TRUE(SBMUtils::isInactive(0.2, 0.5));
  EXPECT_FALSE(SBMUtils::isInactive(0.8, 0.5));
}
