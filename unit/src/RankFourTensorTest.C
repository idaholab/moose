//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankFourTensorTest.h"
#include "RankFourTensorImplementation.h"
#include "RankThreeTensor.h"
#include "MooseTypes.h"
#include "ADReal.h"

#include "metaphysicl/raw_type.h"
#include "metaphysicl/dualnumberarray.h"

#include <sstream>

TEST_F(RankFourTensorTest, invSymm1)
{
  // inverse check for a standard symmetric-isotropic tensor
  std::vector<Real> input(2);
  input[0] = 1;
  input[1] = 3;
  RankFourTensor a(input, RankFourTensor::symmetric_isotropic);
  RankFourTensor iSymmetric = RankFourTensor(RankFourTensor::initIdentitySymmetricFour);

  EXPECT_NEAR(0, (iSymmetric - a.invSymm() * a).L2norm(), 1E-5);
}

TEST_F(RankFourTensorTest, inverse)
{
  // Invert a random-ish rank 4 tensor
  RankFourTensor A(
      {0.37187984, 0.1205132,  0.20604735, 0.37231474, 0.43842493, 0.70315511, 0.4290739,
       0.13976259, 0.09908869, 0.30118635, 0.77499901, 0.27357056, 0.60135915, 0.23465929,
       0.22293769, 0.19040275, 0.27290676, 0.74815661, 0.28154392, 0.60755658, 0.03619608,
       0.3624321,  0.11706153, 0.17841735, 0.97943584, 0.09110013, 0.46637981, 0.03828105,
       0.33870729, 0.32737051, 0.77580343, 0.24627582, 0.78159833, 0.9107652,  0.25967326,
       0.78975337, 0.16262543, 0.39379351, 0.76695509, 0.06725889, 0.05830972, 0.35663544,
       0.30705526, 0.10678273, 0.86049446, 0.35409874, 0.11516089, 0.19278114, 0.13198504,
       0.59468892, 0.56816644, 0.12003418, 0.22053863, 0.0982336,  0.62431929, 0.71956444,
       0.21428326, 0.44609002, 0.41339009, 0.42191298, 0.48892364, 0.62091132, 0.18801674,
       0.70655228, 0.19426012, 0.46781494, 0.71600797, 0.60481787, 0.70137889, 0.03859477,
       0.77192629, 0.32630718, 0.75514572, 0.40633981, 0.8421178,  0.75133872, 0.18673922,
       0.74648423, 0.89100271, 0.74446494, 0.65905056},
      RankFourTensor::general);
  RankFourTensor B = A.inverse();
  RankFourTensor C = A * B;

  EXPECT_NEAR(0, (C - RankFourTensor::IdentityFour()).L2norm(), 1E-5);
}

