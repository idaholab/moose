#include "MooseArrayTest.h"

//Moose includes
#include "MooseArray.h"

CPPUNIT_TEST_SUITE_REGISTRATION( MooseArrayTest );

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
  CPPUNIT_ASSERT( ma[2] == 33 );
  CPPUNIT_ASSERT( ma[3] == 33 );
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
  //TODO: is this testing sufficient?
  //shallow copy a few different sizes of arrays and make sure the sizes and values stay consistant
  MooseArray<int> ma4( 4, 8 );
  MooseArray<int> ma3( 3 );
  ma3[0] = 1;
  ma3[1] = 2;
  ma3[2] = 3;
  MooseArray<int> ma2( 2, 9 );

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
}
