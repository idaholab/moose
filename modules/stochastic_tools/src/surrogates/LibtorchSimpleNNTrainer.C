//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibtorchSimpleNNTrainer.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", LibtorchSimpleNNTrainer);

InputParameters
LibtorchSimpleNNTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Trains a simple neural network using libtorch.");

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
  params.addParam<std::string>(
      "filename", "net.pt", "Filename used to output the neural net parameters.");
  params.addParam<Real>("learning_rate", 0.001, "Learning rate (relaxation).");

  return params;
}

LibtorchSimpleNNTrainer::LibtorchSimpleNNTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sampler_row(getSamplerData()),
    _sample_points(declareModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _response(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _no_batches(getParam<unsigned int>("no_batches")),
    _no_epocs(getParam<unsigned int>("no_epochs")),
    _no_hidden_layers(declareModelData<unsigned int>("no_hidden_layers")),
    _no_neurons_per_layer(declareModelData<std::vector<unsigned int>>("no_neurons_per_layer")),
    _filename(getParam<std::string>("filename")),
    _learning_rate(getParam<Real>("learning_rate")),
    _nn(declareModelData<std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet>>("nn"))
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

  _no_hidden_layers = getParam<unsigned int>("no_hidden_layers");
  _no_neurons_per_layer = getParam<std::vector<unsigned int>>("no_neurons_per_layer");
  _filename = getParam<std::string>("filename");
}

void
LibtorchSimpleNNTrainer::preTrain()
{
  // Resize to number of sample points
  for (auto & it : _sample_points)
    it.resize(_sampler.getNumberOfLocalRows());
}

void
LibtorchSimpleNNTrainer::train()
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
LibtorchSimpleNNTrainer::postTrain()
{
  for (auto & it : _sample_points)
    _communicator.allgather(it);

#ifdef TORCH_ENABLED

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
  unsigned int n_rows = _sample_points.size() - 1;
  unsigned int n_cols = _sample_points[0].size();

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
  auto data_set = my_data.map(torch::data::transforms::Stack<>());
  auto data_loader = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(
      std::move(data_set), sample_per_batch);

  // We create a neural net (for the definition of the net see the header file)
  _nn = std::make_shared<StochasticTools::LibtorchSimpleNeuralNet>(
      _filename, n_rows, _no_hidden_layers, _no_neurons_per_layer, 1);

  // Initialize the optimizer
  torch::optim::Adam optimizer(_nn->parameters(), torch::optim::AdamOptions(_learning_rate));

  try
  {
    torch::load(_nn, _filename);
    _console << "Loaded requested .pt file." << std::endl;
  }
  catch (...)
  {
    _console << "The requested .pt file does not exist, training a new neural net." << std::endl;
  }

  // Begin training loop
  for (size_t epoch = 1; epoch <= _no_epocs; ++epoch)
  {
    Real epoch_error = 0.0;
    for (auto & batch : *data_loader)
    {
      // Reset gradients
      optimizer.zero_grad();

      // Compute prediction
      torch::Tensor prediction = _nn->forward(torch::transpose(batch.data, 1, 2));

      // Compute loss values using a MSE ( mean squared error)
      torch::Tensor loss = torch::mse_loss(prediction, batch.target);

      // Propagate error back
      loss.backward();

      // Use new gradients to update the parameters
      optimizer.step();

      epoch_error += loss.item<double>();
    }

    epoch_error = epoch_error / _no_batches;

    if (epoch % 10 == 0)
      _console << "Epoch: " << epoch << " | Loss: " << COLOR_GREEN << epoch_error << COLOR_DEFAULT
               << std::endl;
  }

#endif
}

template <>
void
dataStore(std::ostream & stream,
          std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
          void * context)
{
#ifdef TORCH_ENABLED

  std::string n(nn->name());
  dataStore(stream, n, context);

  unsigned int ni(nn->noInputs());
  dataStore(stream, ni, context);

  unsigned int nhl(nn->noHiddenLayers());
  dataStore(stream, nhl, context);

  std::vector<unsigned int> nnpl(nn->noNeuronsPerLayer());
  dataStore(stream, nnpl, context);

  unsigned int no(nn->noOutputs());
  dataStore(stream, no, context);

  torch::save(nn, nn->name());
#endif
}

template <>
void
dataLoad(std::istream & stream,
         std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
         void * context)
{
#ifdef TORCH_ENABLED
  std::string name;
  dataLoad(stream, name, context);

  unsigned int no_inputs;
  dataLoad(stream, no_inputs, context);

  unsigned int no_hidden_layers;
  dataLoad(stream, no_hidden_layers, context);

  std::vector<unsigned int> no_neurons_per_layer;
  no_neurons_per_layer.resize(no_hidden_layers);
  dataLoad(stream, no_neurons_per_layer, context);

  unsigned int no_outputs;
  dataLoad(stream, no_outputs, context);

  nn = std::make_shared<StochasticTools::LibtorchSimpleNeuralNet>(
      name, no_inputs, no_hidden_layers, no_neurons_per_layer, no_outputs);

  torch::load(nn, name);
#endif
}
