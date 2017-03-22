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

#ifndef MOOSERANDOMTEST_H
#define MOOSERANDOMTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class MooseRandomTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MooseRandomTest);

  CPPUNIT_TEST(rand);
  CPPUNIT_TEST(randSeq);
  CPPUNIT_TEST(randNormal);
  CPPUNIT_TEST(randNormal2);
  CPPUNIT_TEST(randl);
  CPPUNIT_TEST(states);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void rand();
  void randSeq();
  void randNormal();
  void randNormal2();
  void randl();
  void states();
};

#endif // MOOSERANDOMTEST_H
