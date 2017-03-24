/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "BrentsMethodTest.h"
#include "BrentsMethod.h"

CPPUNIT_TEST_SUITE_REGISTRATION(BrentsMethodTest);

Real
BrentsMethodTest::f(Real x) const
{
  return std::log(1.0 + x) * std::tanh(x / 3.0) + x / 4.0 - 3;
}

void
BrentsMethodTest::bracket()
{
  // Initial guess for bracketing interval does not bracket root
  Real x1 = 0.5;
  Real x2 = 1.0;

  auto func = [this](Real x) { return this->f(x); };

  // Call bracket to determine the bracketing interval
  BrentsMethod::bracket(func, x1, x2);

  // The product of the function f(x) at the bracketing interval (x1, x2) must
  // be negative to bracket a root
  CPPUNIT_ASSERT(f(x1) * f(x2) < 0.0);

  // Test that a warning is thrown if the initial guesses are equal
  try
  {
    // Trigger identical initial guess error
    BrentsMethod::bracket(func, x1, x1);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("Bad initial range (0) used in BrentsMethod::bracket") !=
                   std::string::npos);
  }

  // Test that a warning is thrown if no bracketing interval is found after 50 iterations.
  try
  {
    // Trigger no bracketing interval warning by adding 4 to f(x), whereby no real root exists
    auto func2 = [this](Real x) { return this->f(x) + 4.0; };

    BrentsMethod::bracket(func2, x1, x2);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(
        msg.find("No bracketing interval found by BrentsMethod::bracket after 50 iterations") !=
        std::string::npos);
  }
}

void
BrentsMethodTest::root()
{
  // Bracketing interval that does bracket root
  Real x1 = 0.5;
  Real x2 = 10.0;

  auto func = [this](Real x) { return this->f(x); };

  // Check that the root is 5.170302597
  CPPUNIT_ASSERT_DOUBLES_EQUAL(5.170302597, BrentsMethod::root(func, x1, x2), 1.0E-8);

  // Test that a warning is thrown if the supplied interval does not bracket the root
  try
  {
    // Trigger no bracketing interval error
    x2 = 1.0;
    BrentsMethod::root(func, x1, x2);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    CPPUNIT_ASSERT(msg.find("Root must be bracketed in BrentsMethod::root") != std::string::npos);
  }
}
