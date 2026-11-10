//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MortarUtils.h"

#include "libmesh/elem.h"
#include "libmesh/quadrature_gauss.h"

#include <vector>

namespace
{
// Roundoff allowance for values of order one
constexpr Real tol = 1e-12;

std::vector<Point>
samplePoints(const ElemType type)
{
  if (type == EDGE3)
    return {Point(-0.9), Point(-0.3), Point(0.2), Point(0.7)};
  if (type == TRI6)
    return {Point(0.1, 0.1), Point(0.3, 0.3), Point(0.6, 0.1), Point(0.1, 0.7), Point(0.2, 0.45)};
  return {Point(-0.8, -0.7),
          Point(0.1, -0.2),
          Point(0.6, 0.3),
          Point(-0.4, 0.8),
          Point(0.9, -0.85),
          Point(0.3, 0.6)};
}
}

TEST(MortarPiecewiseLinearShapesTest, InterpolatesNodesAndPartitionsUnity)
{
  for (const auto type : {EDGE3, TRI6, QUAD8, QUAD9})
  {
    const auto elem = Elem::build(type);
    std::vector<Real> phi;

    for (const auto node : make_range(elem->n_nodes()))
    {
      Moose::Mortar::piecewiseLinearShapes(*elem, elem->master_point(node), phi);
      for (const auto i : index_range(phi))
        EXPECT_NEAR(phi[i], i == node ? 1 : 0, tol) << "type " << type << " node " << node;
    }

    for (const auto & point : samplePoints(type))
    {
      Moose::Mortar::piecewiseLinearShapes(*elem, point, phi);
      Real sum = 0;
      for (const auto i : index_range(phi))
      {
        EXPECT_GE(phi[i], -tol);
        sum += phi[i];
      }
      EXPECT_NEAR(sum, 1, tol);
    }
  }
}

TEST(MortarPiecewiseLinearShapesTest, IntegratesBasisFunctions)
{
  for (const auto type : {EDGE3, TRI6, QUAD8, QUAD9})
  {
    const auto elem = Elem::build(type);
    std::vector<Real> integrals(elem->n_nodes(), 0);
    std::vector<Real> phi;

    if (type == EDGE3)
    {
      const auto sub_elem = Elem::build(EDGE2);
      QGauss qrule(1, SECOND);
      qrule.init(*sub_elem);
      for (const Real center : {-0.5, 0.5})
        for (const auto qp : make_range(qrule.n_points()))
        {
          Moose::Mortar::piecewiseLinearShapes(*elem, Point(center + 0.5 * qrule.qp(qp)(0)), phi);
          for (const auto i : index_range(phi))
            integrals[i] += 0.5 * qrule.w(qp) * phi[i];
        }
    }
    else
      for (const auto sub_elem_id : make_range(elem->n_sub_elem()))
      {
        const auto nodes = Moose::Mortar::getMortarSubElementNodeIndices(*elem, sub_elem_id);
        const bool triangle = nodes.size() == 3;
        const auto sub_elem = Elem::build(triangle ? TRI3 : QUAD4);
        QGauss qrule(2, SECOND);
        qrule.init(*sub_elem);

        const Point p0 = elem->master_point(nodes[0]);
        const Point origin = triangle ? p0 : Point(0.5 * (p0 + elem->master_point(nodes[2])));
        const Real scale = triangle ? 1 : 0.5;
        const Point e0 = scale * (elem->master_point(nodes[1]) - p0);
        const Point e1 = scale * (elem->master_point(nodes.back()) - p0);
        const Real determinant = std::abs(e0(0) * e1(1) - e0(1) * e1(0));

        for (const auto qp : make_range(qrule.n_points()))
        {
          const Point point = origin + qrule.qp(qp)(0) * e0 + qrule.qp(qp)(1) * e1;
          Moose::Mortar::piecewiseLinearShapes(*elem, point, phi);
          for (const auto i : index_range(phi))
            integrals[i] += determinant * qrule.w(qp) * phi[i];
        }
      }

    std::vector<Real> expected;
    switch (type)
    {
      case EDGE3:
        expected = {0.5, 0.5, 1};
        break;
      case TRI6:
        expected = {1. / 24, 1. / 24, 1. / 24, 1. / 8, 1. / 8, 1. / 8};
        break;
      case QUAD8:
        expected = {1. / 6, 1. / 6, 1. / 6, 1. / 6, 5. / 6, 5. / 6, 5. / 6, 5. / 6};
        break;
      case QUAD9:
        expected = {0.25, 0.25, 0.25, 0.25, 0.5, 0.5, 0.5, 0.5, 1};
        break;
      default:
        FAIL() << "Unexpected element type " << type;
    }

    for (const auto i : index_range(integrals))
      EXPECT_NEAR(integrals[i], expected[i], tol) << "type " << type << " node " << i;
  }
}
