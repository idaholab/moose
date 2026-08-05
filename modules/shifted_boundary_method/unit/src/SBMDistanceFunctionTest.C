//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObjectUnitTest.h"
#include "Function.h"
#include "SBMUtils.h"

#include "gtest/gtest.h"

// Fixture that provides an app/problem so real Function objects can be built and handed to the
// SBMUtils distance helpers, which accept only ParsedFunction / (Un)SignedDistanceToSurfaceMesh.
class SBMDistanceFunctionTest : public MooseObjectUnitTest
{
public:
  SBMDistanceFunctionTest() : MooseObjectUnitTest("ShiftedBoundaryMethodApp") {}

protected:
  const Function & buildFunction(const std::string & type,
                                 const std::string & name,
                                 const std::function<void(InputParameters &)> & set_params)
  {
    InputParameters params = _factory.getValidParams(type);
    params.set<FEProblem *>("_fe_problem") = _fe_problem.get();
    params.set<FEProblemBase *>("_fe_problem_base") = _fe_problem.get();
    set_params(params);
    _fe_problem->addFunction(type, name, params);
    Function & f = _fe_problem->getFunction(name);
    f.initialSetup();
    return f;
  }

  const Function & parsed(const std::string & name, const std::string & expr)
  {
    return buildFunction("ParsedFunction",
                         name,
                         [&expr](InputParameters & p) { p.set<std::string>("expression") = expr; });
  }
};

// distanceVectorFromFunction returns the vector from the point to the zero level set:
// -(phi / |grad phi|) * grad phi. For phi = x (the plane x = 0) at (2, 0, 0) that is (-2, 0, 0).
TEST_F(SBMDistanceFunctionTest, DistanceVectorFromPlane)
{
  const Function & phi = parsed("phi_x", "x");
  const RealVectorValue d = SBMUtils::distanceVectorFromFunction(&phi, Point(2, 0, 0), 0);
  EXPECT_NEAR(d(0), -2.0, 1e-6);
  EXPECT_NEAR(d(1), 0.0, 1e-6);
  EXPECT_NEAR(d(2), 0.0, 1e-6);
}

// A vanishing gradient (constant expression) yields a zero distance vector rather than dividing
// by zero.
TEST_F(SBMDistanceFunctionTest, DistanceVectorZeroGradient)
{
  const Function & c = parsed("phi_const", "5");
  const RealVectorValue d = SBMUtils::distanceVectorFromFunction(&c, Point(1, 1, 1), 0);
  EXPECT_NEAR(d.norm(), 0.0, 1e-12);
}

// trueNormalFromFunction returns the unit outward normal grad phi / |grad phi|; for phi = x it is
// (1, 0, 0) everywhere.
TEST_F(SBMDistanceFunctionTest, TrueNormalFromPlane)
{
  const Function & phi = parsed("phi_x2", "x");
  const RealVectorValue n = SBMUtils::trueNormalFromFunction(&phi, Point(2, 0, 0), 0);
  EXPECT_NEAR(n(0), 1.0, 1e-6);
  EXPECT_NEAR(n(1), 0.0, 1e-6);
  EXPECT_NEAR(n(2), 0.0, 1e-6);
}

// closestDistanceVector / closestTrueNormalVector select the nearest of several surfaces. At
// (2, 0, 0) the plane x = 0 (distance 2) is closer than x = 10 (distance 8).
TEST_F(SBMDistanceFunctionTest, ClosestPicksNearestSurface)
{
  const Function & near = parsed("near", "x");
  const Function & far = parsed("far", "x - 10");
  const std::vector<const Function *> funcs{&near, &far};
  const Point pt(2, 0, 0);

  const RealVectorValue d = SBMUtils::closestDistanceVector(funcs, pt, 0);
  EXPECT_NEAR(d(0), -2.0, 1e-6);

  const RealVectorValue n = SBMUtils::closestTrueNormalVector(funcs, pt, 0);
  EXPECT_NEAR(n(0), 1.0, 1e-6);
}

// unionSignedDistance is the minimum signed distance over the functions: min(2, -8) = -8.
TEST_F(SBMDistanceFunctionTest, UnionSignedDistanceTakesMin)
{
  const Function & u1 = parsed("u1", "x");
  const Function & u2 = parsed("u2", "x - 10");
  const std::vector<const Function *> funcs{&u1, &u2};
  EXPECT_NEAR(SBMUtils::unionSignedDistance(funcs, 0, Point(2, 0, 0)), -8.0, 1e-6);
}

// A function that is not a supported signed-distance strategy is rejected. This exercises the same
// type validation used by buildDistanceFunctions.
TEST_F(SBMDistanceFunctionTest, UnionSignedDistanceRejectsUnsupportedType)
{
  const Function & c = buildFunction(
      "ConstantFunction", "c", [](InputParameters & p) { p.set<Real>("value") = 1.0; });
  const std::vector<const Function *> funcs{&c};
  EXPECT_ANY_THROW(SBMUtils::unionSignedDistance(funcs, 0, Point(0, 0, 0)));
}
