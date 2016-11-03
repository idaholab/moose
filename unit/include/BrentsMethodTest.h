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

#ifndef BRENTSMETHODTEST_H
#define BRENTSMETHODTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

#include "Moose.h"

class BrentsMethodTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(BrentsMethodTest);

  CPPUNIT_TEST(bracket);
  CPPUNIT_TEST(root);

  CPPUNIT_TEST_SUITE_END();

public:
  /// Tests that roots are bracketed
  void bracket();
  /// Tests that the root is correctly calculated
  void root();

  /**
   * Test function for Brents method.
   * f(x) = log(1 + x) * tanh(x / 3) + x / 4 - 3 which has a root at 5.170302597.
   *
   * Note: the implementation of Brents bracketing method restricts the bracketing
   * interval to positive values
   */
  Real f(Real x) const;
};

#endif // BRENTSMETHODTEST_H
