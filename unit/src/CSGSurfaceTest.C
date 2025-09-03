//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CSGPlane.h"
#include "CSGSphere.h"
#include "CSGXCylinder.h"
#include "CSGYCylinder.h"
#include "CSGZCylinder.h"

#include "MooseUnitUtils.h"

using namespace CSG;

/// Tests CSGPlane::CSGPlane constructors and methods
TEST(CSGSurfaceTest, testCSGPlane)
{
  // expected coefficients: a=-1, b=0, c=0, d=2.0
  std::unordered_map<std::string, Real> exp_coeffs = {
      {"a", -1.0}, {"b", 0.0}, {"c", 0.0}, {"d", 2.0}};
  // expected surface type string
  std::string exp_type = "CSG::CSGPlane";

  // Construct plane from points: plane at x=-2
  {
    const std::array<Point, 3> points{Point(-2, 0, 0), Point(-2, 1, 0), Point(-2, 0, 1)};
    CSGPlane plane("plane_surf", points[0], points[1], points[2]);
    ASSERT_EQ(exp_coeffs, plane.getCoeffs());
    ASSERT_EQ(exp_type, plane.getSurfaceType());
  }
  // Construct plane from coefficients
  {
    // a=-1.0, b=0.0, c=0.0, d=2.0
    CSGPlane plane("plane_surf", -1.0, 0.0, 0.0, 2.0);
    ASSERT_EQ(exp_coeffs, plane.getCoeffs());
    ASSERT_EQ(exp_type, plane.getSurfaceType());
  }
  // Collinear points; error
  {
    const std::array<Point, 3> points{Point(0, 0, 0), Point(1, 0, 0), Point(2, 0, 0)};
    Moose::UnitUtils::assertThrows(
        [&points]() { CSGPlane plane("plane_surf", points[0], points[1], points[2]); },
        "Provided points to define a CSGPlane are collinear");
  }
  // evaluateSurfaceEquationAtPoint
  {
    CSGPlane plane("plane_surf", 1.0, 0.0, 0.0, 2.0);
    const Point p = Point(3.0, 0.0, 0.0);
    ASSERT_FLOAT_EQ(1.0, plane.evaluateSurfaceEquationAtPoint(p));
  }
}

/// Tests CSGSphere::CSGSphere constructors and methods
TEST(CSGSurfaceTest, testCSGSphere)
{
  // expected surface type string
  std::string exp_type = "CSG::CSGSphere";

  // Construct sphere at specified center point
  {
    std::unordered_map<std::string, Real> exp_coeffs = {
        {"x0", 1.0}, {"y0", 2.0}, {"z0", 3.0}, {"r", 4.0}};
    Point center = Point(1, 2, 3);
    Real radius = 4.0;
    CSGSphere sphere("sphere_surf", center, radius);
    ASSERT_EQ(exp_coeffs, sphere.getCoeffs());
    ASSERT_EQ(exp_type, sphere.getSurfaceType());
  }
  // Construct sphere at origin
  {
    std::unordered_map<std::string, Real> exp_coeffs = {
        {"x0", 0.0}, {"y0", 0.0}, {"z0", 0.0}, {"r", 4.0}};
    Real radius = 4.0;
    CSGSphere sphere("sphere_surf", radius);
    ASSERT_EQ(exp_coeffs, sphere.getCoeffs());
    ASSERT_EQ(exp_type, sphere.getSurfaceType());
  }
  // negative radius; error
  {
    Real radius = -1.0;
    Moose::UnitUtils::assertThrows([&radius]() { CSGSphere sphere("sphere_surf", radius); },
                                   "Radius of sphere must be positive.");
  }
  // evaluateSurfaceEquationAtPoint
  {
    Real radius = 4.0;
    CSGSphere sphere("sphere_surf", radius);
    const Point p = Point(2.0, 0.0, 0.0);
    ASSERT_FLOAT_EQ(-12.0, sphere.evaluateSurfaceEquationAtPoint(p));
  }
}