TEST_F(RankFourTensorTest, invSymm2)
{
  // following (basically random) "a" tensor has symmetry
  // a_ijkl = a_jikl = a_ijlk
  // BUT it doesn't have a_ijkl = a_klij
  RankFourTensor a;
  a(0, 0, 0, 0) = 1;
  a(0, 0, 0, 1) = a(0, 0, 1, 0) = 2;
  a(0, 0, 0, 2) = a(0, 0, 2, 0) = 1.1;
  a(0, 0, 1, 1) = 0.5;
  a(0, 0, 1, 2) = a(0, 0, 2, 1) = -0.2;
  a(0, 0, 2, 2) = 0.8;
  a(1, 1, 0, 0) = 0.6;
  a(1, 1, 0, 1) = a(1, 1, 1, 0) = 1.3;
  a(1, 1, 0, 2) = a(1, 1, 2, 0) = 0.9;
  a(1, 1, 1, 1) = 0.6;
  a(1, 1, 1, 2) = a(1, 1, 2, 1) = -0.3;
  a(1, 1, 2, 2) = -1.1;
  a(2, 2, 0, 0) = -0.6;
  a(2, 2, 0, 1) = a(2, 2, 1, 0) = -0.1;
  a(2, 2, 0, 2) = a(2, 2, 2, 0) = -0.3;
  a(2, 2, 1, 1) = 0.5;
  a(2, 2, 1, 2) = a(2, 2, 2, 1) = 0.7;
  a(2, 2, 2, 2) = -0.9;
  a(0, 1, 0, 0) = a(1, 0, 0, 0) = 1.2;
  a(0, 1, 0, 1) = a(0, 1, 1, 0) = a(1, 0, 0, 1) = a(1, 0, 1, 0) = 0.1;
  a(0, 1, 0, 2) = a(0, 1, 2, 0) = a(1, 0, 0, 2) = a(1, 0, 2, 0) = 0.15;
  a(0, 1, 1, 1) = a(1, 0, 1, 1) = 0.24;
  a(0, 1, 1, 2) = a(0, 1, 2, 1) = a(1, 0, 1, 2) = a(1, 0, 2, 1) = -0.4;
  a(0, 1, 2, 2) = a(1, 0, 2, 2) = -0.3;
  a(0, 2, 0, 0) = a(2, 0, 0, 0) = -0.3;
  a(0, 2, 0, 1) = a(0, 2, 1, 0) = a(2, 0, 0, 1) = a(2, 0, 1, 0) = -0.4;
  a(0, 2, 0, 2) = a(0, 2, 2, 0) = a(2, 0, 0, 2) = a(2, 0, 2, 0) = 0.22;
  a(0, 2, 1, 1) = a(2, 0, 1, 1) = -0.9;
  a(0, 2, 1, 2) = a(0, 2, 2, 1) = a(2, 0, 1, 2) = a(2, 0, 2, 1) = -0.05;
  a(0, 2, 2, 2) = a(2, 0, 2, 2) = 0.32;
  a(1, 2, 0, 0) = a(2, 1, 0, 0) = -0.35;
  a(1, 2, 0, 1) = a(1, 2, 1, 0) = a(2, 1, 0, 1) = a(2, 1, 1, 0) = -1.4;
  a(1, 2, 0, 2) = a(1, 2, 2, 0) = a(2, 1, 0, 2) = a(2, 1, 2, 0) = 0.2;
  a(1, 2, 1, 1) = a(2, 1, 1, 1) = -0.91;
  a(1, 2, 1, 2) = a(1, 2, 2, 1) = a(2, 1, 1, 2) = a(2, 1, 2, 1) = 1.4;
  a(1, 2, 2, 2) = a(2, 1, 2, 2) = 0.1;

  RankFourTensor iSymmetric = RankFourTensor(RankFourTensor::initIdentitySymmetricFour);
  EXPECT_NEAR(0, (iSymmetric - a.invSymm() * a).L2norm(), 1E-5);
}

TEST_F(RankFourTensorTest, ADConversion)
{
  RankFourTensor reg;
  ADRankFourTensor ad;

  ad = reg;
  reg = MetaPhysicL::raw_value(ad);

  GenericRankFourTensor<false> generic_reg;
  GenericRankFourTensor<true> generic_ad;

  generic_ad = generic_reg;
  generic_reg = MetaPhysicL::raw_value(generic_ad);
}

