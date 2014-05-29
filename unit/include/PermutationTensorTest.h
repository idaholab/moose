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

#ifndef PERMUTATIONTENSORTEST_H
#define PERMUTATIONTENSORTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class PermutationTensorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( PermutationTensorTest );

  CPPUNIT_TEST( twoD );
  CPPUNIT_TEST( threeD );
  CPPUNIT_TEST( fourD );

  CPPUNIT_TEST_SUITE_END();

public:
  void twoD();
  void threeD();
  void fourD();
};

#endif  // PERMUTATIONTENSORTEST_H
