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

#ifndef RANKTWOTENSORTEST_H
#define RANKTWOTENSORTEST_H

#include "gtest/gtest.h"

// Moose includes
#include "RankTwoTensor.h"

class RankTwoTensorTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    _m0 = RankTwoTensor(0, 0, 0, 0, 0, 0, 0, 0, 0);
    _m1 = RankTwoTensor(1, 0, 0, 0, 1, 0, 0, 0, 1);
    _m2 = RankTwoTensor(1, 0, 0, 0, 2, 0, 0, 0, 3);
    _m3 = RankTwoTensor(1, 2, 3, 2, -5, -6, 3, -6, 9);
    _unsymmetric0 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 9);
    _unsymmetric1 = RankTwoTensor(1, 2, 3, -4, -5, -6, 7, 8, 10);
  }

  RankTwoTensor _m0;
  RankTwoTensor _m1;
  RankTwoTensor _m2;
  RankTwoTensor _m3;
  RankTwoTensor _unsymmetric0;
  RankTwoTensor _unsymmetric1;
};

#endif // RANKTWOTENSORTEST_H