TEST_F(RankFourTensorTest, anisotropic21)
{

  std::vector<Real> input(21);
  for (size_t i = 0; i < input.size(); ++i)
    input[i] = i + 1.1;

  RankFourTensor a(input, RankFourTensor::symmetric21);
  RankFourTensor b;
  b(0, 0, 0, 0) = 1.1;
  b(0, 0, 0, 1) = 6.1;
  b(0, 0, 0, 2) = 5.1;
  b(0, 0, 1, 0) = 6.1;
  b(0, 0, 1, 1) = 2.1;
  b(0, 0, 1, 2) = 4.1;
  b(0, 0, 2, 0) = 5.1;
  b(0, 0, 2, 1) = 4.1;
  b(0, 0, 2, 2) = 3.1;
  b(0, 1, 0, 0) = 6.1;
  b(0, 1, 0, 1) = 21.1;
  b(0, 1, 0, 2) = 20.1;
  b(0, 1, 1, 0) = 21.1;
  b(0, 1, 1, 1) = 11.1;
  b(0, 1, 1, 2) = 18.1;
  b(0, 1, 2, 0) = 20.1;
  b(0, 1, 2, 1) = 18.1;
  b(0, 1, 2, 2) = 15.1;
  b(0, 2, 0, 0) = 5.1;
  b(0, 2, 0, 1) = 20.1;
  b(0, 2, 0, 2) = 19.1;
  b(0, 2, 1, 0) = 20.1;
  b(0, 2, 1, 1) = 10.1;
  b(0, 2, 1, 2) = 17.1;
  b(0, 2, 2, 0) = 19.1;
  b(0, 2, 2, 1) = 17.1;
  b(0, 2, 2, 2) = 14.1;
  b(1, 0, 0, 0) = 6.1;
  b(1, 0, 0, 1) = 21.1;
  b(1, 0, 0, 2) = 20.1;
  b(1, 0, 1, 0) = 21.1;
  b(1, 0, 1, 1) = 11.1;
  b(1, 0, 1, 2) = 18.1;
  b(1, 0, 2, 0) = 20.1;
  b(1, 0, 2, 1) = 18.1;
  b(1, 0, 2, 2) = 15.1;
  b(1, 1, 0, 0) = 2.1;
  b(1, 1, 0, 1) = 11.1;
  b(1, 1, 0, 2) = 10.1;
  b(1, 1, 1, 0) = 11.1;
  b(1, 1, 1, 1) = 7.1;
  b(1, 1, 1, 2) = 9.1;
  b(1, 1, 2, 0) = 10.1;
  b(1, 1, 2, 1) = 9.1;
  b(1, 1, 2, 2) = 8.1;
  b(1, 2, 0, 0) = 4.1;
  b(1, 2, 0, 1) = 18.1;
  b(1, 2, 0, 2) = 17.1;
  b(1, 2, 1, 0) = 18.1;
  b(1, 2, 1, 1) = 9.1;
  b(1, 2, 1, 2) = 16.1;
  b(1, 2, 2, 0) = 17.1;
  b(1, 2, 2, 1) = 16.1;
  b(1, 2, 2, 2) = 13.1;
  b(2, 0, 0, 0) = 5.1;
  b(2, 0, 0, 1) = 20.1;
  b(2, 0, 0, 2) = 19.1;
  b(2, 0, 1, 0) = 20.1;
  b(2, 0, 1, 1) = 10.1;
  b(2, 0, 1, 2) = 17.1;
  b(2, 0, 2, 0) = 19.1;
  b(2, 0, 2, 1) = 17.1;
  b(2, 0, 2, 2) = 14.1;
  b(2, 1, 0, 0) = 4.1;
  b(2, 1, 0, 1) = 18.1;
  b(2, 1, 0, 2) = 17.1;
  b(2, 1, 1, 0) = 18.1;
  b(2, 1, 1, 1) = 9.1;
  b(2, 1, 1, 2) = 16.1;
  b(2, 1, 2, 0) = 17.1;
  b(2, 1, 2, 1) = 16.1;
  b(2, 1, 2, 2) = 13.1;
  b(2, 2, 0, 0) = 3.1;
  b(2, 2, 0, 1) = 15.1;
  b(2, 2, 0, 2) = 14.1;
  b(2, 2, 1, 0) = 15.1;
  b(2, 2, 1, 1) = 8.1;
  b(2, 2, 1, 2) = 13.1;
  b(2, 2, 2, 0) = 14.1;
  b(2, 2, 2, 1) = 13.1;
  b(2, 2, 2, 2) = 12.1;
  EXPECT_NEAR(0, (a - b).L2norm(), 1E-5);
}

