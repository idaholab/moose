//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "gtest/gtest.h"
#include "LibtorchUtils.h"

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

#endif
