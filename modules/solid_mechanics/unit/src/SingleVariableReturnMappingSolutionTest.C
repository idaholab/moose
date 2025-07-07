//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "SingleVariableReturnMappingSolution.h"
#include "MooseMain.h"

namespace
{
// Polynomial Residual
template <bool is_ad>
class SVRMTest : public SingleVariableReturnMappingSolutionTempl<is_ad>
{
public:
  SVRMTest(InputParameters & params, ConsoleStream & console)
    : SingleVariableReturnMappingSolutionTempl<is_ad>(params), _console(console)
  {
  }

  virtual GenericReal<is_ad> computeResidual(const GenericReal<is_ad> & ets,
                                             const GenericReal<is_ad> & s)
  {
    return (s - ets / 3.0) * (s - ets / 3.0) * (s - ets / 3.0) - 8;
  }
  virtual GenericReal<is_ad> computeDerivative(const GenericReal<is_ad> & ets,
                                               const GenericReal<is_ad> & s)
  {
    return 3.0 * (s - ets / 3.0) * (s - ets / 3.0);
  }
  virtual GenericChainedReal<is_ad>
  computeResidualAndDerivative(const GenericReal<is_ad> & ets, const GenericChainedReal<is_ad> & s)
  {
    return (s - ets / 2.0) * (s - ets / 2.0);
  }

  virtual Real computeReferenceResidual(const GenericReal<is_ad> &, const GenericReal<is_ad> &)
  {
    return 0.0;
  }
  virtual GenericReal<is_ad> initialGuess(const GenericReal<is_ad> & /*effective_trial_stress*/)
  {
    return _initial_guess;
  }

  GenericReal<is_ad> run(GenericReal<is_ad> ets, GenericReal<is_ad> initial)
  {
    _initial_guess = initial;
    GenericReal<is_ad> solution;
    this->returnMappingSolve(ets, solution, _console);
    return solution;
  }

private:
  ConsoleStream & _console;
  GenericReal<is_ad> _initial_guess;
};

// Exponential Residual
template <bool is_ad>
class SVRMTestExp : public SVRMTest<is_ad>
{
public:
  using SVRMTest<is_ad>::SVRMTest;
  virtual GenericReal<is_ad> computeResidual(const GenericReal<is_ad> & ets,
                                             const GenericReal<is_ad> & s)
  {
    return std::exp(s - ets / 5.0) - 1;
  }
  virtual GenericReal<is_ad> computeDerivative(const GenericReal<is_ad> & ets,
                                               const GenericReal<is_ad> & s)
  {
    return std::exp(s - ets / 5.0);
  }
};
}

TEST(SingleVariableReturnMappingSolutionTest, without_ad)
{
  static ConsoleStream console(Moose::createMinimalMooseApp()->getOutputWarehouse());

  auto params = SingleVariableReturnMappingSolution::validParams();
  params.set<std::string>("_object_name") = "test";

  // simple solves

  SVRMTest<false> test(params, console);
  // solve with initial guess 0.0
  EXPECT_NEAR(test.run(3.0, 0.0), 3.0, 1e-5);
  // guess 1.0 has a Jacobian of 0.0, we handle this special case
  EXPECT_NEAR(test.run(3.0, 1.0), 3.0, 1e-5);

  params.set<Real>("absolute_tolerance") = 1e-16;
  SVRMTest<false> test2(params, console);
  EXPECT_NEAR(test2.run(3.0, 11.0), 3.0, 1e-8);
  EXPECT_NEAR(test2.run(3.0, -10.0), 3.0, 1e-8);
  EXPECT_NEAR(test2.run(3.0, -100.0), 3.0, 1e-8);
  EXPECT_NEAR(test2.run(3.0, 100.0), 3.0, 1e-8);

  SVRMTestExp<false> test3(params, console);
  EXPECT_NEAR(test3.run(3.0, 11.0), 3.0 / 5.0, 1e-8);
  EXPECT_NEAR(test3.run(3.0, -10.0), 3.0 / 5.0, 1e-8);

  params.set<bool>("automatic_differentiation_return_mapping") = true;
  SVRMTest<false> test4(params, console);
  EXPECT_NEAR(test4.run(3.0, 11.0), 1.5, 1e-8);
}

