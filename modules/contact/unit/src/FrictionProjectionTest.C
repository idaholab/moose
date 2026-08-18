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
#include "MathUtils.h"

#include "metaphysicl/raw_type.h"

#include <array>
#include <cmath>

namespace ContactUtils = Moose::Mortar::Contact;

using MetaPhysicL::raw_value;

template <typename T>
class FrictionProjectionTest : public ::testing::Test
{
};

using FrictionProjectionTypes = ::testing::Types<Real, ADReal>;
TYPED_TEST_SUITE(FrictionProjectionTest, FrictionProjectionTypes);

TYPED_TEST(FrictionProjectionTest, StickAndSlipRoots)
{
  using T = TypeParam;

  const std::array<T, 1> stick_pressure = {1.0};
  const std::array<T, 1> stick_augmented = {1.0};
  const auto ac_stick =
      ContactUtils::alartCurnierFrictionResidual(stick_pressure, stick_augmented, T(2.0));
  const auto hsw_stick =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(stick_pressure, stick_augmented, T(2.0));
  EXPECT_DOUBLE_EQ(raw_value(ac_stick[0]), 0.0);
  EXPECT_DOUBLE_EQ(raw_value(hsw_stick[0]), 0.0);

  const std::array<T, 1> slip_pressure = {2.0};
  const std::array<T, 1> slip_augmented = {4.0};
  const auto ac_slip =
      ContactUtils::alartCurnierFrictionResidual(slip_pressure, slip_augmented, T(2.0));
  const auto hsw_slip =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(slip_pressure, slip_augmented, T(2.0));
  EXPECT_DOUBLE_EQ(raw_value(ac_slip[0]), 0.0);
  EXPECT_DOUBLE_EQ(raw_value(hsw_slip[0]), 0.0);
}

TYPED_TEST(FrictionProjectionTest, SeparationAndAugmentedNormalSign)
{
  using T = TypeParam;

  const T open_augmented_pressure = ContactUtils::augmentedNormalPressure(T(1.0), T(2.0));
  EXPECT_DOUBLE_EQ(raw_value(open_augmented_pressure), -1.0);
  EXPECT_DOUBLE_EQ(raw_value(ContactUtils::coulombFrictionRadius(T(0.5), open_augmented_pressure)),
                   0.0);

  const std::array<T, 1> pressure = {0.0};
  const std::array<T, 1> augmented_pressure = {3.0};
  const auto ac = ContactUtils::alartCurnierFrictionResidual(pressure, augmented_pressure, T(0.0));
  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, T(0.0));
  EXPECT_DOUBLE_EQ(raw_value(ac[0]), 0.0);
  EXPECT_DOUBLE_EQ(raw_value(hsw[0]), 0.0);
}

TYPED_TEST(FrictionProjectionTest, ThreeDimensionalProjection)
{
  using T = TypeParam;

  const std::array<T, 2> augmented_pressure = {3.0, 4.0};
  const auto projection = ContactUtils::projectToFrictionBall(augmented_pressure, T(2.0));
  EXPECT_DOUBLE_EQ(raw_value(projection[0]), 1.2);
  EXPECT_DOUBLE_EQ(raw_value(projection[1]), 1.6);
  EXPECT_DOUBLE_EQ(raw_value(MathUtils::norm(projection)), 2.0);
}

TYPED_TEST(FrictionProjectionTest, Homogeneity)
{
  using T = TypeParam;

  const std::array<T, 2> pressure = {0.7, -0.2};
  const std::array<T, 2> augmented_pressure = {2.0, -1.0};
  const T radius = 0.8;
  const T alpha = 3.0;
  const std::array<T, 2> scaled_pressure = {alpha * pressure[0], alpha * pressure[1]};
  const std::array<T, 2> scaled_augmented = {alpha * augmented_pressure[0],
                                             alpha * augmented_pressure[1]};

  const auto ac = ContactUtils::alartCurnierFrictionResidual(pressure, augmented_pressure, radius);
  const auto ac_scaled =
      ContactUtils::alartCurnierFrictionResidual(scaled_pressure, scaled_augmented, alpha * radius);
  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, radius);
  const auto hsw_scaled = ContactUtils::hueberStadlerWohlmuthFrictionResidual(
      scaled_pressure, scaled_augmented, alpha * radius);

  for (const auto i : index_range(pressure))
  {
    EXPECT_NEAR(raw_value(ac_scaled[i]), raw_value(alpha * ac[i]), 1e-14);
    EXPECT_NEAR(raw_value(hsw_scaled[i]), raw_value(alpha * alpha * hsw[i]), 1e-14);
  }
}

TYPED_TEST(FrictionProjectionTest, PositiveWeightRelationship)
{
  using T = TypeParam;

  const std::array<T, 2> pressure = {0.6, -0.1};
  const std::array<T, 2> augmented_pressure = {1.5, -0.5};
  const T radius = 0.9;
  const T weight = std::max(radius, MathUtils::norm(augmented_pressure));
  ASSERT_GT(raw_value(weight), 0.0);

  const auto ac = ContactUtils::alartCurnierFrictionResidual(pressure, augmented_pressure, radius);
  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, radius);
  for (const auto i : index_range(pressure))
    EXPECT_NEAR(raw_value(hsw[i]), raw_value(weight * ac[i]), 1e-14);
}

TYPED_TEST(FrictionProjectionTest, DegreeTwoDegenerateState)
{
  using T = TypeParam;

  const std::array<T, 2> pressure = {2.0, -3.0};
  const std::array<T, 2> augmented_pressure = {0.0, 0.0};
  const auto ac = ContactUtils::alartCurnierFrictionResidual(pressure, augmented_pressure, T(0.0));
  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, T(0.0));

  for (const auto i : index_range(pressure))
  {
    EXPECT_DOUBLE_EQ(raw_value(ac[i]), raw_value(pressure[i]));
    EXPECT_DOUBLE_EQ(raw_value(hsw[i]), raw_value(pressure[i]));
  }
}
