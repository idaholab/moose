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

  const auto fully_occupied = SBMUtils::elementDomainOccupancy(*elem, SECOND, all_active);
  EXPECT_TRUE(fully_occupied.all_nodes_in_domain);
  EXPECT_FALSE(fully_occupied.all_nodes_outside_domain);
  EXPECT_NEAR(fully_occupied.domain_fraction, 1.0, 1e-12);

  const auto unoccupied = SBMUtils::elementDomainOccupancy(*elem, SECOND, none_active);
  EXPECT_FALSE(unoccupied.all_nodes_in_domain);
  EXPECT_TRUE(unoccupied.all_nodes_outside_domain);
  EXPECT_NEAR(unoccupied.domain_fraction, 0.0, 1e-12);

  const auto partially_occupied = SBMUtils::elementDomainOccupancy(*elem, SECOND, left_half);
  EXPECT_FALSE(partially_occupied.all_nodes_in_domain);
  EXPECT_FALSE(partially_occupied.all_nodes_outside_domain);
  EXPECT_NEAR(partially_occupied.domain_fraction, 0.5, 1e-12);
}

// Nodal and quadrature occupancy are independent measurements. This predicate excludes every
// corner of the unit square while including every SECOND-order quadrature point.
TEST(ElementDomainOccupancyTest, DomainEnclosedWithinElement)
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

  const auto centered_domain = [](const Point & p)
  {
    const Real x = p(0) - 0.5;
    const Real y = p(1) - 0.5;
    return x * x + y * y < 0.45 * 0.45;
  };

  const auto occupancy = SBMUtils::elementDomainOccupancy(*elem, SECOND, centered_domain);
  EXPECT_FALSE(occupancy.all_nodes_in_domain);
  EXPECT_TRUE(occupancy.all_nodes_outside_domain);
  EXPECT_NEAR(occupancy.domain_fraction, 1.0, 1e-12);

  const auto signed_value = [](const Point & p)
  { return p(0) * (1.0 - p(0)) * p(1) * (1.0 - p(1)); };
  const auto is_in_domain = [&signed_value](const Point & p) { return signed_value(p) > 0.0; };
  const auto is_outside_domain = [&signed_value](const Point & p) { return signed_value(p) < 0.0; };

  const auto boundary_occupancy =
      SBMUtils::elementDomainOccupancy(*elem, SECOND, is_in_domain, is_outside_domain);
  EXPECT_FALSE(boundary_occupancy.all_nodes_in_domain);
  EXPECT_FALSE(boundary_occupancy.all_nodes_outside_domain);
  EXPECT_NEAR(boundary_occupancy.domain_fraction, 1.0, 1e-12);
}
