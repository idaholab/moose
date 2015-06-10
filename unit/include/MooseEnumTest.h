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

#ifndef MOOSEENUMTEST_H
#define MOOSEENUMTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

#include <string>

class MooseEnumTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( MooseEnumTest );

  CPPUNIT_TEST( multiTestOne );
  CPPUNIT_TEST( withNamesFromTest );
  CPPUNIT_TEST( testErrors );

  CPPUNIT_TEST_SUITE_END();

public:
  void multiTestOne();
  void withNamesFromTest();
  void testErrors();
};

#endif  // MOOSEENUMTEST_H
