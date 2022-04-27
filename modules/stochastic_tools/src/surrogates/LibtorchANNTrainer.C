//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LibtorchANNTrainer.h"
#include "LibtorchDataset.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", LibtorchANNTrainer);

InputParameters
LibtorchANNTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Trains a simple neural network using libtorch.");

  MooseEnum data_type("real=0 vector_real=1", "real");
  params.addRequiredParam<ReporterName>(
      "response",
      "Reporter value of response results, can be vpp with <vpp_name>/<vector_name> or sampler "
      "column with 'sampler/col_<index>'.");
  params.addParam<MooseEnum>("response_type", data_type, "Response data type.");
  params.addRangeCheckedParam<unsigned int>(
      "num_batches", 1, "1<=num_batches", "Number of batches.");
  params.addRangeCheckedParam<unsigned int>(
      "num_epochs", 1, "0<num_epochs", "Number of training epochs.");
  params.addRangeCheckedParam<Real>(
      "rel_loss_tol",
      0,
      "0<=rel_loss_tol<=1",
      "The relative loss where we stop the training of the neural net.");
  params.addParam<std::vector<unsigned int>>(
      "num_neurons_per_layer", std::vector<unsigned int>(), "Number of neurons per layer.");
  params.addParam<std::vector<std::string>>(
      "activation_function",
      std::vector<std::string>({"relu"}),
      "The type of activation functions to use. It is either one value "
      "or one value per hidden layer.");
  params.addParam<std::string>(
      "filename", "net.pt", "Filename used to output the neural net parameters.");
  params.addParam<bool>("read_from_file",
                        false,
                        "Switch to allow reading old trained neural nets for further training.");
  params.addParam<Real>("learning_rate", 0.001, "Learning rate (relaxation).");
  params.addRangeCheckedParam<unsigned int>(
      "print_epoch_loss",
      0,
      "0<=print_epoch_loss",
      "Epoch training loss printing. 0 - no printing, 1 - every epoch, 10 - every 10th epoch.");
  params.addParam<unsigned int>(
      "seed", 11, "Random number generator seed for stochastic optimizers.");

  return params;
}

LibtorchANNTrainer::LibtorchANNTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sampler_row(getSamplerData()),
    _response(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _num_batches(getParam<unsigned int>("num_batches")),
    _num_epocs(getParam<unsigned int>("num_epochs")),
    _rel_loss_tol(getParam<Real>("rel_loss_tol")),
    _num_neurons_per_layer(declareModelData<std::vector<unsigned int>>(
        "num_neurons_per_layer", getParam<std::vector<unsigned int>>("num_neurons_per_layer"))),
    _num_hidden_layers(
        declareModelData<unsigned int>("num_hidden_layers", _num_neurons_per_layer.size())),
    _activation_function(declareModelData<std::vector<std::string>>(
        "activation_function", getParam<std::vector<std::string>>("activation_function"))),
    _filename(getParam<std::string>("filename")),
    _read_from_file(getParam<bool>("read_from_file")),
    _learning_rate(getParam<Real>("learning_rate")),
    _print_epoch_loss(getParam<unsigned int>("print_epoch_loss"))
#ifdef LIBTORCH_ENABLED
    ,
    _nn(declareModelData<std::shared_ptr<Moose::LibtorchArtificialNeuralNet>>("nn"))
#endif
{
  // We check if MOOSE is compiled with torch, if not this throws an error
  StochasticToolsApp::requiresTorch(*this);

#ifdef LIBTORCH_ENABLED
  // Fixing the RNG seed to make sure every experiment is the same.
  // Otherwise sampling / stochastic gradient descent would be different.
  torch::manual_seed(getParam<unsigned int>("seed"));
#endif
}

void
LibtorchANNTrainer::preTrain()
{
  // Resize to number of sample points
  _flattened_data.resize(_sampler.getNumberOfLocalRows() * _sampler.getNumberOfCols());
  _flattened_response.resize(_sampler.getNumberOfLocalRows());
}

void
LibtorchANNTrainer::train()
{
  unsigned int num_parameters = _sampler.getNumberOfCols();

  for (unsigned int i = 0; i < num_parameters; ++i)
    _flattened_data[_local_row * num_parameters + i] = _sampler_row[i];

  _flattened_response[_local_row] = _response;
}

void
LibtorchANNTrainer::postTrain()
{
  _communicator.allgather(_flattened_data);
  _communicator.allgather(_flattened_response);

#ifdef LIBTORCH_ENABLED

  // Then, we create and load our Tensors
  unsigned int num_samples = _flattened_response.size();
  unsigned int num_inputs = _sampler.getNumberOfCols();

  // The default data type in pytorch is float, while we use double in MOOSE.
  // Therefore, in some cases we have to convert Tensors to double.
  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor data_tensor =
      torch::from_blob(_flattened_data.data(), {num_samples, num_inputs}, options).to(at::kDouble);
  torch::Tensor response_tensor =
      torch::from_blob(_flattened_response.data(), {num_samples}, options).to(at::kDouble);

  // We create a custom data loader which can be used to select samples for the in
  // the training process. See the header file for the definition of this structure.
  Moose::LibtorchDataset my_data(data_tensor, response_tensor);

  // We initialize a data_loader for the training part.
  unsigned int sample_per_batch = num_samples > _num_batches ? num_samples / _num_batches : 1;

  auto data_set = my_data.map(torch::data::transforms::Stack<>());
  auto data_loader = torch::data::make_data_loader<torch::data::samplers::SequentialSampler>(
      std::move(data_set), sample_per_batch);

  // We create a neural net (for the definition of the net see the header file)
  _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      _filename, num_inputs, 1, _num_neurons_per_layer, _activation_function);

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

  Real rel_loss = 1.0;
  Real initial_loss = 1.0;
  Real epoch_loss = 0.0;

  // Begin training loop
  unsigned int epoch = 1;
  while (epoch <= _num_epocs && rel_loss > _rel_loss_tol)
  {
    epoch_loss = 0.0;
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

      epoch_loss += loss.item<double>();
    }

    epoch_loss = epoch_loss / _num_batches;

    if (epoch == 1)
      initial_loss = epoch_loss;

    rel_loss = epoch_loss / initial_loss;

    if (_print_epoch_loss)
      if (epoch % _print_epoch_loss == 0 || epoch == 1)
        _console << "Epoch: " << epoch << " | Loss: " << COLOR_GREEN << epoch_loss << COLOR_DEFAULT
                 << " | Rel. loss: " << COLOR_GREEN << rel_loss << COLOR_DEFAULT << std::endl;
    epoch += 1;
  }

#endif
}
