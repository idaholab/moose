//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"
#include "predicates.h"

namespace
{

// The magnitude a predicate returns is only an approximation of the determinant, so the tests below
// compare signs and never values.
int
predicateSign(const double value)
{
  if (value > 0.0)
    return 1;
  if (value < 0.0)
    return -1;
  return 0;
}

} // namespace

/**
 * moose_orient2d() is exactly zero on collinear input, where the textbook determinant is not.
 *
 * The three points lie on the line y = 3 * x, so the determinant is exactly zero. Every coordinate
 * is an integer that a double holds exactly, but the differences from pa land just above 2^53,
 * where consecutive doubles are more than one apart, so three of the four of them round. That
 * rounding is not symmetric between the two products, and the textbook determinant comes out on the
 * order of 2^54 instead of zero.
 *
 * This case is the regression evidence for the -ffp-contract=off rule that framework/build.mk
 * applies to predicates.c: without it the adaptive arithmetic silently stops being exact and
 * moose_orient2d() reports a nonzero determinant here.
 */
TEST(GeometricPredicatesTest, collinearIsExactlyZero)
{
  initPredicates();

  // The volatile intermediates force each operation to round to double exactly once, no matter
  // how this test file itself is compiled: a compiler defaulting to fast math (Intel's icx) or
  // contracting into FMA (GCC and Clang on aarch64) could otherwise fold or fuse the textbook
  // determinant into something more exact, and the point of this control is its rounding.
  auto naive_orient2d = [](const double * pa, const double * pb, const double * pc)
  {
    volatile double dbx = pb[0] - pa[0];
    volatile double dby = pb[1] - pa[1];
    volatile double dcx = pc[0] - pa[0];
    volatile double dcy = pc[1] - pa[1];
    volatile double left = dbx * dcy;
    volatile double right = dby * dcx;
    return left - right;
  };

  const double two_53 = 9007199254740992.0;
  const double two_52 = 4503599627370496.0;
  const double pa[2] = {-1.0, -3.0};
  const double pb[2] = {two_53, 3.0 * two_53};
  const double pc[2] = {two_52, 3.0 * two_52};

  EXPECT_NE(0.0, naive_orient2d(pa, pb, pc));
  EXPECT_EQ(0.0, moose_orient2d(pa, pb, pc));

  // Zero has no sign, so the argument order cannot change the answer.
  EXPECT_EQ(0.0, moose_orient2d(pb, pa, pc));
  EXPECT_EQ(0.0, moose_orient2d(pc, pb, pa));

  // The same line at a scale where the textbook determinant is exact as well.
  const double small_a[2] = {0.0, 0.0};
  const double small_b[2] = {1.0, 3.0};
  const double small_c[2] = {2.0, 6.0};
  EXPECT_EQ(0.0, naive_orient2d(small_a, small_b, small_c));
  EXPECT_EQ(0.0, moose_orient2d(small_a, small_b, small_c));
}

/**
 * The contract the mesh generator relies on: positive for counter-clockwise input, negative for
 * clockwise input.
 */
TEST(GeometricPredicatesTest, orientationSign)
{
  initPredicates();

  const double pa[2] = {-0.75, 0.25};
  const double pb[2] = {2.5, -1.0};
  const double pc[2] = {0.125, 3.0};

  EXPECT_GT(moose_orient2d(pa, pb, pc), 0.0);
  EXPECT_LT(moose_orient2d(pa, pc, pb), 0.0);
}

/**
 * moose_incircle() is exactly zero on cocircular input, and flips sign across the circle.
 *
 * Both quadruples have integer coordinates, so the in-circle determinant is an integer that is
 * exactly zero. The radius itself need not be representable, as the square case shows.
 */
TEST(GeometricPredicatesTest, cocircularIsExactlyZero)
{
  initPredicates();

  // The four axis crossings of the circle of radius 4 centered at the origin.
  const double radius = 4.0;
  const double east[2] = {radius, 0.0};
  const double north[2] = {0.0, radius};
  const double west[2] = {-radius, 0.0};
  const double south[2] = {0.0, -radius};

  ASSERT_GT(moose_orient2d(east, north, west), 0.0);
  EXPECT_EQ(0.0, moose_incircle(east, north, west, south));

  const double delta = 1.0 / 1024.0;
  const double south_inside[2] = {0.0, -radius + delta};
  const double south_outside[2] = {0.0, -radius - delta};
  EXPECT_GT(moose_incircle(east, north, west, south_inside), 0.0);
  EXPECT_LT(moose_incircle(east, north, west, south_outside), 0.0);

  // The four corners of a square are cocircular on a circle of radius sqrt(2).
  const double sw[2] = {-1.0, -1.0};
  const double se[2] = {1.0, -1.0};
  const double ne[2] = {1.0, 1.0};
  const double nw[2] = {-1.0, 1.0};

  ASSERT_GT(moose_orient2d(sw, se, ne), 0.0);
  EXPECT_EQ(0.0, moose_incircle(sw, se, ne, nw));

  const double center[2] = {0.0, 0.0};
  const double corner_outside[2] = {-2.0, 2.0};
  EXPECT_GT(moose_incircle(sw, se, ne, center), 0.0);
  EXPECT_LT(moose_incircle(sw, se, ne, corner_outside), 0.0);
}

/**
 * A cyclic rotation of moose_orient2d()'s arguments preserves the sign, and swapping any two of
 * them negates it.
 */
TEST(GeometricPredicatesTest, orient2dPermutationAntisymmetry)
{
  initPredicates();

  const double pa[2] = {-0.75, 0.25};
  const double pb[2] = {2.5, -1.0};
  const double pc[2] = {0.125, 3.0};

  const int ccw = predicateSign(moose_orient2d(pa, pb, pc));
  ASSERT_EQ(1, ccw);

  EXPECT_EQ(ccw, predicateSign(moose_orient2d(pb, pc, pa)));
  EXPECT_EQ(ccw, predicateSign(moose_orient2d(pc, pa, pb)));

  EXPECT_EQ(-ccw, predicateSign(moose_orient2d(pb, pa, pc)));
  EXPECT_EQ(-ccw, predicateSign(moose_orient2d(pa, pc, pb)));
  EXPECT_EQ(-ccw, predicateSign(moose_orient2d(pc, pb, pa)));
}

/**
 * Swapping two of moose_incircle()'s first three arguments reverses the counter-clockwise order
 * they must be in, and so negates the sign; a cyclic rotation of them preserves it.
 */
TEST(GeometricPredicatesTest, incirclePermutationAntisymmetry)
{
  initPredicates();

  const double pa[2] = {0.0, 0.0};
  const double pb[2] = {1.0, 0.0};
  const double pc[2] = {0.0, 1.0};
  const double pd[2] = {0.25, 0.25};

  ASSERT_GT(moose_orient2d(pa, pb, pc), 0.0);

  const int inside = predicateSign(moose_incircle(pa, pb, pc, pd));
  ASSERT_EQ(1, inside);

  EXPECT_EQ(inside, predicateSign(moose_incircle(pb, pc, pa, pd)));
  EXPECT_EQ(inside, predicateSign(moose_incircle(pc, pa, pb, pd)));

  EXPECT_EQ(-inside, predicateSign(moose_incircle(pb, pa, pc, pd)));
  EXPECT_EQ(-inside, predicateSign(moose_incircle(pa, pc, pb, pd)));
  EXPECT_EQ(-inside, predicateSign(moose_incircle(pc, pb, pa, pd)));
}
