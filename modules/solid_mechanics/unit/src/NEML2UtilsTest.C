//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "NEML2Utils.h"

#ifdef NEML2_ENABLED
TEST(NEML2Utils, fromBlob_Real)
{
  std::vector<Real> data(3);
  data[0] = 1.0;
  data[1] = 2.0;
  data[2] = 3.0;

  auto tensor = NEML2Utils::fromBlob(data);
  ASSERT_TRUE(tensor.defined());
  ASSERT_TRUE(tensor.dim() == 1);
  ASSERT_TRUE(tensor.batch_dim() == 1);
  ASSERT_TRUE(tensor.base_dim() == 0);
  ASSERT_TRUE(tensor.size(0) == 3);

  for (neml2::Size n : index_range(data))
    EXPECT_NEAR(tensor.index({n}).item<Real>(), data[n], 1e-12);
}

TEST(NEML2Utils, fromBlob_RealVectorValue)
{
  std::vector<RealVectorValue> data(3);
  data[0] = RealVectorValue(1.0, 2.0, 3.0);
  data[1] = RealVectorValue(4.0, 5.0, 6.0);
  data[2] = RealVectorValue(7.0, 8.0, 9.0);

  auto tensor = NEML2Utils::fromBlob(data);
  ASSERT_TRUE(tensor.defined());
  ASSERT_TRUE(tensor.dim() == 2);
  ASSERT_TRUE(tensor.batch_dim() == 1);
  ASSERT_TRUE(tensor.base_dim() == 1);
  ASSERT_TRUE(tensor.size(0) == 3);
  ASSERT_TRUE(tensor.size(1) == 3);

  for (neml2::Size n : index_range(data))
    for (neml2::Size i : make_range(3))
      EXPECT_NEAR(tensor.index({n, i}).item<Real>(), data[n](i), 1e-12);
}

