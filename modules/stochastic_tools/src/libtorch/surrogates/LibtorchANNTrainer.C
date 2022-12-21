//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchANNTrainer.h"
#include "LibtorchDataset.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", LibtorchANNTrainer);

InputParameters
LibtorchANNTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Trains a simple neural network using libtorch.");

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
  params.addParam<unsigned int>(
      "max_processes", 1, "The maximum number of parallel processes that the trainer will use.");

  params.suppressParameter<MooseEnum>("response_type");
  return params;
}

LibtorchANNTrainer::LibtorchANNTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _predictor_row(getPredictorData()),
    _num_neurons_per_layer(declareModelData<std::vector<unsigned int>>(
        "num_neurons_per_layer", getParam<std::vector<unsigned int>>("num_neurons_per_layer"))),
    _activation_function(declareModelData<std::vector<std::string>>(
        "activation_function", getParam<std::vector<std::string>>("activation_function"))),
    _filename(getParam<std::string>("filename")),
    _read_from_file(getParam<bool>("read_from_file")),
    _nn(declareModelData<std::shared_ptr<Moose::LibtorchArtificialNeuralNet>>("nn"))
{
  // Fixing the RNG seed to make sure every experiment is the same.
  // Otherwise sampling / stochastic gradient descent would be different.
  torch::manual_seed(getParam<unsigned int>("seed"));

  _optim_options.optimizer_type = "adam";
  _optim_options.learning_rate = getParam<Real>("learning_rate");
  _optim_options.num_epochs = getParam<unsigned int>("num_epochs");
  _optim_options.num_batches = getParam<unsigned int>("num_batches");
  _optim_options.rel_loss_tol = getParam<Real>("rel_loss_tol");
  _optim_options.print_loss = getParam<unsigned int>("print_epoch_loss") > 0;
  _optim_options.print_epoch_loss = getParam<unsigned int>("print_epoch_loss");
  _optim_options.parallel_processes = getParam<unsigned int>("max_processes");
}

void
LibtorchANNTrainer::preTrain()
{
  // Resize to number of sample points
  _flattened_data.clear();
  _flattened_response.clear();
  _flattened_data.reserve(getLocalSampleSize() * _n_dims);
  _flattened_response.reserve(getLocalSampleSize());
}

void
LibtorchANNTrainer::train()
{
  for (auto & p : _predictor_row)
    _flattened_data.push_back(p);

  _flattened_response.push_back(*_rval);
}

void
LibtorchANNTrainer::postTrain()
{
  _communicator.allgather(_flattened_data);
  _communicator.allgather(_flattened_response);

  // Then, we create and load our Tensors
  unsigned int num_samples = _flattened_response.size();
  unsigned int num_inputs = _sampler.getNumberOfCols();

  // We create a neural net (for the definition of the net see the header file)
  _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      _filename, num_inputs, 1, _num_neurons_per_layer, _activation_function);

  if (_read_from_file)
    try
    {
      torch::load(_nn, _filename);
      _console << "Loaded requested .pt file." << std::endl;
    }
    catch (const c10::Error & e)
    {
      mooseError("The requested pytorch file could not be loaded.\n", e.msg());
    }

  // The default data type in pytorch is float, while we use double in MOOSE.
  // Therefore, in some cases we have to convert Tensors to double.
  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor data_tensor =
      torch::from_blob(_flattened_data.data(), {num_samples, num_inputs}, options).to(at::kDouble);
  torch::Tensor response_tensor =
      torch::from_blob(_flattened_response.data(), {num_samples, 1}, options).to(at::kDouble);

  // We create a custom data set from our converted data
  Moose::LibtorchDataset my_data(data_tensor, response_tensor);

  // We create atrainer for our neral net and train it with the dataset
  Moose::LibtorchArtificialNeuralNetTrainer<> trainer(*_nn, comm());
  trainer.train(my_data, _optim_options);
}

#endif
