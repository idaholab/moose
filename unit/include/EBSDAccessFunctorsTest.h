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

#ifndef EBSDACCESSFUNCTORSTEST_H
#define EBSDACCESSFUNCTORSTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class EBSDAccessFunctorsTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( EBSDAccessFunctorsTest );

  CPPUNIT_TEST( pointData );
  CPPUNIT_TEST( avgData );

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void pointData();
  void avgData();

private:
};

#endif //EBSDACCESSFUNCTORSTEST_H