TEST(NEML2Utils, fromBlob_RankTwoTensor)
{
  std::vector<RankTwoTensor> data(2);
  data[0] = RankTwoTensor(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
  data[1] = RankTwoTensor(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0, -7.0, -8.0, -9.0);

  auto tensor = NEML2Utils::fromBlob(data);
  ASSERT_TRUE(tensor.defined());
  ASSERT_TRUE(tensor.dim() == 3);
  ASSERT_TRUE(tensor.batch_dim() == 1);
  ASSERT_TRUE(tensor.base_dim() == 2);
  ASSERT_TRUE(tensor.size(0) == 2);
  ASSERT_TRUE(tensor.size(1) == 3);
  ASSERT_TRUE(tensor.size(2) == 3);

  for (neml2::Size n : index_range(data))
    for (neml2::Size i : make_range(3))
      for (neml2::Size j : make_range(3))
        EXPECT_NEAR(tensor.index({n, i, j}).item<Real>(), data[n](i, j), 1e-12);
}

TEST(NEML2Utils, fromBlob_SymmetricRankTwoTensor)
{
  std::vector<SymmetricRankTwoTensor> data(2);
  data[0] = SymmetricRankTwoTensor(1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
  data[1] = SymmetricRankTwoTensor(-1.0, -2.0, -3.0, -4.0, -5.0, -6.0);

  auto tensor = NEML2Utils::fromBlob(data);
  ASSERT_TRUE(tensor.defined());
  ASSERT_TRUE(tensor.dim() == 2);
  ASSERT_TRUE(tensor.batch_dim() == 1);
  ASSERT_TRUE(tensor.base_dim() == 1);
  ASSERT_TRUE(tensor.size(0) == 2);
  ASSERT_TRUE(tensor.size(1) == 6);

  for (neml2::Size n : index_range(data))
    for (neml2::Size i : make_range(6))
      EXPECT_NEAR(tensor.index({n, i}).item<Real>(), data[n](i), 1e-12);
}

TEST(NEML2Utils, copyTensorToMOOSEData_Real)
{
  Real data;
  const auto tensor = torch::tensor(1.0, torch::TensorOptions().dtype(torch::kFloat64));
  NEML2Utils::copyTensorToMOOSEData(tensor, data);

  EXPECT_NEAR(tensor.item<Real>(), data, 1e-12);
}

TEST(NEML2Utils, copyTensorToMOOSEData_RealVectorValue)
{
  RealVectorValue data;
  const auto tensor = torch::tensor({1.0, 2.0, 3.0}, torch::TensorOptions().dtype(torch::kFloat64));
  NEML2Utils::copyTensorToMOOSEData(tensor, data);

  for (neml2::Size i : make_range(3))
    EXPECT_NEAR(tensor.index({i}).item<Real>(), data(i), 1e-12);
}

TEST(NEML2Utils, copyTensorToMOOSEData_RankTwoTensor)
{
  RankTwoTensor data;
  const auto tensor = torch::tensor({{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}},
                                    torch::TensorOptions().dtype(torch::kFloat64));
  NEML2Utils::copyTensorToMOOSEData(tensor, data);

  for (neml2::Size i : make_range(3))
    for (neml2::Size j : make_range(3))
      EXPECT_NEAR(tensor.index({i, j}).item<Real>(), data(i, j), 1e-12);
}

TEST(NEML2Utils, copyTensorToMOOSEData_SymmetricRankTwoTensor)
{
  SymmetricRankTwoTensor data;
  const auto tensor =
      torch::tensor({1.0, 2.0, 3.0, 4.0, 5.0, 6.0}, torch::TensorOptions().dtype(torch::kFloat64));
  NEML2Utils::copyTensorToMOOSEData(tensor, data);

  for (neml2::Size i : make_range(6))
    EXPECT_NEAR(tensor.index({i}).item<Real>(), data(i), 1e-12);
}

TEST(NEML2Utils, copyTensorToMOOSEData_RankFourTensor)
{
  RankFourTensor data;
  const auto tensor =
      torch::tensor({{{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}},
                      {{10.0, 11.0, 12.0}, {13.0, 14.0, 15.0}, {16.0, 17.0, 18.0}},
                      {{19.0, 20.0, 21.0}, {22.0, 23.0, 24.0}, {25.0, 26.0, 27.0}}},
                     {{{28.0, 29.0, 30.0}, {31.0, 32.0, 33.0}, {34.0, 35.0, 36.0}},
                      {{37.0, 38.0, 39.0}, {40.0, 41.0, 42.0}, {43.0, 44.0, 45.0}},
                      {{46.0, 47.0, 48.0}, {49.0, 50.0, 51.0}, {52.0, 53.0, 54.0}}},
                     {{{55.0, 56.0, 57.0}, {58.0, 59.0, 60.0}, {61.0, 62.0, 63.0}},
                      {{64.0, 65.0, 66.0}, {67.0, 68.0, 69.0}, {70.0, 71.0, 72.0}},
                      {{73.0, 74.0, 75.0}, {76.0, 77.0, 78.0}, {79.0, 80.0, 81.0}}}},
                    torch::TensorOptions().dtype(torch::kFloat64));
  NEML2Utils::copyTensorToMOOSEData(tensor, data);

  for (neml2::Size i : make_range(3))
    for (neml2::Size j : make_range(3))
      for (neml2::Size k : make_range(3))
        for (neml2::Size l : make_range(3))
          EXPECT_NEAR(tensor.index({i, j, k, l}).item<Real>(), data(i, j, k, l), 1e-12);
}

TEST(NEML2Utils, copyTensorToMOOSEData_SymmetricRankFourTensor)
{
  SymmetricRankFourTensor data;
  const auto tensor = torch::tensor({{1.0, 2.0, 3.0, 4.0, 5.0, 6.0},
                                     {2.0, 3.0, 4.0, 5.0, 6.0, 7.0},
                                     {3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
                                     {4.0, 5.0, 6.0, 7.0, 8.0, 9.0},
                                     {5.0, 6.0, 7.0, 8.0, 9.0, 10.0},
                                     {6.0, 7.0, 8.0, 9.0, 10.0, 11.0}},
                                    torch::TensorOptions().dtype(torch::kFloat64));
  NEML2Utils::copyTensorToMOOSEData(tensor, data);

  for (neml2::Size i : make_range(6))
    for (neml2::Size j : make_range(6))
      EXPECT_NEAR(tensor.index({i, j}).item<Real>(), data(i, j), 1e-12);
}
#endif
