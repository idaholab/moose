//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include <torch/torch.h>
#include "LibtorchUtils.h"
#include "LibtorchVectorConversionTest.h"

registerMooseObject("MooseTestApp", LibtorchVectorConversionTest);

InputParameters
LibtorchVectorConversionTest::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  return params;
}

LibtorchVectorConversionTest::LibtorchVectorConversionTest(const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _vector_from_tensor(declareVector("vector_from_tensor")),
    _tensor_from_vector(declareVector("tensor_from_vector"))
{
  torch::manual_seed(17);
  torch::Tensor tensor = torch::randn({5, 1}, at::kDouble);
  std::vector<Real> vector;
  LibtorchUtils::tensorToVector(tensor, vector);
  for (const auto & item : vector)
    _vector_from_tensor.push_back(item);

  torch::Tensor new_tensor;
  LibtorchUtils::vectorToTensor(vector, new_tensor);
  int size = new_tensor.sizes()[0];

  for (const auto i : make_range(size))
    _tensor_from_vector.push_back(new_tensor[i][0].item<Real>());
}

#endif
