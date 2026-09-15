//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ADUtils.h"
#include "AutomaticMortarGeneration.h"

TEST(MortarNodalGeometryTest, weightedNormalAndHouseholderTangents)
{
  constexpr dof_id_type derivative_index = 0;
  // For central differences, this step balances truncation and roundoff; the derivative tolerance
  // covers that approximation, while direct orthogonality checks use a roundoff-level tolerance.
  constexpr Real epsilon = 1e-7;
  constexpr Real finite_difference_tolerance = 1e-8;
  constexpr Real orthogonality_tolerance = 1e-14;
  constexpr Real coordinate = 0.4;

  const auto tangents = [](const auto & perturbed_coordinate)
  {
    using T = std::decay_t<decltype(perturbed_coordinate)>;
    const libMesh::VectorValue<T> first_area =
        libMesh::VectorValue<T>(perturbed_coordinate, -0.3, 0.8)
            .cross(libMesh::VectorValue<T>(0.2, 0.9, -0.1));
    const libMesh::VectorValue<T> second_area =
        libMesh::VectorValue<T>(0.1, perturbed_coordinate, 0.7)
            .cross(libMesh::VectorValue<T>(-0.6, 0.3, 0.2));
    const libMesh::VectorValue<T> normal = (0.7 * first_area + 1.2 * second_area).unit();
    return std::make_pair(normal, Moose::Mortar::householderTangents(normal));
  };

  ADReal ad_coordinate = coordinate;
  Moose::derivInsert(ad_coordinate.derivatives(), derivative_index, 1.0);
  const auto [ad_normal, ad_tangents] = tangents(ad_coordinate);
  const auto [plus_normal, plus_tangents] = tangents(coordinate + epsilon);
  const auto [minus_normal, minus_tangents] = tangents(coordinate - epsilon);

  const auto normal_finite_difference = (plus_normal - minus_normal) / (2 * epsilon);
  for (const auto component : make_range(Moose::dim))
    EXPECT_NEAR(ad_normal(component).derivatives()[derivative_index],
                normal_finite_difference(component),
                finite_difference_tolerance);

  for (const auto direction : make_range(2))
  {
    EXPECT_NEAR(
        MetaPhysicL::raw_value(ad_normal * ad_tangents[direction]), 0.0, orthogonality_tolerance);
    EXPECT_NEAR(
        MetaPhysicL::raw_value(ad_tangents[direction].norm()), 1.0, orthogonality_tolerance);

    const auto finite_difference =
        (plus_tangents[direction] - minus_tangents[direction]) / (2 * epsilon);
    for (const auto component : make_range(Moose::dim))
      EXPECT_NEAR(ad_tangents[direction](component).derivatives()[derivative_index],
                  finite_difference(component),
                  finite_difference_tolerance);
  }
  EXPECT_NEAR(
      MetaPhysicL::raw_value(ad_tangents[0] * ad_tangents[1]), 0.0, orthogonality_tolerance);

  // Repeat the same orthonormality and finite-difference checks along a curve through the pole
  // (-1, 0, 0), where the primary Householder chart (h = n + e_x) is singular and reflection
  // hands off to the complementary chart (h = n - e_x). The chart handover must not break
  // orthonormality or differentiability, unlike the constant-fallback it replaced.
  constexpr Real singular_coordinate = 0.0;
  const auto singular_tangents_at = [](const auto & perturbed_coordinate)
  {
    using T = std::decay_t<decltype(perturbed_coordinate)>;
    const libMesh::VectorValue<T> normal(
        -sqrt(1.0 - perturbed_coordinate * perturbed_coordinate), perturbed_coordinate, T(0.0));
    return std::make_pair(normal, Moose::Mortar::householderTangents(normal));
  };

  ADReal ad_singular_coordinate = singular_coordinate;
  Moose::derivInsert(ad_singular_coordinate.derivatives(), derivative_index, 1.0);
  const auto [ad_singular_normal, ad_singular_tangents] =
      singular_tangents_at(ad_singular_coordinate);
  const auto [plus_singular_normal, plus_singular_tangents] =
      singular_tangents_at(singular_coordinate + epsilon);
  const auto [minus_singular_normal, minus_singular_tangents] =
      singular_tangents_at(singular_coordinate - epsilon);

  const auto singular_normal_finite_difference =
      (plus_singular_normal - minus_singular_normal) / (2 * epsilon);
  for (const auto component : make_range(Moose::dim))
    EXPECT_NEAR(ad_singular_normal(component).derivatives()[derivative_index],
                singular_normal_finite_difference(component),
                finite_difference_tolerance);

  for (const auto direction : make_range(2))
  {
    EXPECT_NEAR(MetaPhysicL::raw_value(ad_singular_normal * ad_singular_tangents[direction]),
                0.0,
                orthogonality_tolerance);
    EXPECT_NEAR(MetaPhysicL::raw_value(ad_singular_tangents[direction].norm()),
                1.0,
                orthogonality_tolerance);

    const auto singular_finite_difference =
        (plus_singular_tangents[direction] - minus_singular_tangents[direction]) / (2 * epsilon);
    for (const auto component : make_range(Moose::dim))
      EXPECT_NEAR(ad_singular_tangents[direction](component).derivatives()[derivative_index],
                  singular_finite_difference(component),
                  finite_difference_tolerance);
  }
  EXPECT_NEAR(MetaPhysicL::raw_value(ad_singular_tangents[0] * ad_singular_tangents[1]),
              0.0,
              orthogonality_tolerance);
}
