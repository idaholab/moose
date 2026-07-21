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

#include <array>
#include <cmath>

namespace ContactUtils = Moose::Mortar::Contact;

TEST(FrictionProjection, StickAndSlipRoots)
{
  const std::array<Real, 1> stick_pressure = {1.0};
  const std::array<Real, 1> stick_augmented = {1.0};
  const auto ac_stick =
      ContactUtils::alartCurnierFrictionResidual(stick_pressure, stick_augmented, 2.0);
  const auto hsw_stick =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(stick_pressure, stick_augmented, 2.0);
  EXPECT_DOUBLE_EQ(ac_stick[0], 0.0);
  EXPECT_DOUBLE_EQ(hsw_stick[0], 0.0);

  const std::array<Real, 1> slip_pressure = {2.0};
  const std::array<Real, 1> slip_augmented = {4.0};
  const auto ac_slip =
      ContactUtils::alartCurnierFrictionResidual(slip_pressure, slip_augmented, 2.0);
  const auto hsw_slip =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(slip_pressure, slip_augmented, 2.0);
  EXPECT_DOUBLE_EQ(ac_slip[0], 0.0);
  EXPECT_DOUBLE_EQ(hsw_slip[0], 0.0);
}

TEST(FrictionProjection, SeparationAndAugmentedNormalSign)
{
  const Real open_augmented_pressure = ContactUtils::augmentedNormalPressure(1.0, 2.0);
  EXPECT_DOUBLE_EQ(open_augmented_pressure, -1.0);
  EXPECT_DOUBLE_EQ(ContactUtils::coulombFrictionRadius(0.5, open_augmented_pressure), 0.0);

  const std::array<Real, 1> pressure = {0.0};
  const std::array<Real, 1> augmented_pressure = {3.0};
  const auto ac = ContactUtils::alartCurnierFrictionResidual(pressure, augmented_pressure, 0.0);
  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, 0.0);
  EXPECT_DOUBLE_EQ(ac[0], 0.0);
  EXPECT_DOUBLE_EQ(hsw[0], 0.0);
}

TEST(FrictionProjection, ThreeDimensionalProjection)
{
  const std::array<Real, 2> augmented_pressure = {3.0, 4.0};
  const auto projection = ContactUtils::projectToFrictionBall(augmented_pressure, 2.0);
  EXPECT_DOUBLE_EQ(projection[0], 1.2);
  EXPECT_DOUBLE_EQ(projection[1], 1.6);
  EXPECT_DOUBLE_EQ(ContactUtils::tangentialNorm(projection), 2.0);
}

TEST(FrictionProjection, Homogeneity)
{
  const std::array<Real, 2> pressure = {0.7, -0.2};
  const std::array<Real, 2> augmented_pressure = {2.0, -1.0};
  const Real radius = 0.8;
  const Real alpha = 3.0;
  const std::array<Real, 2> scaled_pressure = {alpha * pressure[0], alpha * pressure[1]};
  const std::array<Real, 2> scaled_augmented = {alpha * augmented_pressure[0],
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
    EXPECT_NEAR(ac_scaled[i], alpha * ac[i], 1e-14);
    EXPECT_NEAR(hsw_scaled[i], alpha * alpha * hsw[i], 1e-14);
  }
}

TEST(FrictionProjection, PositiveWeightRelationship)
{
  const std::array<Real, 2> pressure = {0.6, -0.1};
  const std::array<Real, 2> augmented_pressure = {1.5, -0.5};
  const Real radius = 0.9;
  const Real weight = std::max(radius, ContactUtils::tangentialNorm(augmented_pressure));
  ASSERT_GT(weight, 0.0);

  const auto ac = ContactUtils::alartCurnierFrictionResidual(pressure, augmented_pressure, radius);
  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, radius);
  for (const auto i : index_range(pressure))
    EXPECT_NEAR(hsw[i], weight * ac[i], 1e-14);
}

TEST(FrictionProjection, DegreeTwoPressureScaleCompensation)
{
  const std::array<Real, 2> pressure = {0.6, -0.1};
  const std::array<Real, 2> augmented_pressure = {1.5, -0.5};
  const Real radius = 0.9;
  const Real pressure_scale = 2.0;
  const Real alpha = 3.0;
  const std::array<Real, 2> scaled_pressure = {alpha * pressure[0], alpha * pressure[1]};
  const std::array<Real, 2> scaled_augmented = {alpha * augmented_pressure[0],
                                                alpha * augmented_pressure[1]};

  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, radius);
  const auto hsw_scaled = ContactUtils::hueberStadlerWohlmuthFrictionResidual(
      scaled_pressure, scaled_augmented, alpha * radius);
  for (const auto i : index_range(pressure))
    EXPECT_NEAR(hsw_scaled[i] / (alpha * pressure_scale), alpha * hsw[i] / pressure_scale, 1e-14);
}

TEST(FrictionProjection, DegreeTwoDegenerateState)
{
  const std::array<Real, 2> pressure = {2.0, -3.0};
  const std::array<Real, 2> augmented_pressure = {0.0, 0.0};
  const auto ac = ContactUtils::alartCurnierFrictionResidual(pressure, augmented_pressure, 0.0);
  const auto hsw =
      ContactUtils::hueberStadlerWohlmuthFrictionResidual(pressure, augmented_pressure, 0.0);

  for (const auto i : index_range(pressure))
  {
    EXPECT_DOUBLE_EQ(ac[i], pressure[i]);
    EXPECT_DOUBLE_EQ(hsw[i], pressure[i]);
  }
}
