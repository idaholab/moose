//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BasicNNTrainer.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", BasicNNTrainer);

InputParameters
BasicNNTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Computes coefficients for polynomial regession model.");

  MooseEnum data_type("real=0 vector_real=1", "real");
  params.addRequiredParam<ReporterName>(
      "response",
      "Reporter value of response results, can be vpp with <vpp_name>/<vector_name> or sampler "
      "column with 'sampler/col_<index>'.");
  params.addParam<MooseEnum>("response_type", data_type, "Response data type.");
  params.addParam<std::vector<ReporterName>>(
      "predictors",
      std::vector<ReporterName>(),
      "Reporter values used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<std::vector<unsigned int>>(
      "predictor_cols",
      std::vector<unsigned int>(),
      "Sampler columns used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<unsigned int>("no_batches", 1, "Number of batches.");
  params.addParam<unsigned int>("no_epochs", 1, "Number of epochs.");
  params.addParam<unsigned int>("no_hidden_layers", 0, "Number of hidden layers.");
  params.addParam<std::vector<unsigned int>>(
      "no_neurons_per_layer", std::vector<unsigned int>(), "Number of neurons per layer.");
  params.addParam<Real>("learning_rate", 0.001, "Learning rate (relaxation).");

  return params;
}

BasicNNTrainer::BasicNNTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sampler_row(getSamplerData()),
    _sample_points(declareModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _response(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _no_batches(getParam<unsigned int>("no_batches")),
    _no_epocs(getParam<unsigned int>("no_epochs")),
    _no_hidden_layers(getParam<unsigned int>("no_hidden_layers")),
    _no_neurons_per_layer(getParam<std::vector<unsigned int>>("no_neurons_per_layer")),
    _learning_rate(getParam<Real>("learning_rate"))
{
  const auto & pnames = getParam<std::vector<ReporterName>>("predictors");
  for (unsigned int i = 0; i < pnames.size(); ++i)
    _pvals[i] = &getTrainingData<Real>(pnames[i]);

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_pvals.empty() && _pcols.empty())
  {
    _pcols.resize(_sampler.getNumberOfCols());
    std::iota(_pcols.begin(), _pcols.end(), 0);
  }

  // Resize sample points to number of predictors
  _sample_points.resize(_pvals.size() + _pcols.size() + 1);

  if (_no_hidden_layers != _no_neurons_per_layer.size())
    mooseError("The number of layers are not the same!");
}

void
BasicNNTrainer::preTrain()
{
  // Resize to number of sample points
  for (auto & it : _sample_points)
    it.resize(_sampler.getNumberOfLocalRows());
}

void
BasicNNTrainer::train()
{
  unsigned int d = 0;
  // Get predictors from reporter values
  for (const auto & val : _pvals)
    _sample_points[d++][_local_row] = *val;
  // Get predictors from sampler
  for (const auto & col : _pcols)
    _sample_points[d++][_local_row] = _sampler_row[col];

  _sample_points.back()[_local_row] = _response;
}

void
BasicNNTrainer::postTrain()
{
  for (auto & it : _sample_points)
    _communicator.allgather(it);

#ifdef ENABLE_PT

  // Linearizing the data to make sure we can stor it in a tensor

  // Fixing the RNG seed to make sure every experiment is the same.
  // Otherwise sampling / stochastic gradient descent would be different.
  torch::manual_seed(11);

  // Now we convert the internal data-structure to torch::Tensors
  std::vector<Real> flattened_data;
  std::vector<Real> flattened_response;

  // First, we flatten (serialize) out containers
  for (const auto & r : _sample_points)
  {
    if (&r == &_sample_points.back())
      for (const auto & c : r)
        flattened_response.push_back(c);
    else
      for (const auto & c : r)
        flattened_data.push_back(c);
  }

  // Then, we create and load our Tensors
  int n_rows = _sample_points.size() - 1;
  int n_cols = _sample_points[0].size();

  // The default data type in pytorch is float, while we use double in MOOSE.
  // Therefore, in some cases we have to convert Tensors to double.
  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor data_tensor =
      torch::from_blob(flattened_data.data(), {n_rows, n_cols}, options).to(at::kDouble);
  torch::Tensor response_tensor =
      torch::from_blob(flattened_response.data(), {n_cols}, options).to(at::kDouble);

  // We create a custom data loader which can be used to select samples for the in
  // the training process. See the header file for the definition of this structure.
  MyData my_data(data_tensor, response_tensor);

  // We initialize a data_loader for the training part.
  unsigned int sample_per_batch = n_cols > _no_batches ? n_cols / _no_batches : 1;
  auto data_loader = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(
      std::move(my_data.map(torch::data::transforms::Stack<>())), sample_per_batch);

  // We create a neural net (for the definition of the net see the header file)
  auto net = std::make_shared<MyNet>(n_rows, _no_hidden_layers, _no_neurons_per_layer, 1);

  // Initialize the optimizer
  torch::optim::Adam optimizer(net->parameters(), torch::optim::AdamOptions(_learning_rate));

  // Begin training loop
  for (size_t epoch = 1; epoch <= _no_epocs; ++epoch)
  {
    size_t batch_index = 0;
    Real epoch_error = 0.0;
    for (auto & batch : *data_loader)
    {
      // Reset gradients
      optimizer.zero_grad();

      // Compute prediction
      torch::Tensor prediction = net->forward(torch::transpose(batch.data, 1, 2));

      // Compute loss values using a MSE ( mean squared error)
      torch::Tensor loss = torch::mse_loss(prediction, batch.target);

      // Propagate error back
      loss.backward();

      // Use new gradients to update the parameters
      optimizer.step();

      epoch_error += loss.item<double>();
    }

    if (epoch % 10 == 0)
      std::cout << "Epoch: " << epoch << " | Loss: " << epoch_error << std::endl;
  }
#endif
}