TEST_F(RankFourTensorTest, anisotropic9)
{

  std::vector<Real> input(9);
  for (size_t i = 0; i < input.size(); ++i)
    input[i] = i + 1.1;

  RankFourTensor a(input, RankFourTensor::symmetric9);
  RankFourTensor b;
  b(0, 0, 0, 0) = 1.1;
  b(0, 0, 0, 1) = 0;
  b(0, 0, 0, 2) = 0;
  b(0, 0, 1, 0) = 0;
  b(0, 0, 1, 1) = 2.1;
  b(0, 0, 1, 2) = 0;
  b(0, 0, 2, 0) = 0;
  b(0, 0, 2, 1) = 0;
  b(0, 0, 2, 2) = 3.1;
  b(0, 1, 0, 0) = 0;
  b(0, 1, 0, 1) = 9.1;
  b(0, 1, 0, 2) = 0;
  b(0, 1, 1, 0) = 9.1;
  b(0, 1, 1, 1) = 0;
  b(0, 1, 1, 2) = 0;
  b(0, 1, 2, 0) = 0;
  b(0, 1, 2, 1) = 0;
  b(0, 1, 2, 2) = 0;
  b(0, 2, 0, 0) = 0;
  b(0, 2, 0, 1) = 0;
  b(0, 2, 0, 2) = 8.1;
  b(0, 2, 1, 0) = 0;
  b(0, 2, 1, 1) = 0;
  b(0, 2, 1, 2) = 0;
  b(0, 2, 2, 0) = 8.1;
  b(0, 2, 2, 1) = 0;
  b(0, 2, 2, 2) = 0;
  b(1, 0, 0, 0) = 0;
  b(1, 0, 0, 1) = 9.1;
  b(1, 0, 0, 2) = 0;
  b(1, 0, 1, 0) = 9.1;
  b(1, 0, 1, 1) = 0;
  b(1, 0, 1, 2) = 0;
  b(1, 0, 2, 0) = 0;
  b(1, 0, 2, 1) = 0;
  b(1, 0, 2, 2) = 0;
  b(1, 1, 0, 0) = 2.1;
  b(1, 1, 0, 1) = 0;
  b(1, 1, 0, 2) = 0;
  b(1, 1, 1, 0) = 0;
  b(1, 1, 1, 1) = 4.1;
  b(1, 1, 1, 2) = 0;
  b(1, 1, 2, 0) = 0;
  b(1, 1, 2, 1) = 0;
  b(1, 1, 2, 2) = 5.1;
  b(1, 2, 0, 0) = 0;
  b(1, 2, 0, 1) = 0;
  b(1, 2, 0, 2) = 0;
  b(1, 2, 1, 0) = 0;
  b(1, 2, 1, 1) = 0;
  b(1, 2, 1, 2) = 7.1;
  b(1, 2, 2, 0) = 0;
  b(1, 2, 2, 1) = 7.1;
  b(1, 2, 2, 2) = 0;
  b(2, 0, 0, 0) = 0;
  b(2, 0, 0, 1) = 0;
  b(2, 0, 0, 2) = 8.1;
  b(2, 0, 1, 0) = 0;
  b(2, 0, 1, 1) = 0;
  b(2, 0, 1, 2) = 0;
  b(2, 0, 2, 0) = 8.1;
  b(2, 0, 2, 1) = 0;
  b(2, 0, 2, 2) = 0;
  b(2, 1, 0, 0) = 0;
  b(2, 1, 0, 1) = 0;
  b(2, 1, 0, 2) = 0;
  b(2, 1, 1, 0) = 0;
  b(2, 1, 1, 1) = 0;
  b(2, 1, 1, 2) = 7.1;
  b(2, 1, 2, 0) = 0;
  b(2, 1, 2, 1) = 7.1;
  b(2, 1, 2, 2) = 0;
  b(2, 2, 0, 0) = 3.1;
  b(2, 2, 0, 1) = 0;
  b(2, 2, 0, 2) = 0;
  b(2, 2, 1, 0) = 0;
  b(2, 2, 1, 1) = 5.1;
  b(2, 2, 1, 2) = 0;
  b(2, 2, 2, 0) = 0;
  b(2, 2, 2, 1) = 0;
  b(2, 2, 2, 2) = 6.1;
  EXPECT_NEAR(0, (a - b).L2norm(), 1E-5);
}

TEST_F(RankFourTensorTest, transposeIj)
{
  const RankFourTensor computed_val = _r4.transposeIj();
  for (unsigned int l = 0; l < 3; ++l)
    for (unsigned int k = 0; k < 3; ++k)
      for (unsigned int j = 0; j < 3; ++j)
        for (unsigned int i = 0; i < 3; ++i)
          EXPECT_NEAR(computed_val(i, j, k, l), _r4(j, i, k, l), 1e-5);
}

TEST_F(RankFourTensorTest, printReal)
{
  std::stringstream ss;
  _r4.printReal(ss);
  const std::string gold = "i = 0 j = 0\n"
                           "              1               2               3 \n"
                           "              4               5               6 \n"
                           "              7               8               9 \n"
                           "i = 0 j = 1\n"
                           "             10              11              12 \n"
                           "             13              14              15 \n"
                           "             16              17              18 \n"
                           "i = 0 j = 2\n"
                           "             19              20              21 \n"
                           "             22              23              24 \n"
                           "             25              26              27 \n"
                           "i = 1 j = 0\n"
                           "             28              29              30 \n"
                           "             31              32              33 \n"
                           "             34              35              36 \n"
                           "i = 1 j = 1\n"
                           "             37              38              39 \n"
                           "             40              41              42 \n"
                           "             43              44              45 \n"
                           "i = 1 j = 2\n"
                           "             46              47              48 \n"
                           "             49              50              51 \n"
                           "             52              53              54 \n"
                           "i = 2 j = 0\n"
                           "             55              56              57 \n"
                           "             58              59              60 \n"
                           "             61              62              63 \n"
                           "i = 2 j = 1\n"
                           "             64              65              66 \n"
                           "             67              68              69 \n"
                           "             70              71              72 \n"
                           "i = 2 j = 2\n"
                           "             73              74              75 \n"
                           "             76              77              78 \n"
                           "             79              80              81 \n";

  EXPECT_EQ(ss.str(), gold);
}

