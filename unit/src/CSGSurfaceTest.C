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

namespace CSG
{

// helper function for creating axis aligned cylinders
template <typename T>
T
makeCylinder()
{
  Real c0 = 1.0;
  Real c1 = 2.0;
  Real radius = 3.0;
  T cylinder("cyl", c0, c1, radius);
  return cylinder;
}

// helper function for checking axis aligned cylinders
void
checkCylinder(std::string axis,
              std::unordered_map<std::string, Real> test_coeffs,
              std::string test_type)
{
  Real c0 = 1.0;
  Real c1 = 2.0;
  Real radius = 3.0;

  std::string k0, k1;
  if (axis == "X")
  {
    k0 = "y0";
    k1 = "z0";
  }
  else if (axis == "Y")
  {
    k0 = "x0";
    k1 = "z0";
  }
  else if (axis == "Z")
  {
    k0 = "x0";
    k1 = "y0";
  }
  std::unordered_map<std::string, Real> exp_coeffs = {{k0, c0}, {k1, c1}, {"r", radius}};
  std::string exp_type = "CSG::CSG" + axis + "Cylinder";
  ASSERT_TRUE(exp_coeffs == test_coeffs);
  ASSERT_TRUE(exp_type == test_type);
}

/// Tests CSGPlane::CSGPlane constructors, from 3 points
TEST(CSGSurfaceTest, testPlaneFromPoints)
{
  // expected coefficients: a=-1, b=0, c=0, d=2.0
  std::unordered_map<std::string, Real> exp_coeffs = {
      {"a", -1.0}, {"b", 0.0}, {"c", 0.0}, {"d", 2.0}};
  // expected surface type string
  std::string exp_type = "CSG::CSGPlane";

  const std::array<Point, 3> points{Point(-2, 0, 0), Point(-2, 1, 0), Point(-2, 0, 1)};
  CSGPlane plane("plane_surf", points[0], points[1], points[2]);
  ASSERT_EQ(exp_coeffs, plane.getCoeffs());
  ASSERT_EQ(exp_type, plane.getSurfaceType());
}

/// Tests CSGPlane::CSGPlane constructors, from coefficients
TEST(CSGSurfaceTest, testPlaneFromCoeffs)
{
  // expected coefficients: a=-1, b=0, c=0, d=2.0
  std::unordered_map<std::string, Real> exp_coeffs = {
      {"a", -1.0}, {"b", 0.0}, {"c", 0.0}, {"d", 2.0}};
  // expected surface type string
  std::string exp_type = "CSG::CSGPlane";

  // a=-1.0, b=0.0, c=0.0, d=2.0
  CSGPlane plane("plane_surf", -1.0, 0.0, 0.0, 2.0);
  ASSERT_EQ(exp_coeffs, plane.getCoeffs());
  ASSERT_EQ(exp_type, plane.getSurfaceType());
}

/// Tests CSGPlane::CSGPlane, error if points are collinear
TEST(CSGSurfaceTest, testPlaneCollinearPoints)
{
  // three collinear points, expect error from constructor check
  const std::array<Point, 3> points{Point(0, 0, 0), Point(1, 0, 0), Point(2, 0, 0)};
  Moose::UnitUtils::assertThrows([&points]()
                                 { CSGPlane plane("plane_surf", points[0], points[1], points[2]); },
                                 "Provided points to define a CSGPlane are collinear");
}

/// tests CSGPlane::evaluateSurfaceEquationAtPoint
TEST(CSGSurfaceTest, testPlaneEvaluateEq)
{
  CSGPlane plane("plane_surf", 1.0, 0.0, 0.0, 2.0);
  const Point p = Point(3.0, 0.0, 0.0);
  ASSERT_FLOAT_EQ(1.0, plane.evaluateSurfaceEquationAtPoint(p));
}

/// Tests CSGSphere::CSGSphere constructor, at specified center point
TEST(CSGSurfaceTest, testSphereAtPoint)
{
  // expected surface type string
  std::string exp_type = "CSG::CSGSphere";
  std::unordered_map<std::string, Real> exp_coeffs = {
      {"x0", 1.0}, {"y0", 2.0}, {"z0", 3.0}, {"r", 4.0}};

  // Construct sphere at specified center point
  Point center = Point(1, 2, 3);
  Real radius = 4.0;
  CSGSphere sphere("sphere_surf", center, radius);

  ASSERT_EQ(exp_coeffs, sphere.getCoeffs());
  ASSERT_EQ(exp_type, sphere.getSurfaceType());
}

/// Tests CSGSphere::CSGSphere constructor, at origin
TEST(CSGSurfaceTest, testphereAtOrigin)
{
  // expected surface type string
  std::string exp_type = "CSG::CSGSphere";

  // Construct sphere at origin
  std::unordered_map<std::string, Real> exp_coeffs = {
      {"x0", 0.0}, {"y0", 0.0}, {"z0", 0.0}, {"r", 4.0}};
  Real radius = 4.0;
  CSGSphere sphere("sphere_surf", radius);

  ASSERT_EQ(exp_coeffs, sphere.getCoeffs());
  ASSERT_EQ(exp_type, sphere.getSurfaceType());
}

/// Tests CSGSphere::evaluateSurfaceEquationAtPoint
TEST(CSGSurfaceTest, testSphereEvaluateEq)
{
  Real radius = 4.0;
  CSGSphere sphere("sphere_surf", radius);
  const Point p = Point(2.0, 0.0, 0.0);
  ASSERT_FLOAT_EQ(-12.0, sphere.evaluateSurfaceEquationAtPoint(p));
}

/// Tests error is raised during construction for negative radius
TEST(CSGSurfaceTest, testSphereNegativeRadius)
{
  Real radius = -1.0;
  Moose::UnitUtils::assertThrows([&radius]() { CSGSphere sphere("sphere_surf", radius); },
                                 "Radius of sphere must be positive.");
}

/// tests CSGXCylinder constructor and attributes
TEST(CSGSurfaceTest, testXCylinder)
{
  CSGXCylinder cyl = makeCylinder<CSGXCylinder>();
  checkCylinder("X", cyl.getCoeffs(), cyl.getSurfaceType());
}

/// tests CSGYCylinder constructor and attributes
TEST(CSGSurfaceTest, testYCylinder)
{
  CSGYCylinder cyl = makeCylinder<CSGYCylinder>();
  checkCylinder("Y", cyl.getCoeffs(), cyl.getSurfaceType());
}

/// tests CSGZCylinder constructor and attributes
TEST(CSGSurfaceTest, testZCylinder)
{
  CSGZCylinder cyl = makeCylinder<CSGZCylinder>();
  checkCylinder("Z", cyl.getCoeffs(), cyl.getSurfaceType());
}

/// Expect error if cylinders are constructed with a negative radius
TEST(CSGSurfaceTest, testCylNegativeRadius)
{
  Real neg_r = -1.0;
  // XCylinder
  {
    Moose::UnitUtils::assertThrows([&neg_r]() { CSGXCylinder xcyl("cyl", 0, 0, neg_r); },
                                   "Radius of x-cylinder must be positive.");
  }
  // YCylinder
  {
    Moose::UnitUtils::assertThrows([&neg_r]() { CSGYCylinder ycyl("cyl", 0, 0, neg_r); },
                                   "Radius of y-cylinder must be positive.");
  }
  // ZCylinder
  {
    Moose::UnitUtils::assertThrows([&neg_r]() { CSGZCylinder zcyl("cyl", 0, 0, neg_r); },
                                   "Radius of z-cylinder must be positive.");
  }
}

// tests CSG[X/Y/Z]Cylinder::evaluateSurfaceEquationAtPoint
TEST(CSGSurfaceTest, testCylEvaluateEq)
{
  Real exp_val = -4.0;
  const Point p = Point(0.0, 0.0, 0.0);
  // XCylinder
  {
    CSGXCylinder cyl = makeCylinder<CSGXCylinder>();
    ASSERT_FLOAT_EQ(exp_val, cyl.evaluateSurfaceEquationAtPoint(p));
  }
  // YCylinder
  {
    CSGYCylinder cyl = makeCylinder<CSGYCylinder>();
    ASSERT_FLOAT_EQ(exp_val, cyl.evaluateSurfaceEquationAtPoint(p));
  }
  // ZCylinder
  {
    CSGZCylinder cyl = makeCylinder<CSGZCylinder>();
    ASSERT_FLOAT_EQ(exp_val, cyl.evaluateSurfaceEquationAtPoint(p));
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
TEST(CSGSurfaceTest, testGetName)
{
  std::string name = "sph_surf_name";
  CSGSphere sph(name, 1.0);
  ASSERT_EQ(name, sph.getName());
}

/// test CSGSurface::setName
TEST(CSGSurfaceTest, testSetName)
{
  CSGSphere sph("first_name", 1.0);
  sph.setName("new_name");
  ASSERT_EQ("new_name", sph.getName());
}

} // namespace CSG
