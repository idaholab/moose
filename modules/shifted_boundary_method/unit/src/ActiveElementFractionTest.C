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

#include "libmesh/serial_mesh.h"
#include "libmesh/face_quad4.h"
#include "libmesh/enum_order.h"

using namespace libMesh;

// activeElementFraction integrates a predicate over the element with quadrature and returns the
// active fraction of the total measure. On the unit square, a symmetric Gauss rule splits an
// x < 0.5 half-space predicate exactly in two, and the all/none predicates give the exact
// endpoints one/zero.
TEST(ActiveElementFractionTest, UnitSquare)
{
  Parallel::Communicator comm(MPI_COMM_SELF);
  auto mesh = std::make_unique<SerialMesh>(comm);

  Node * n0 = mesh->add_point(Point(0.0, 0.0, 0.0), 0);
  Node * n1 = mesh->add_point(Point(1.0, 0.0, 0.0), 1);
  Node * n2 = mesh->add_point(Point(1.0, 1.0, 0.0), 2);
  Node * n3 = mesh->add_point(Point(0.0, 1.0, 0.0), 3);

  auto quad = new Quad4();
  quad->set_node(0, n0);
  quad->set_node(1, n1);
  quad->set_node(2, n2);
  quad->set_node(3, n3);
  const Elem * elem = mesh->add_elem(quad);

  mesh->prepare_for_use();

  const auto all_active = [](const Point &) { return true; };
  const auto none_active = [](const Point &) { return false; };
  const auto left_half = [](const Point & p) { return p(0) < 0.5; };

  // SECOND-order Gauss has 2 points per dimension, symmetric about the centre with none on it.
  EXPECT_NEAR(SBMUtils::activeElementFraction(*elem, SECOND, all_active), 1.0, 1e-12);
  EXPECT_NEAR(SBMUtils::activeElementFraction(*elem, SECOND, none_active), 0.0, 1e-12);
  EXPECT_NEAR(SBMUtils::activeElementFraction(*elem, SECOND, left_half), 0.5, 1e-12);
}