/// Tests CSG[X/Y/Z]Cylinder::CSG[X/Y/Z]Cylinder constructors and methods
TEST(CSGSurfaceTest, testCSGCylinders)
{
  // center and radius to use for all axially aligned cylinders
  Real c0 = 1.0;
  Real c1 = 2.0;
  Real radius = 3.0;

  // Construct each cylinder
  CSGXCylinder xcyl("xcyl", c0, c1, radius);
  CSGYCylinder ycyl("ycyl", c0, c1, radius);
  CSGZCylinder zcyl("zcyl", c0, c1, radius);

  // XCylinder coefficients
  {
    std::unordered_map<std::string, Real> exp_coeffs = {{"y0", 1.0}, {"z0", 2.0}, {"r", 3.0}};
    ASSERT_EQ(exp_coeffs, xcyl.getCoeffs());
    ASSERT_EQ("CSG::CSGXCylinder", xcyl.getSurfaceType());
  }
  // YCylinder coefficients
  {
    std::unordered_map<std::string, Real> exp_coeffs = {{"x0", 1.0}, {"z0", 2.0}, {"r", 3.0}};
    ASSERT_EQ(exp_coeffs, ycyl.getCoeffs());
    ASSERT_EQ("CSG::CSGYCylinder", ycyl.getSurfaceType());
  }
  // ZCylinder coefficients
  {
    std::unordered_map<std::string, Real> exp_coeffs = {{"x0", 1.0}, {"y0", 2.0}, {"r", 3.0}};
    ASSERT_EQ(exp_coeffs, zcyl.getCoeffs());
    ASSERT_EQ("CSG::CSGZCylinder", zcyl.getSurfaceType());
  }

  // negative radius; error
  {
    Real radius = -1.0;
    Moose::UnitUtils::assertThrows([&c0, &c1, &radius]()
                                   { CSGXCylinder xcyl("cyl", c0, c1, radius); },
                                   "Radius of x-cylinder must be positive.");
    Moose::UnitUtils::assertThrows([&c0, &c1, &radius]()
                                   { CSGYCylinder ycyl("cyl", c0, c1, radius); },
                                   "Radius of y-cylinder must be positive.");
    Moose::UnitUtils::assertThrows([&c0, &c1, &radius]()
                                   { CSGZCylinder zcyl("cyl", c0, c1, radius); },
                                   "Radius of z-cylinder must be positive.");
  }
  // evaluateSurfaceEquationAtPoint
  {
    const Point px = Point(2.0, 0.0, 0.0);
    ASSERT_FLOAT_EQ(-4.0, xcyl.evaluateSurfaceEquationAtPoint(px));
    const Point py = Point(0.0, 2.0, 0.0);
    ASSERT_FLOAT_EQ(-4.0, ycyl.evaluateSurfaceEquationAtPoint(py));
    const Point pz = Point(0.0, 0.0, 2.0);
    ASSERT_FLOAT_EQ(-4.0, zcyl.evaluateSurfaceEquationAtPoint(pz));
  }
}

/// Tests CSGSurface::getHalfspaceFromPoint
TEST(CSGSurfaceTest, testGetHalfspaceFromPoint)
{
  // Surface in the +x direction
  const std::array<Point, 3> points{Point(1, 0, 0), Point(1, 1, 0), Point(1, 0, 1)};
  CSGPlane plane("plus_x", points[0], points[1], points[2]);

  // Centroid of the points
  Point centroid;
  for (const auto & point : points)
    centroid += point;
  centroid /= 3;

  // Positive halfspace
  {
    const auto point = centroid + Point(-1, 0, 0);
    ASSERT_EQ(CSGSurface::Halfspace::POSITIVE, plane.getHalfspaceFromPoint(point));
  }
  // Negative halfspace
  {
    const auto point = centroid + Point(1, 0, 0);
    ASSERT_EQ(CSGSurface::Halfspace::NEGATIVE, plane.getHalfspaceFromPoint(point));
  }
  // Lies on the plane; error
  {
    Moose::UnitUtils::assertThrows([&centroid, &plane]() { plane.getHalfspaceFromPoint(centroid); },
                                   "ambiguously defined halfspace");
  }
}

/// tests surface equality operators
TEST(CSGSurfaceTest, testSurfaceEquality)
{
  // make two identical surfaces and check they register as equal
  Real radius = 1.0;
  CSGSphere sphere1("sphere_surf", radius);
  CSGSphere sphere2("sphere_surf", radius);
  ASSERT_TRUE(sphere1 == sphere2);

  // make spheres that differ from sphere1 by one characteristic and check
  // that each registers as not equal to sphere1
  std::array<CSGSphere, 3> sphs{CSGSphere("new_name", radius),
                                CSGSphere("sphere_surf", 2.0),
                                CSGSphere("sphere_surf", Point(1, 2, 3), radius)};
  for (auto surf : sphs)
  {
    ASSERT_TRUE(sphere1 != surf);
  }
}

/// test CSGSurface::getName
TEST(CSGSurfaceTest, testSurfaceGetName)
{
  std::string name = "sph_surf_name";
  CSGSphere sph(name, 1.0);
  ASSERT_EQ(name, sph.getName());
}
