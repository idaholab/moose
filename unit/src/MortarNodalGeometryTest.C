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

TEST(MortarNodalGeometryTest, householderTangents)
{
  constexpr dof_id_type derivative_index = 0;
  constexpr Real epsilon = 1e-7;
  constexpr Real coordinate = 0.4;

  const auto tangents = [](const auto & perturbed_coordinate)
  {
    using T = std::decay_t<decltype(perturbed_coordinate)>;
    const libMesh::VectorValue<T> normal =
        libMesh::VectorValue<T>(perturbed_coordinate, -0.3, 0.8).unit();
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
                1e-8);

  for (const auto direction : make_range(2))
  {
    EXPECT_NEAR(MetaPhysicL::raw_value(ad_normal * ad_tangents[direction]), 0.0, 1e-14);
    EXPECT_NEAR(MetaPhysicL::raw_value(ad_tangents[direction].norm()), 1.0, 1e-14);

    const auto finite_difference =
        (plus_tangents[direction] - minus_tangents[direction]) / (2 * epsilon);
    for (const auto component : make_range(Moose::dim))
      EXPECT_NEAR(ad_tangents[direction](component).derivatives()[derivative_index],
                  finite_difference(component),
                  1e-8);
  }
  EXPECT_NEAR(MetaPhysicL::raw_value(ad_tangents[0] * ad_tangents[1]), 0.0, 1e-14);

  ADRealVectorValue singular_normal(-1.0, 0.0, 0.0);
  Moose::derivInsert(singular_normal(1).derivatives(), derivative_index, 1.0);
  const auto singular_tangents = Moose::Mortar::householderTangents(singular_normal);
  for (const auto & tangent : singular_tangents)
    for (const auto component : make_range(Moose::dim))
      EXPECT_EQ(tangent(component).derivatives()[derivative_index], 0.0);
}
