//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "RankFourTensor.h"
#include "RankTwoTensor.h"

RankFourTensor A(
    {0.37187984, 0.1205132,  0.20604735, 0.37231474, 0.43842493, 0.70315511, 0.4290739,  0.13976259,
     0.09908869, 0.30118635, 0.77499901, 0.27357056, 0.60135915, 0.23465929, 0.22293769, 0.19040275,
     0.27290676, 0.74815661, 0.28154392, 0.60755658, 0.03619608, 0.3624321,  0.11706153, 0.17841735,
     0.97943584, 0.09110013, 0.46637981, 0.03828105, 0.33870729, 0.32737051, 0.77580343, 0.24627582,
     0.78159833, 0.9107652,  0.25967326, 0.78975337, 0.16262543, 0.39379351, 0.76695509, 0.06725889,
     0.05830972, 0.35663544, 0.30705526, 0.10678273, 0.86049446, 0.35409874, 0.11516089, 0.19278114,
     0.13198504, 0.59468892, 0.56816644, 0.12003418, 0.22053863, 0.0982336,  0.62431929, 0.71956444,
     0.21428326, 0.44609002, 0.41339009, 0.42191298, 0.48892364, 0.62091132, 0.18801674, 0.70655228,
     0.19426012, 0.46781494, 0.71600797, 0.60481787, 0.70137889, 0.03859477, 0.77192629, 0.32630718,
     0.75514572, 0.40633981, 0.8421178,  0.75133872, 0.18673922, 0.74648423, 0.89100271, 0.74446494,
     0.65905056},
    RankFourTensor::general);

RankTwoTensor a(std::vector<double>({0.95329888,
                                     0.34413534,
                                     0.37195927,
                                     0.79306425,
                                     0.64554447,
                                     0.00288018,
                                     0.98826094,
                                     0.26405734,
                                     0.80046234}));

TEST(RankFourTensor, inverse)
{
  // Invert a random-ish rank 4 tensor
  RankFourTensor Ai = A.inverse();
  RankFourTensor sI = A * Ai;

  EXPECT_NEAR(0, (sI - RankFourTensor::IdentityFour()).L2norm(), 1E-5);
}

TEST(RankFourTensor, idDeviatoric)
{
  RankFourTensor I = RankFourTensor(RankFourTensor::initIdentityDeviatoric);

  RankFourTensor ID_should =
      RankFourTensor::IdentityFour() -
      RankTwoTensor::Identity().outerProduct(RankTwoTensor::Identity()) / 3.0;

  EXPECT_NEAR(0, (I - ID_should).L2norm(), 1E-5);
}

TEST(RankFourTensor, contractionIj)
{
  unsigned int i = 1;
  unsigned int j = 0;

  auto R1 = A.contractionIj(i, j, a);

  Real R2 = 0;
  for (unsigned int k = 0; k < 3; k++)
    for (unsigned int l = 0; l < 3; l++)
      R2 += A(i, j, k, l) * a(k, l);

  EXPECT_NEAR(0, R1 - R2, 1E-5);
}

TEST(RankFourTensor, contractionKl)
{
  unsigned int k = 0;
  unsigned int l = 0;

  auto R1 = A.contractionKl(k, l, a);

  Real R2 = 0;
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      R2 += A(i, j, k, l) * a(i, j);

  EXPECT_NEAR(0, R1 - R2, 1E-5);
}

TEST(RankFourTensor, tripleProductJkl)
{
  auto R1 = A.tripleProductJkl(a, a, a);

  RankFourTensor R2;
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
          for (unsigned int m = 0; m < 3; m++)
            for (unsigned int n = 0; n < 3; n++)
              for (unsigned int t = 0; t < 3; t++)
                R2(i, j, k, l) += A(i, m, n, t) * a(j, m) * a(k, n) * a(l, t);

  EXPECT_NEAR(0, (R1 - R2).L2norm(), 1E-5);
}

TEST(RankFourTensor, singleProductI)
{
  auto R1 = A.singleProductI(a);

  RankFourTensor R2;

  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < 3; j++)
      for (unsigned int k = 0; k < 3; k++)
        for (unsigned int l = 0; l < 3; l++)
          for (unsigned int m = 0; m < 3; m++)
            R2(i, j, k, l) += A(m, j, k, l) * a(i, m);

  EXPECT_NEAR(0, (R1 - R2).L2norm(), 1E-5);
}
