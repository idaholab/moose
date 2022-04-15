//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED
#include "LibtorchArtificialNeuralNet.h"
#include <torch/torch.h>
#endif

#include "LibtorchArtificialNeuralNetTest.h"
#include "ThreadedGeneralUserObject.h"

registerMooseObject("MooseTestApp", LibtorchArtificialNeuralNetTest);

InputParameters
LibtorchSimpleNeuralNetTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<std::vector<std::string>>(
      "activation_functions", std::vector<std::string>({"relu"}), "Test activation functions");

  return params;
}

LibtorchSimpleNeuralNetTest::LibtorchSimpleNeuralNetTest(const InputParameters & params)
  : GeneralUserObject(params)
{
#ifdef LIBTORCH_ENABLED
  std::vector<unsigned int> num_neurons_per_layer({4, 4});
  torch::Tensor input = at::ones({1, 3}, at::kDouble);
  torch::Tensor output = at::ones({1}, at::kDouble);

  torch::manual_seed(11);

  std::shared_ptr<Moose::LibtorchArtificialNeuralNet> nn =
      std::make_shared<Moose::LibtorchArtificialNeuralNet>(
          "test",
          3,
          1,
          num_neurons_per_layer,
          getParam<std::vector<std::string>>("activation_functions"));

  torch::optim::Adam optimizer(nn->parameters(), torch::optim::AdamOptions(0.02));

  optimizer.zero_grad();
  torch::Tensor prediction = nn->forward(input);
  torch::Tensor loss = torch::mse_loss(prediction.reshape({prediction.size(0)}), output);

  loss.backward();
  optimizer.step();

  optimizer.zero_grad();
  prediction = nn->forward(input);

  _console << "My prediction: " << prediction << std::endl;

#endif
}
