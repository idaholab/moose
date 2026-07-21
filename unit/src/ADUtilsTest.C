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
#include "MooseLagrangeHelpers.h"

#include <type_traits>
#include <vector>

TEST(ADUtilsTest, faceAreaVector)
{
  struct FaceCase
  {
    libMesh::ElemType type;
    libMesh::Order order;
    std::vector<libMesh::Point> nodes;
    libMesh::Point reference_point;
    unsigned int perturbed_node;
    unsigned int perturbed_component;
  };

  const std::vector<FaceCase> cases{
      {libMesh::EDGE2, libMesh::FIRST, {{-1.0, 0.1, 0.0}, {1.0, 0.8, 0.0}}, {0.2, 0.0, 0.0}, 1, 1},
      {libMesh::EDGE3,
       libMesh::SECOND,
       {{-1.0, 0.1, 0.0}, {1.0, 0.8, 0.0}, {0.0, 0.7, 0.0}},
       {0.3, 0.0, 0.0},
       2,
       1},
      {libMesh::TRI3,
       libMesh::FIRST,
       {{0.0, 0.0, 0.0}, {1.1, 0.0, 0.1}, {0.1, 1.0, 0.2}},
       {0.2, 0.3, 0.0},
       2,
       2},
      {libMesh::QUAD4,
       libMesh::FIRST,
       {{-1.0, -1.0, 0.0}, {1.0, -1.0, 0.1}, {1.0, 1.0, 0.3}, {-1.0, 1.0, -0.1}},
       {0.2, -0.3, 0.0},
       2,
       2},
      {libMesh::QUAD9,
       libMesh::SECOND,
       {{-1.0, -1.0, 0.0},
        {1.0, -1.0, 0.1},
        {1.0, 1.0, 0.3},
        {-1.0, 1.0, -0.1},
        {0.0, -1.0, 0.2},
        {1.0, 0.0, 0.35},
        {0.0, 1.0, 0.25},
        {-1.0, 0.0, 0.05},
        {0.0, 0.0, 0.4}},
       {0.23, -0.31, 0.0},
       8,
       2}};

  constexpr dof_id_type derivative_index = 0;
  constexpr Real epsilon = 1e-7;

  const auto normal = [](const FaceCase & face, const auto & perturbed_coordinate)
  {
    using T = std::decay_t<decltype(perturbed_coordinate)>;
    libMesh::VectorValue<T> tangent_xi;
    libMesh::VectorValue<T> tangent_eta;

    for (const auto node : index_range(face.nodes))
    {
      libMesh::VectorValue<T> coordinate(face.nodes[node]);
      if (node == face.perturbed_node)
        coordinate(face.perturbed_component) = perturbed_coordinate;

      if (face.type == libMesh::EDGE2 || face.type == libMesh::EDGE3)
        tangent_xi.add_scaled(
            coordinate,
            Moose::fe_lagrange_1D_shape_deriv(face.order, node, face.reference_point(0)));
      else
      {
        tangent_xi.add_scaled(coordinate,
                              Moose::fe_lagrange_2D_shape_deriv(
                                  face.type, face.order, node, 0, face.reference_point));
        tangent_eta.add_scaled(coordinate,
                               Moose::fe_lagrange_2D_shape_deriv(
                                   face.type, face.order, node, 1, face.reference_point));
      }
    }

    if (face.type == libMesh::EDGE2 || face.type == libMesh::EDGE3)
      return Moose::faceAreaVector(tangent_xi).unit();
    return Moose::faceAreaVector(tangent_xi, tangent_eta).unit();
  };

  for (const auto & face : cases)
  {
    const auto coordinate = face.nodes[face.perturbed_node](face.perturbed_component);
    ADReal ad_coordinate = coordinate;
    Moose::derivInsert(ad_coordinate.derivatives(), derivative_index, 1.0);
    const auto ad_normal = normal(face, ad_coordinate);

    const auto plus_normal = normal(face, coordinate + epsilon);
    const auto minus_normal = normal(face, coordinate - epsilon);
    const auto finite_difference = (plus_normal - minus_normal) / (2 * epsilon);

    EXPECT_NEAR(MetaPhysicL::raw_value(ad_normal.norm()), 1.0, 1e-14);
    for (const auto component : make_range(Moose::dim))
      EXPECT_NEAR(
          ad_normal(component).derivatives()[derivative_index], finite_difference(component), 1e-8);
  }
}

TEST(ADUtilsTest, mortarHouseholderTangents)
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
