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

#ifndef SPLITFILENAMETEST_H
#define SPLITFILENAMETEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

class SplitFileNameTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SplitFileNameTest);
  CPPUNIT_TEST(validName);
  CPPUNIT_TEST_SUITE_END();

public:
  void validName();
  void invalidName();
};

#endif // SPLITFILENAMETEST_H