TEST(SingleVariableReturnMappingSolutionTest, with_ad)
{
  static ConsoleStream console(Moose::createMinimalMooseApp()->getOutputWarehouse());

  auto params = SingleVariableReturnMappingSolution::validParams();
  params.set<std::string>("_object_name") = "test";

  // simple solves

  SVRMTest<true> test(params, console);
  SVRMTestExp<true> test4(params, console);
  // solve with initial guess 0.0
  EXPECT_NEAR(MetaPhysicL::raw_value(test.run(3.0, 0.0)), 3.0, 1e-5);
  // guess 1.0 has a Jacobian of 0.0, we handle this special case
  EXPECT_NEAR(MetaPhysicL::raw_value(test.run(3.0, 1.0)), 3.0, 1e-5);

  params.set<Real>("absolute_tolerance") = 1e-16;
  SVRMTest<true> test2(params, console);
  EXPECT_NEAR(MetaPhysicL::raw_value(test2.run(3.0, 11.0)), 3.0, 1e-8);
  EXPECT_NEAR(MetaPhysicL::raw_value(test2.run(3.0, -10.0)), 3.0, 1e-8);
  EXPECT_NEAR(MetaPhysicL::raw_value(test2.run(3.0, -100.0)), 3.0, 1e-8);
  EXPECT_NEAR(MetaPhysicL::raw_value(test2.run(3.0, 100.0)), 3.0, 1e-8);

  SVRMTestExp<true> test3(params, console);
  EXPECT_NEAR(MetaPhysicL::raw_value(test3.run(3.0, 11.0)), 3.0 / 5.0, 1e-8);
  EXPECT_NEAR(MetaPhysicL::raw_value(test3.run(3.0, -10.0)), 3.0 / 5.0, 1e-8);

  // propagating derivative

  ADReal ets = 7.0;
  Moose::derivInsert(ets.derivatives(), 22, 1);
  ADReal guess = 0.0;
  Moose::derivInsert(guess.derivatives(), 33, 1);
  auto result = test4.run(ets, guess);

  EXPECT_NEAR(MetaPhysicL::raw_value(result), 7.0 / 5.0, 1e-8);
  // derivative of the solution w.r.t. ets should be 1/5
  EXPECT_NEAR(MetaPhysicL::raw_value(result.derivatives()[22]), 0.2, 1e-8);
  // derivative w.r.t. the initial guess must be zero
  EXPECT_NEAR(MetaPhysicL::raw_value(result.derivatives()[33]), 0.0, 1e-12);

  // exact initial guess
  guess = 7.0 / 5.0;
  auto result2 = test4.run(ets, guess);
  EXPECT_NEAR(MetaPhysicL::raw_value(result2.derivatives()[22]), 0.2, 1e-8);
  EXPECT_NEAR(MetaPhysicL::raw_value(result2.derivatives()[33]), 0.0, 1e-12);

  // zero Jacobian (we cannot get a derivative in this case)
  ets = 3.0;
  guess = 0.0;
  auto result3 = test.run(ets, guess);
  EXPECT_NEAR(MetaPhysicL::raw_value(result3.derivatives()[22]), 0.0, 1e-8);
  EXPECT_NEAR(MetaPhysicL::raw_value(result3.derivatives()[33]), 0.0, 1e-12);

  // zero Jacobian, exact guess (we cannot get a derivative in this case)
  ets = 3.0;
  guess = 1.0;
  auto result4 = test.run(ets, guess);
  EXPECT_NEAR(MetaPhysicL::raw_value(result4.derivatives()[22]), 0.0, 1e-8);
  EXPECT_NEAR(MetaPhysicL::raw_value(result4.derivatives()[33]), 0.0, 1e-12);
}
