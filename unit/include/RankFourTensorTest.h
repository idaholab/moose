#ifndef RANKFOURTENSORTEST_H
#define RANKFOURTENSORTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Moose includes
#include "RankFourTensor.h"

class RankFourTensorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE( RankFourTensorTest );

  //CPPUNIT_TEST( dtraceTest );
  //CPPUNIT_TEST( dsecondInvariantTest );
  //CPPUNIT_TEST( ddetTest );

  CPPUNIT_TEST_SUITE_END();

public:
  RankFourTensorTest();
  ~RankFourTensorTest();

 private:
  RankTwoTensor _m0;

};

#endif  // RANKFOURTENSORTEST_H
