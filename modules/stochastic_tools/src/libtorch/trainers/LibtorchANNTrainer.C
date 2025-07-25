//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

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
      "nn_filename", "net.pt", "Filename used to output the neural net parameters.");
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

  params.addParam<bool>(
      "standardize_input", true, "Standardize (center and scale) training inputs (x values)");
  params.addParam<bool>(
      "standardize_output", true, "Standardize (center and scale) training outputs (y values)");

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
    _nn_filename(getParam<std::string>("nn_filename")),
    _read_from_file(getParam<bool>("read_from_file")),
    _nn(declareModelData<std::shared_ptr<Moose::LibtorchArtificialNeuralNet>>("nn")),
    _standardize_input(getParam<bool>("standardize_input")),
    _standardize_output(getParam<bool>("standardize_output")),
    _input_standardizer(declareModelData<StochasticTools::Standardizer>("input_standardizer")),
    _output_standardizer(declareModelData<StochasticTools::Standardizer>("output_standardizer"))
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
  unsigned int num_inputs = _n_dims;

  // We create a neural net (for the definition of the net see the header file)
  _nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      _nn_filename, num_inputs, 1, _num_neurons_per_layer, _activation_function);

  if (_read_from_file)
    try
    {
      torch::load(_nn, _nn_filename);
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

  // We standardize the input/output pairs if the user requested it
  if (_standardize_input)
  {
    auto data_std_mean = torch::std_mean(data_tensor, 0);
    auto & data_std = std::get<0>(data_std_mean);
    auto & data_mean = std::get<1>(data_std_mean);

    data_tensor = (data_tensor - data_mean) / data_std;

    std::vector<Real> converted_data_mean;
    LibtorchUtils::tensorToVector(data_mean, converted_data_mean);
    std::vector<Real> converted_data_std;
    LibtorchUtils::tensorToVector(data_std, converted_data_std);
    _input_standardizer.set(converted_data_mean, converted_data_std);
  }
  else
    _input_standardizer.set(_n_dims);

  if (_standardize_output)
  {
    auto response_std_mean = torch::std_mean(response_tensor, 0);
    auto & response_std = std::get<0>(response_std_mean);
    auto & response_mean = std::get<1>(response_std_mean);

    response_tensor = (response_tensor - response_mean) / response_std;

    std::vector<Real> converted_response_mean;
    LibtorchUtils::tensorToVector(response_mean, converted_response_mean);
    std::vector<Real> converted_response_std;
    LibtorchUtils::tensorToVector(response_std, converted_response_std);
    _output_standardizer.set(converted_response_mean, converted_response_std);
  }
  else
    _output_standardizer.set(1);

  // We create a custom data set from our converted data
  Moose::LibtorchDataset my_data(data_tensor, response_tensor);

  // We create atrainer for our neral net and train it with the dataset
  Moose::LibtorchArtificialNeuralNetTrainer<> trainer(*_nn, comm());
  trainer.train(my_data, _optim_options);
}

#endif
