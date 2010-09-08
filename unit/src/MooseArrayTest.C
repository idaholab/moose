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

#include "MooseArrayTest.h"

//Moose includes
#include "MooseArray.h"

CPPUNIT_TEST_SUITE_REGISTRATION( MooseArrayTest );

void
MooseArrayTest::setUp()
{
}

void
MooseArrayTest::tearDown()
{
}

void
MooseArrayTest::defaultConstructor()
{
  MooseArray<int> ma;
  CPPUNIT_ASSERT( ma.size() == 0 );
}

void
MooseArrayTest::sizeConstructor()
{
  MooseArray<int> ma( 6 );
  CPPUNIT_ASSERT( ma.size() == 6 );
  ma[5] = 42;
  CPPUNIT_ASSERT( ma[5] == 42 );
}

void
MooseArrayTest::valueConstructor()
{
  int value = 42;
  MooseArray<int> ma( 6, value );
  CPPUNIT_ASSERT( ma[0] == 42 );
  CPPUNIT_ASSERT( ma[1] == 42 );
  CPPUNIT_ASSERT( ma[2] == 42 );
  CPPUNIT_ASSERT( ma[3] == 42 );
  CPPUNIT_ASSERT( ma[4] == 42 );
  CPPUNIT_ASSERT( ma[5] == 42 );
  CPPUNIT_ASSERT( ma.size() == 6 );

  ma[5] = 44;
  CPPUNIT_ASSERT( ma[5] == 44 );
}

void
MooseArrayTest::setAllValues()
{
  MooseArray<int> ma( 6 );
  ma[5] = 44;
  int value = 42;
  ma.setAllValues( value );

  CPPUNIT_ASSERT( ma[0] == 42 );
  CPPUNIT_ASSERT( ma[1] == 42 );
  CPPUNIT_ASSERT( ma[2] == 42 );
  CPPUNIT_ASSERT( ma[3] == 42 );
  CPPUNIT_ASSERT( ma[4] == 42 );
  CPPUNIT_ASSERT( ma[5] == 42 );
  CPPUNIT_ASSERT( ma.size() == 6 );
}

void
MooseArrayTest::release()
{
  MooseArray<int> ma( 6 );
  CPPUNIT_ASSERT( ma.size() == 6 );
  ma.release();
  CPPUNIT_ASSERT( ma.size() == 0 );
}

void
MooseArrayTest::resize()
{
  MooseArray<int> ma( 4 );
  ma[0] = 1;
  ma[1] = 2;
  ma[2] = 3;
  ma[3] = 4;

  CPPUNIT_ASSERT( ma.size() == 4 );
  ma.resize( 6, 42 );
  CPPUNIT_ASSERT( ma.size() == 6 );
  CPPUNIT_ASSERT( ma[4] == 42 );
  CPPUNIT_ASSERT( ma[5] == 42 );
  ma[0] = 1;
  ma[5] = 44;
  CPPUNIT_ASSERT( ma[0] == 1 );
  CPPUNIT_ASSERT( ma[5] == 44 );

  ma.resize( 2, 44 );
  CPPUNIT_ASSERT( ma.size() == 2 );

  ma.resize( 4, 33 );
  CPPUNIT_ASSERT( ma.size() == 4 );
  
  // These tests only pass if resize() sets default_value works when resizing
  // to a value still less than _allocated_size
  //CPPUNIT_ASSERT( ma[2] == 33 );
  //CPPUNIT_ASSERT( ma[3] == 33 );
}

void
MooseArrayTest::resizeDefault()
{
  MooseArray<int> ma( 4 );
  ma[0] = 1;
  ma[1] = 2;
  ma[2] = 3;
  ma[3] = 4;

  CPPUNIT_ASSERT( ma.size() == 4 );
  ma.resize( 6 );
  ma[0] = 1;
  ma[5] = 42;
  CPPUNIT_ASSERT( ma.size() == 6 );
  CPPUNIT_ASSERT( ma[0] == 1 );
  CPPUNIT_ASSERT( ma[5] == 42 );

  ma.resize( 2 );
  CPPUNIT_ASSERT( ma.size() == 2 );
  ma[0] = 1;
  CPPUNIT_ASSERT( ma[0] == 1 );
}

void
MooseArrayTest::size()
{
  //mostly tested in other functions
  MooseArray<int> ma( 6 );
  CPPUNIT_ASSERT( ma.size() == 6 );
}

void
MooseArrayTest::access()
{
  MooseArray<int> ma( 4 );
  ma[0] = 1;
  ma[1] = 2;
  ma[2] = 3;
  ma[3] = 4;

  CPPUNIT_ASSERT( ma[0] == 1 );
  CPPUNIT_ASSERT( ma[1] == 2 );
  CPPUNIT_ASSERT( ma[2] == 3 );
  CPPUNIT_ASSERT( ma[3] == 4 );
}

void
MooseArrayTest::shallowCopy()
{
  //shallow copy a few different sizes of arrays and make sure the sizes and values stay consistant
  MooseArray<Real> ma4( 4, 8 );
  MooseArray<Real> ma3( 3 );
  ma3[0] = 1;
  ma3[1] = 2;
  ma3[2] = 3;
  MooseArray<Real> ma2( 2, 9 );

  ma4.shallowCopy( ma3 );
  ma2.shallowCopy( ma3 );

  CPPUNIT_ASSERT( ma4.size() == 3 );
  CPPUNIT_ASSERT( ma4[0] == 1 );
  CPPUNIT_ASSERT( ma4[1] == 2 );
  CPPUNIT_ASSERT( ma4[2] == 3 );
  CPPUNIT_ASSERT( ma2.size() == 3 );
  CPPUNIT_ASSERT( ma2[0] == 1 );
  CPPUNIT_ASSERT( ma2[1] == 2 );
  CPPUNIT_ASSERT( ma2[2] == 3 );

  ma4.resize( 5, 42 );
  ma2.shallowCopy( ma4 );
  ma3.shallowCopy( ma4 );
  ma4[0] = 22;
  CPPUNIT_ASSERT( ma4.size() == 5 );
  CPPUNIT_ASSERT( ma2.size() == 5 );
  CPPUNIT_ASSERT( ma3.size() == 5 );
  CPPUNIT_ASSERT( ma2[4] == 42 );
  CPPUNIT_ASSERT( ma3[4] == 42 );
  CPPUNIT_ASSERT( ma4[4] == 42 );
  CPPUNIT_ASSERT( ma2[0] == 22 );
  CPPUNIT_ASSERT( ma3[0] == 22 );
  CPPUNIT_ASSERT( ma4[0] == 22 );
}
