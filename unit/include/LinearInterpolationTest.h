#ifndef LINEARINTERPOLATIONTEST_H
#define LINEARINTERPOLATIONTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

class LinearInterpolationTest : public CppUnit::TestFixture 
{
  CPPUNIT_TEST_SUITE( LinearInterpolationTest );

  CPPUNIT_TEST( constructor );
  //CPPUNIT_TEST( sample );
  CPPUNIT_TEST( getSampleSize );

  CPPUNIT_TEST_SUITE_END();
  
public:
  void setUp();
  void tearDown();
  
  void constructor();
  void sample();
  void getSampleSize();

private:
  std::vector<double> * _x;
  std::vector<double> * _y;
};

#endif  // LINEARINTERPOLATIONTEST_H
