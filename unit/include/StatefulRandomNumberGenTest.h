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

#ifndef STATEFULRANDOMNUMBERGENTEST_H
#define STATEFULRANDOMNUMBERGENTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

#include "MooseStatefulRandom.h"

#include <string>

class StatefulRandomNumberGenTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( StatefulRandomNumberGenTest );

  CPPUNIT_TEST( testRandomGen );

  CPPUNIT_TEST_SUITE_END();

public:
  void testRandomGen();
};

#endif  // STATEFULRANDOMNUMBERGENTEST_H
