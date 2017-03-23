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

#ifndef MINIMALAPP_H
#define MINIMALAPP_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

#include <string>

class MinimalApp : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(MinimalApp);

  CPPUNIT_TEST(createMinimalAppTest);

  CPPUNIT_TEST_SUITE_END();

public:
  void createMinimalAppTest();
};

#endif // MINIMALAPP_H
