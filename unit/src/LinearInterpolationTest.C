#include "LinearInterpolationTest.h"

//Moose includes
#include "LinearInterpolation.h"

CPPUNIT_TEST_SUITE_REGISTRATION( LinearInterpolationTest );

void
LinearInterpolationTest::setUp()
{
  _x = new std::vector<double>( 4 );
  _y = new std::vector<double>( 4 );

  std::vector<double> & x = *_x;
  std::vector<double> & y = *_y;

  x[0] = 1.; y[0] = 0.;
  x[1] = 2.; y[1] = 5.;
  x[2] = 3.; y[2] = 6.;
  x[3] = 5.; y[3] = 8.;
}

void
LinearInterpolationTest::tearDown()
{
  delete _x;
  delete _y;
}

void
LinearInterpolationTest::constructor()
{
  LinearInterpolation interp( *_x, *_y );
  CPPUNIT_ASSERT( interp.getSampleSize() == _x->size() );
}

void
LinearInterpolationTest::sample()
{
  //TODO: this doesn't work right now, need to fix the Moose code
  LinearInterpolation interp( *_x, *_y );

  std::cout << _x->at( 0 ) << ' ' << _y->at( 0 ) << '\n';
  std::cout << _x->at( 1 ) << ' ' << _y->at( 1 ) << "\n";
  std::cout << _x->at( 2 ) << ' ' << _y->at( 2 ) << '\n';
  std::cout << _x->at( 3 ) << ' ' << _y->at( 3 ) << "\n\n";

  std::cout << "0: " << interp.sample( 0. ) << '\n';
  std::cout << "1: " << interp.sample( 1. ) << '\n';
  std::cout << "2: " << interp.sample( 2. ) << '\n';
  std::cout << "3: " << interp.sample( 3. ) << '\n';
  std::cout << "4: " << interp.sample( 4. ) << '\n';
  std::cout << "5: " << interp.sample( 5. ) << '\n';
  std::cout << "6: " << interp.sample( 6. ) << '\n';
  std::cout << "7: " << interp.sample( 7. ) << '\n';

  CPPUNIT_ASSERT( interp.sample( 0. ) == 0. );
  CPPUNIT_ASSERT( interp.sample( 1. ) == 0. );
  CPPUNIT_ASSERT( interp.sample( 2. ) == 5. );
  CPPUNIT_ASSERT( interp.sample( 3. ) == 6. );
  CPPUNIT_ASSERT( interp.sample( 4. ) == 7. );
  CPPUNIT_ASSERT( interp.sample( 5. ) == 8. );
  CPPUNIT_ASSERT( interp.sample( 6. ) == 8. );

  CPPUNIT_ASSERT( interp.sample( 1.5 ) == 2.5 );
}

void
LinearInterpolationTest::getSampleSize()
{
  LinearInterpolation interp( *_x, *_y );
  CPPUNIT_ASSERT( interp.getSampleSize() == _x->size() );
}
