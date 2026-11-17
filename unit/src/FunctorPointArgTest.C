//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseAppTestUtils.h"
#include "MooseFunctor.h"
#include "MooseMain.h"

#include <string>

using MooseAppTestUtils::Args;

namespace
{
const std::string input = "files/FunctorPointArgTest/point_arg.i";

/// Evaluates the functor material property \p name, which is a PiecewiseByBlockLambdaFunctor,
/// at an arbitrary physical point
Real
evaluateAtPoint(FEProblemBase & fe_problem, const std::string & name)
{
  const auto & functor = fe_problem.getFunctor<ADReal>(
      name, /*tid=*/0, "FunctorPointArgTest", /*requestor_is_ad=*/true);
  return MetaPhysicL::raw_value(
      functor(Moose::PointArg{Point(0.3, 0.2, 0)}, Moose::currentState()));
}
}

// A scalar variable has no spatial dependence, so a functor material property that forwards to one
// can be evaluated at a point that has no associated element
TEST(FunctorPointArg, ScalarVariable)
{
  Args args({"-i", input});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  auto & fe_problem = app->getExecutioner()->feProblem();

  EXPECT_EQ(evaluateAtPoint(fe_problem, "scalar_prop"), 3);
}

// A field variable cannot be evaluated without an element, so a functor material property that
// forwards to one must report that point evaluation is unsupported
TEST(FunctorPointArg, FieldVariable)
{
  Args args({"-i", input});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  auto & fe_problem = app->getExecutioner()->feProblem();

  try
  {
    evaluateAtPoint(fe_problem, "field_prop");
    FAIL() << "Evaluating a field variable at a point should have thrown";
  }
  catch (const std::runtime_error & e)
  {
    EXPECT_NE(std::string(e.what()).find("Evaluation at a point is not supported for functor u"),
              std::string::npos);
  }
}

// A point has no subdomain, so a functor material property that is not defined on every block
// cannot be evaluated at one
TEST(FunctorPointArg, PartialBlockRestriction)
{
  Args args({"-i", input});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  auto & fe_problem = app->getExecutioner()->feProblem();

  try
  {
    evaluateAtPoint(fe_problem, "block_prop");
    FAIL() << "Evaluating a block-restricted property at a point should have thrown";
  }
  catch (const std::runtime_error & e)
  {
    EXPECT_NE(std::string(e.what()).find(
                  "did not provide a functor material definition on that subdomain"),
              std::string::npos);
  }
}
