//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "gtest/gtest.h"
#include "LibtorchUtils.h"
#include "MooseError.h"

TEST(LibtorchUtilsTest, vectorConversion)
{
  torch::manual_seed(17);
  torch::Tensor tensor = torch::randn({5, 1}, at::kDouble);
  std::vector<Real> vector;
  LibtorchUtils::tensorToVector(tensor, vector);

  torch::Tensor new_tensor;
  LibtorchUtils::vectorToTensor(vector, new_tensor);

  std::vector<Real> tensor_from_vector(new_tensor.data_ptr<Real>(),
                                       new_tensor.data_ptr<Real>() + new_tensor.numel());

  EXPECT_EQ(vector, tensor_from_vector);
}

TEST(LibtorchUtilsTest, vectorToTensorView)
{
  std::vector<Real> vector{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
  auto tensor = LibtorchUtils::vectorToTensorView(vector, {2, 3});

  EXPECT_EQ(tensor.scalar_type(), at::kDouble);
  EXPECT_EQ(tensor.dim(), 2);
  EXPECT_EQ(tensor.size(0), 2);
  EXPECT_EQ(tensor.size(1), 3);
  EXPECT_EQ(tensor.data_ptr<Real>(), vector.data());

  auto values = tensor.accessor<Real, 2>();
  EXPECT_EQ(values[0][0], 1.0);
  EXPECT_EQ(values[1][2], 6.0);

  values[0][1] = 7.0;
  EXPECT_EQ(vector[1], 7.0);

  vector[4] = 8.0;
  EXPECT_EQ(values[1][1], 8.0);
}

TEST(LibtorchUtilsTest, vectorToTensorViewEmpty)
{
  std::vector<Real> vector;
  auto tensor = LibtorchUtils::vectorToTensorView(vector, {0, 2});

  EXPECT_EQ(tensor.scalar_type(), at::kDouble);
  EXPECT_EQ(tensor.dim(), 2);
  EXPECT_EQ(tensor.size(0), 0);
  EXPECT_EQ(tensor.size(1), 2);
  EXPECT_EQ(tensor.numel(), 0);
}

TEST(LibtorchUtilsTest, vectorToTensorViewInvalidShape)
{
  std::vector<Real> vector{1.0, 2.0, 3.0};

  EXPECT_THROW(LibtorchUtils::vectorToTensorView(vector, {2, 2}), MooseRuntimeError);
}

#endif
