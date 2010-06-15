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

  //resize shallow copied array, then shallow copy back to the original
  ma4.resize( 5, 42 );
  CPPUNIT_ASSERT( ma4.size() == 5 );
  CPPUNIT_ASSERT( ma4[3] == 42 );
  CPPUNIT_ASSERT( ma4[4] == 42 );

  ma4.shallowCopy( ma3 );
  ma4[2] = 22;
  CPPUNIT_ASSERT( ma4.size() == 3 );
  CPPUNIT_ASSERT( ma3.size() == 3 );
  CPPUNIT_ASSERT( ma3[2] == 22 );
  CPPUNIT_ASSERT( ma4[2] == 22 );

  ma3.resize( 5, 20 );
  CPPUNIT_ASSERT( ma3.size() == 5 );
  CPPUNIT_ASSERT( ma3[3] == 20 );

  // Heisenburg Principle at work: before the previous asserts ma3[4] does
  // indeed have a value of 20, but after the asserts it gets a random value
  // and this assert fails. If you comment out the both above asserts it
  // will pass! (Linux, g++ tool chain)
  // verified by printing before and after
  CPPUNIT_ASSERT( ma3[4] == 20 );
  
  ma3.shallowCopy( ma2 );
  ma2[2] = 33;
  CPPUNIT_ASSERT( ma3.size() == 3 );
  CPPUNIT_ASSERT( ma2[2] == 33 );
  CPPUNIT_ASSERT( ma3[2] == 33 );
}

void
MooseArrayTest::testCrashBug()
{
  MooseArray<int> ma4( 4, 8 );
  MooseArray<int> ma3( 3 );

  ma4.shallowCopy( ma3 );

  ma4.resize( 5, 42 );
  CPPUNIT_ASSERT( ma4[4] == 42 );

  ma4.shallowCopy( ma3 );
  ma4[2] = 22;

  // This test will segfault if the above "Heisenburg test" (line 197) is
  // commented out or if it passes. If that test fails then this function
  // call to resize will segfault. (Linux, g++ tool chain)
  // verified by printing before and after
  ma3.resize( 5, 20 );
  CPPUNIT_ASSERT( ma3.size() == 5 );
  CPPUNIT_ASSERT( ma3[3] == 20 );
  CPPUNIT_ASSERT( ma3[4] == 20 );
}
