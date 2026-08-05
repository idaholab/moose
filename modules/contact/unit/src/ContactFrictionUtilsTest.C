//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "gtest/gtest.h"

#include "ContactFrictionUtils.h"
#include "TwoVector.h"

#include "libmesh/elem.h"
#include "libmesh/node.h"

#include "metaphysicl/raw_type.h"

#include <cmath>

namespace
{
using Moose::Contact::ContactTangentialFrame;

constexpr Real tolerance = 1.0e-12;

ADRealVectorValue
adVector(const Real x, const Real y, const Real z)
{
  return ADRealVectorValue(x, y, z);
}

void
expectVectorNear(const RealVectorValue & value,
                 const RealVectorValue & expected,
                 const Real local_tolerance = tolerance)
{
  for (const auto component : make_range(LIBMESH_DIM))
    EXPECT_NEAR(value(component), expected(component), local_tolerance);
}
}

TEST(ContactFrictionUtilsTest, materialFrameAndStoredComponents)
{
  const ContactTangentialFrame current_frame = {RealVectorValue(0.0, 1.0, 0.0),
                                                RealVectorValue(-1.0, 0.0, 0.0),
                                                RealVectorValue(0.0, 0.0, 1.0)};
  const TwoVector state(2.0, 3.0);
  const RealVectorValue transported_gap = state(0) * current_frame[0] + state(1) * current_frame[1];

  expectVectorNear(transported_gap, RealVectorValue(-3.0, 2.0, 0.0));
  EXPECT_NEAR(transported_gap.norm(), std::sqrt(13.0), tolerance);
  EXPECT_NEAR(transported_gap * current_frame[2], 0.0, tolerance);

  Node node_zero(0.0, 0.0, 0.0, 0);
  Node node_one(1.0, 0.2, 0.0, 1);
  auto edge = Elem::build(EDGE2);
  edge->set_node(0) = &node_zero;
  edge->set_node(1) = &node_one;

  RealVectorValue preferred_normal(-0.4, 1.0, 0.0);
  preferred_normal /= preferred_normal.norm();
  const auto frame =
      Moose::Contact::buildContactTangentialFrame(*edge, edge->master_point(0), preferred_normal);

  expectVectorNear(frame[2], preferred_normal);
  EXPECT_NEAR(frame[0] * preferred_normal, 0.0, tolerance);
  EXPECT_NEAR(frame[0].norm(), 1.0, tolerance);
  EXPECT_NEAR(frame[1].norm(), 0.0, tolerance);
}

TEST(ContactFrictionUtilsTest, elasticSlipReturn)
{
  constexpr Real friction_coefficient = 0.5;
  constexpr Real contact_pressure = 10.0;
  constexpr Real elastic_slip = 0.2;
  constexpr Real capacity = friction_coefficient * contact_pressure;
  constexpr Real tangential_stiffness = capacity / elastic_slip;

  const auto stick_gap = adVector(0.1, 0.05, 0.0);
  const auto stick = Moose::Contact::elasticSlipReturnMap(
      stick_gap, friction_coefficient, contact_pressure, elastic_slip);
  expectVectorNear(MetaPhysicL::raw_value(stick.multiplier),
                   RealVectorValue(tangential_stiffness * 0.1, tangential_stiffness * 0.05, 0.0));
  expectVectorNear(stick.elastic_gap, RealVectorValue(0.1, 0.05, 0.0));

  const auto sliding_gap = adVector(0.3, 0.4, 0.0);
  const auto sliding = Moose::Contact::elasticSlipReturnMap(
      sliding_gap, friction_coefficient, contact_pressure, elastic_slip);
  expectVectorNear(MetaPhysicL::raw_value(sliding.multiplier), RealVectorValue(3.0, 4.0, 0.0));
  EXPECT_NEAR(sliding.elastic_gap.norm(), elastic_slip, tolerance);

  const auto open = Moose::Contact::elasticSlipReturnMap(
      adVector(0.1, 0.0, 0.0), friction_coefficient, -contact_pressure, elastic_slip);
  expectVectorNear(MetaPhysicL::raw_value(open.multiplier), RealVectorValue());
  expectVectorNear(open.elastic_gap, RealVectorValue());

  constexpr Real small_elastic_slip = 1.0e-8;
  const auto small_slip_limit = Moose::Contact::elasticSlipReturnMap(
      adVector(0.1, 0.0, 0.0), friction_coefficient, contact_pressure, small_elastic_slip);
  EXPECT_NEAR(MetaPhysicL::raw_value(small_slip_limit.multiplier(0)), capacity, tolerance);
  EXPECT_NEAR(small_slip_limit.elastic_gap.norm(), small_elastic_slip, tolerance);
}

TEST(ContactFrictionUtilsTest, arctanCoefficient)
{
  constexpr Real friction_coefficient = 0.6;
  constexpr Real reference_slip = 0.2;

  ADRealVectorValue zero_slip;
  Moose::derivInsert(zero_slip(0).derivatives(), 0, 1.0);
  const ADReal zero_magnitude = Moose::Contact::tangentialSlipMagnitude(zero_slip);
  const ADReal zero_coefficient = Moose::Contact::arctanFrictionCoefficient(
      friction_coefficient, zero_magnitude, reference_slip);
  EXPECT_TRUE(std::isfinite(MetaPhysicL::raw_value(zero_coefficient)));
  EXPECT_DOUBLE_EQ(MetaPhysicL::raw_value(zero_coefficient), 0.0);
  EXPECT_DOUBLE_EQ(zero_coefficient.derivatives()[0], 0.0);

  auto vector_slip = adVector(0.6 * reference_slip, 0.8 * reference_slip, 0.0);
  Moose::derivInsert(vector_slip(0).derivatives(), 0, 1.0);
  const ADReal vector_coefficient = Moose::Contact::arctanFrictionCoefficient(
      friction_coefficient, Moose::Contact::tangentialSlipMagnitude(vector_slip), reference_slip);
  EXPECT_NEAR(MetaPhysicL::raw_value(vector_coefficient), 0.5 * friction_coefficient, tolerance);
  EXPECT_NEAR(vector_coefficient.derivatives()[0],
              0.6 * friction_coefficient / (libMesh::pi * reference_slip),
              tolerance);

  const ADReal small_reference_limit = Moose::Contact::arctanFrictionCoefficient(
      friction_coefficient, reference_slip, 1.0e-12 * reference_slip);
  EXPECT_NEAR(MetaPhysicL::raw_value(small_reference_limit), friction_coefficient, 1.0e-12);
}
