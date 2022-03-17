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
  params.addParam<unsigned int>("no_batches", 1, "Number of batches.");
  params.addParam<unsigned int>("no_epochs", 1, "Number of epochs.");
  params.addParam<unsigned int>("no_hidden_layers", 0, "Number of hidden layers.");
  params.addParam<std::vector<unsigned int>>(
      "no_neurons_per_layer", std::vector<unsigned int>(), "Number of neurons per layer.");
  params.addParam<std::string>(
      "filename", "net.pt", "Filename used to output the neural net parameters.");
  params.addParam<bool>("read_from_file",
                        false,
                        "Switch to allow reading old trained neural nets for further training.");
  params.addParam<Real>("learning_rate", 0.001, "Learning rate (relaxation).");
  params.addParam<unsigned int>(
      "seed", 11, "Random number generator seed for stochastic optimizers.");

  return params;
}

LibtorchSimpleNNTrainer::LibtorchSimpleNNTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sampler_row(getSamplerData()),
    _response(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _no_batches(getParam<unsigned int>("no_batches")),
    _no_epocs(getParam<unsigned int>("no_epochs")),
    _no_hidden_layers(declareModelData<unsigned int>("no_hidden_layers")),
    _no_neurons_per_layer(declareModelData<std::vector<unsigned int>>("no_neurons_per_layer")),
    _filename(getParam<std::string>("filename")),
    _read_from_file(getParam<bool>("read_from_file")),
    _learning_rate(getParam<Real>("learning_rate"))
#ifdef TORCH_ENABLED
    ,
    _nn(declareModelData<std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet>>("nn"))
#endif
{
  if (_no_hidden_layers != _no_neurons_per_layer.size())
    mooseError("The number of layers are not the same!");

  _no_hidden_layers = getParam<unsigned int>("no_hidden_layers");
  _no_neurons_per_layer = getParam<std::vector<unsigned int>>("no_neurons_per_layer");
  _filename = getParam<std::string>("filename");

#ifdef TORCH_ENABLED
  // Fixing the RNG seed to make sure every experiment is the same.
  // Otherwise sampling / stochastic gradient descent would be different.
  torch::manual_seed(getParam<unsigned int>("seed"));
#endif
}

void
LibtorchSimpleNNTrainer::preTrain()
{
  // Resize to number of sample points
  _flattened_data.resize(_sampler.getNumberOfLocalRows() * _sampler.getNumberOfCols());
  _flattened_response.resize(_sampler.getNumberOfLocalRows());
}

void
LibtorchSimpleNNTrainer::train()
{
  unsigned int no_parameters = _sampler.getNumberOfCols();

  for (unsigned int i = 0; i < no_parameters; ++i)
    _flattened_data[_local_row * no_parameters + i] = _sampler_row[i];

  _flattened_response[_local_row] = _response;
}

void
LibtorchSimpleNNTrainer::postTrain()
{
  _communicator.allgather(_flattened_data);
  _communicator.allgather(_flattened_response);

#ifdef TORCH_ENABLED

  // Then, we create and load our Tensors
  unsigned int n_rows = _flattened_response.size();
  unsigned int n_cols = _sampler.getNumberOfCols();

  // The default data type in pytorch is float, while we use double in MOOSE.
  // Therefore, in some cases we have to convert Tensors to double.
  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor data_tensor =
      torch::from_blob(_flattened_data.data(), {n_rows, n_cols}, options).to(at::kDouble);
  torch::Tensor response_tensor =
      torch::from_blob(_flattened_response.data(), {n_rows}, options).to(at::kDouble);

  // We create a custom data loader which can be used to select samples for the in
  // the training process. See the header file for the definition of this structure.
  MyData my_data(data_tensor, response_tensor);

  // We initialize a data_loader for the training part.
  unsigned int sample_per_batch = n_rows > _no_batches ? n_rows / _no_batches : 1;

  auto data_set = my_data.map(torch::data::transforms::Stack<>());
  auto data_loader = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(
      std::move(data_set), sample_per_batch);

  // We create a neural net (for the definition of the net see the header file)
  _nn = std::make_shared<StochasticTools::LibtorchSimpleNeuralNet>(
      _filename, n_cols, _no_hidden_layers, _no_neurons_per_layer, 1);

  // Initialize the optimizer
  torch::optim::Adam optimizer(_nn->parameters(), torch::optim::AdamOptions(_learning_rate));

  if (_read_from_file)
    try
    {
      torch::load(_nn, _filename);
      _console << "Loaded requested .pt file." << std::endl;
    }
    catch (...)
    {
      mooseError("The requested pytorch file could not be loaded.");
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
      torch::Tensor prediction = _nn->forward(batch.data);

      // Compute loss values using a MSE ( mean squared error)
      torch::Tensor loss = torch::mse_loss(prediction.reshape({prediction.size(0)}), batch.target);

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

#ifdef TORCH_ENABLED
template <>
void
dataStore(std::ostream & stream,
          std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
          void * context)
{
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
}

template <>
void
dataLoad(std::istream & stream,
         std::shared_ptr<StochasticTools::LibtorchSimpleNeuralNet> & nn,
         void * context)
{
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
}
#endif
