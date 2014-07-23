
#include "RankFourTensorTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( RankFourTensorTest );

RankFourTensorTest::RankFourTensorTest()
{
  _m0 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 1);
  // _m1 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 1);
  //_m2 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 3);
  //_m3 = RankTwoTensor(1, 2, 3, 2, -5, -6, 3, -6, 9);
  //_unsymmetric0 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 9);
  //_unsymmetric1 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 10);
}

void RankFourTensorTest::secondDerivativeTest()

RankFourTensorTest::~RankFourTensorTest()
{}

