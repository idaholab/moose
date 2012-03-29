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

#ifndef LINEARINTERPOLATIONTEST_H
#define LINEARINTERPOLATIONTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class IndirectSort : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( IndirectSort );

  CPPUNIT_TEST( realSort );
  CPPUNIT_TEST( intSort );
  CPPUNIT_TEST( testStableSort );
  CPPUNIT_TEST( testDoubleSort );

  CPPUNIT_TEST_SUITE_END();

public:
  void realSort();
  void intSort();
  void testStableSort();
  void testDoubleSort();
};

#endif  // LINEARINTERPOLATIONTEST_H