TEST_F(RankFourTensorTest, contractionI)
{
  usingTensorIndices(i_, j_, k_, l_);
  const RankThreeTensor computed_val = _r4.contraction<i_>(_v);
  RankThreeTensor expected_val;
  expected_val.fillFromInputVector({222, 228, 234, 240, 246, 252, 258, 264, 270,
                                    276, 282, 288, 294, 300, 306, 312, 318, 324,
                                    330, 336, 342, 348, 354, 360, 366, 372, 378},
                                   RankThreeTensor::general);
  for (unsigned int k = 0; k < 3; ++k)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int i = 0; i < 3; ++i)
        EXPECT_NEAR(expected_val(i, j, k), computed_val(i, j, k), 1e-5);
}

TEST_F(RankFourTensorTest, contractionJ)
{
  usingTensorIndices(i_, j_, k_, l_);
  const RankThreeTensor computed_val = _r4.contraction<j_>(_v);
  RankThreeTensor expected_val;
  expected_val.fillFromInputVector({78,  84,  90,  96,  102, 108, 114, 120, 126,
                                    240, 246, 252, 258, 264, 270, 276, 282, 288,
                                    402, 408, 414, 420, 426, 432, 438, 444, 450},
                                   RankThreeTensor::general);
  for (unsigned int k = 0; k < 3; ++k)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int i = 0; i < 3; ++i)
        EXPECT_NEAR(expected_val(i, j, k), computed_val(i, j, k), 1e-5);
}

TEST_F(RankFourTensorTest, bignum)
{
  RankFourTensor tensor(
      {0.37187984, 0.1205132,  0.20604735, 0.37231474, 0.43842493, 0.70315511, 0.4290739,
       0.13976259, 0.09908869, 0.30118635, 0.77499901, 0.27357056, 0.60135915, 0.23465929,
       0.22293769, 0.19040275, 0.27290676, 0.74815661, 0.28154392, 0.60755658, 0.03619608,
       0.3624321,  0.11706153, 0.17841735, 0.97943584, 0.09110013, 0.46637981, 0.03828105,
       0.33870729, 0.32737051, 0.77580343, 0.24627582, 0.78159833, 0.9107652,  0.25967326,
       0.78975337, 0.16262543, 0.39379351, 0.76695509, 0.06725889, 0.05830972, 0.35663544,
       0.30705526, 0.10678273, 0.86049446, 0.35409874, 0.11516089, 0.19278114, 0.13198504,
       0.59468892, 0.56816644, 0.12003418, 0.22053863, 0.0982336,  0.62431929, 0.71956444,
       0.21428326, 0.44609002, 0.41339009, 0.42191298, 0.48892364, 0.62091132, 0.18801674,
       0.70655228, 0.19426012, 0.46781494, 0.71600797, 0.60481787, 0.70137889, 0.03859477,
       0.77192629, 0.32630718, 0.75514572, 0.40633981, 0.8421178,  0.75133872, 0.18673922,
       0.74648423, 0.89100271, 0.74446494, 0.65905056},
      RankFourTensor::general);

  constexpr std::size_t derivative_size = 1000;
  typedef NumberArray<derivative_size, Real> DNDerivativeType;
  typedef DualNumber<Real, DNDerivativeType, /*allow_skiping_derivatives=*/true> ADBig;

  RankFourTensorTempl<ADBig> A = tensor;
  RankFourTensorTempl<ADReal> B = tensor;
  RankFourTensorTempl<Real> C = tensor;

  const auto iA = MetaPhysicL::raw_value(A.inverse());
  const auto iB = MetaPhysicL::raw_value(B.inverse());
  const auto iC = MetaPhysicL::raw_value(C.inverse());

  EXPECT_NEAR(MetaPhysicL::raw_value((iA - iB).L2norm()), 0.0, 1e-9);
  EXPECT_NEAR(MetaPhysicL::raw_value((iB - iC).L2norm()), 0.0, 1e-9);
}
