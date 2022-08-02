//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED
#include "LibtorchDataset.h"
#include "LibtorchArtificialNeuralNetTrainer.h"
#endif

#include "LibtorchNeuralNetControlTrainer.h"
#include "Sampler.h"
#include "Function.h"

registerMooseObject("StochasticToolsApp", LibtorchNeuralNetControlTrainer);

InputParameters
LibtorchNeuralNetControlTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Trains a simple neural network controller using libtorch.");

  params.addRequiredParam<std::vector<ReporterName>>(
      "response_reporter", "Reporters containing the response values from the model.");
  params.addParam<std::vector<Real>>(
      "response_shift_coeffs",
      "A shift constant which will be used to shift the response values. This is used for the "
      "manipulation of the neural net inputs for better training efficiency.");
  params.addParam<std::vector<Real>>(
      "response_normalization_coeffs",
      "A normalization constant which will be used to divide the response values. This is used for "
      "the manipulation of the neural net inputs for better training efficiency.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "control_reporter", "Reporters containing the control values fromthe models.");
  params.addRequiredParam<std::vector<FunctionName>>(
      "response_constraints", "Constraints on the postprocessor values in the response reporters.");

  params.addRequiredParam<Real>("emulator_learning_rate",
                                "Learning rate (relaxation) for the emulator training.");
  params.addRequiredParam<unsigned int>("num_emulator_batches",
                                        "Number of batches for the emulator training.");
  params.addRequiredParam<unsigned int>("num_emulator_epochs",
                                        "Number of epochs for the emulator training.");
  params.addRequiredParam<std::vector<unsigned int>>(
      "num_emulator_neurons_per_layer", "Number of neurons per layer in the emulator neural net.");
  params.addParam<std::vector<std::string>>(
      "emulator_activation_function",
      std::vector<std::string>({"relu"}),
      "The type of activation functions to use in the emulator neural net. It is either one value "
      "or one value per hidden layer.");

  params.addRequiredParam<Real>("control_learning_rate",
                                "Learning rate (relaxation) for the control neural net training.");
  params.addRequiredParam<unsigned int>("num_control_epochs",
                                        "Number of epochs for the control neural net training.");
  params.addRequiredParam<unsigned int>("num_control_loops",
                                        "Number of loops for training the control neural net.");
  params.addRequiredParam<std::vector<unsigned int>>("num_control_neurons_per_layer",
                                                     "Number of neurons per layer.");
  params.addParam<std::vector<std::string>>(
      "control_activation_function",
      std::vector<std::string>({"relu"}),
      "The type of activation functions to use in the control neural net. It "
      "is either one value "
      "or one value per hidden layer.");

  params.addParam<std::string>(
      "filename", "net.pt", "Filename used to output the neural net parameters.");

  params.addParam<bool>("use_old_response",
                        false,
                        "If we want to use the old responses besides the current responses to "
                        "ealuate the neural network.");

  params.addParam<unsigned int>(
      "seed", 11, "Random number generator seed for stochastic optimizers.");

  return params;
}

LibtorchNeuralNetControlTrainer::LibtorchNeuralNetControlTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _response_names(getParam<std::vector<ReporterName>>("response_reporter")),
    _response_constraints(getParam<std::vector<FunctionName>>("response_constraints")),
    _control_names(getParam<std::vector<ReporterName>>("control_reporter")),
    _num_emulator_batches(getParam<unsigned int>("num_emulator_batches")),
    _num_emulator_epochs(getParam<unsigned int>("num_emulator_epochs")),
    _num_emulator_neurons_per_layer(
        getParam<std::vector<unsigned int>>("num_emulator_neurons_per_layer")),
    _emulator_learning_rate(getParam<Real>("emulator_learning_rate")),
    _num_control_epochs(getParam<unsigned int>("num_control_epochs")),
    _num_control_loops(getParam<unsigned int>("num_control_loops")),
    _num_control_neurons_per_layer(
        getParam<std::vector<unsigned int>>("num_control_neurons_per_layer")),
    _control_learning_rate(getParam<Real>("control_learning_rate")),
    _filename(getParam<std::string>("filename")),
    _use_old_response(getParam<bool>("use_old_response"))
{

  if (_response_names.size() == 0)
    mooseError("The number of reponses reporters should be more than 0!");

  if (isParamValid("response_shift_coeffs"))
  {
    _response_shift_coeffs = getParam<std::vector<Real>>("response_shift_coeffs");
    if (_response_names.size() != _response_shift_coeffs.size())
      paramError("response_shift_coeffs",
                 "The number of shift factors is not the same as the number of responses!");

    _response_normalization_coeffs = getParam<std::vector<Real>>("response_normalization_coeffs");
    if (_response_names.size() != _response_normalization_coeffs.size())
      paramError(
          "response_normalization_coeffs",
          "The number of normalization coefficients is not the same as the number of responses!");
  }
  else
  {
    _response_shift_coeffs = std::vector<Real>(_response_names.size(), 0.0);
    _response_normalization_coeffs = std::vector<Real>(_response_names.size(), 1.0);
  }

  if (_control_names.size() == 0)
    mooseError("The number of control reporters should be more than 0!");

  if (_response_names.size() != _response_constraints.size())
    paramError("response_constraints",
               "The number of responses is not equal to the number of response constraints!");

#ifdef LIBTORCH_ENABLED

  // Fixing the RNG seed to make sure every experiment is the same.
  // Otherwise sampling / stochastic gradient descent would be different.
  torch::manual_seed(getParam<unsigned int>("seed"));

  // Initializing and saving the control neural net so that the control can grab it right away
  unsigned int num_control_inputs = (_use_old_response ? 2 : 1) * _response_names.size();
  unsigned int num_control_outputs = _control_names.size();

  _control_nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      _filename,
      num_control_inputs,
      num_control_outputs,
      _num_control_neurons_per_layer,
      getParam<std::vector<std::string>>("control_activation_function"));
  torch::save(_control_nn, _control_nn->name());

  unsigned int num_emulator_inputs = num_control_inputs + num_control_outputs;
  unsigned int num_emulator_outputs = _response_names.size();

  _emulator_nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(
      "emulator",
      num_emulator_inputs,
      num_emulator_outputs,
      _num_emulator_neurons_per_layer,
      getParam<std::vector<std::string>>("emulator_activation_function"));

#endif
}

void
LibtorchNeuralNetControlTrainer::preTrain()
{
}

void
LibtorchNeuralNetControlTrainer::train()
{
}

void
LibtorchNeuralNetControlTrainer::postTrain()
{
  // We collect the results from the resporters
  unsigned int num_time_steps =
      getReporterValueByName<std::vector<Real>>(_response_names[0]).size();

  unsigned int num_rows = num_time_steps - 1;
  unsigned int num_cols =
      (_use_old_response ? 2 : 1) * _response_names.size() + _control_names.size();
  unsigned int num_responses = _response_names.size();

  // Libtorch needs a 1Draw vector to be able to convert to torch::Tensors
  _flattened_data.resize(num_rows * num_cols);
  _flattened_response.resize(num_rows * _response_names.size());

  // Fill the 1D containers with the reporters that contain the quantities of interest
  for (unsigned int rep_i = 0; rep_i < _response_names.size(); ++rep_i)
  {
    const std::vector<Real> & data =
        getReporterValueByName<std::vector<Real>>(_response_names[rep_i]);
    for (unsigned int step_i = 0; step_i < data.size() - 1; ++step_i)
    {
      _flattened_data[num_cols * step_i + rep_i] =
          (data[step_i] - _response_shift_coeffs[rep_i]) / _response_normalization_coeffs[rep_i];
      if (_use_old_response)
      {
        if (step_i == 0)
          _flattened_data[num_cols * step_i + rep_i + num_responses] =
              (data[step_i] - _response_shift_coeffs[rep_i]) /
              _response_normalization_coeffs[rep_i];
        else
          _flattened_data[num_cols * step_i + rep_i + num_responses] =
              (data[step_i - 1] - _response_shift_coeffs[rep_i]) /
              _response_normalization_coeffs[rep_i];
      }
      _flattened_response[num_responses * step_i + rep_i] = data[step_i + 1];
    }
  }

  // Fill the 1D containers with the reporters that contain the control values
  for (unsigned int rep_i = 0; rep_i < _control_names.size(); ++rep_i)
  {
    const std::vector<Real> & data =
        getReporterValueByName<std::vector<Real>>(_control_names[rep_i]);
    for (unsigned int step_i = 0; step_i < data.size() - 1; ++step_i)
      _flattened_data[num_cols * step_i + rep_i + (_use_old_response ? 2 : 1) * num_responses] =
          data[step_i];
  }

  _communicator.allgather(_flattened_data);
  _communicator.allgather(_flattened_response);

  // We train the emulator to be able to emulate the system response
  trainEmulator();

  // We train the controller using the emulator to get a good control strategy
  trainController();
}

void
LibtorchNeuralNetControlTrainer::trainEmulator()
{

#ifdef LIBTORCH_ENABLED

  // This gets us the number of timesteps
  unsigned int num_time_steps =
      getReporterValueByName<std::vector<Real>>(_response_names[0]).size();

  unsigned int num_rows = num_time_steps - 1;
  unsigned int num_cols =
      (_use_old_response ? 2 : 1) * _response_names.size() + _control_names.size();
  unsigned int num_responses = _response_names.size();

  // The default data type in pytorch is float, while we use double in MOOSE.
  // Therefore, in some cases we have to convert Tensors to double.
  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor data_tensor =
      torch::from_blob(_flattened_data.data(), {num_rows, num_cols}, options).to(at::kDouble);
  torch::Tensor response_tensor =
      torch::from_blob(_flattened_response.data(), {num_rows, num_responses}, options)
          .to(at::kDouble);

  // We create a custom data loader which can be used to select samples for the in
  // the training process. See the header file for the definition of this structure.
  Moose::LibtorchDataset training_data(data_tensor, response_tensor);

  Moose::LibtorchTrainingOptions optim_options;
  optim_options.optimizer_type = "adam";
  optim_options.learning_rate = _emulator_learning_rate;
  optim_options.num_epochs = _num_emulator_epochs;
  optim_options.num_batches = _num_emulator_batches;
  optim_options.rel_loss_tol = 1e-8;
  optim_options.print_loss = true;
  optim_options.print_epoch_loss = 10;
  optim_options.parallel_processes = 1;

  Moose::LibtorchArtificialNeuralNetTrainer<> trainer(_emulator_nn, comm());
  trainer.train(training_data, optim_options);

#endif
}

void
LibtorchNeuralNetControlTrainer::trainController()
{

#ifdef LIBTORCH_ENABLED

  // We get the number of time steps
  unsigned int num_time_steps =
      getReporterValueByName<std::vector<Real>>(_response_names[0]).size();

  unsigned int num_cols = (_use_old_response ? 2 : 1) * _response_names.size();
  unsigned int num_responses = _response_names.size();

  // Initialize the optimizer
  torch::optim::Adam optimizer(_control_nn->parameters(),
                               torch::optim::AdamOptions(_control_learning_rate));

  // We initialize the starting vector for the sweeps
  std::vector<Real> start_vector(&_flattened_data[0], &_flattened_data[num_cols]);
  auto options = torch::TensorOptions().dtype(at::kDouble);
  torch::Tensor input =
      torch::from_blob(start_vector.data(), {1, num_cols}, options).to(at::kDouble);

  // Begin controller training loop
  for (unsigned int loop_i = 0; loop_i < _num_control_loops; ++loop_i)
  {
    // We loop through the timesteps simulating the system using the emulator
    // neural net
    for (unsigned int step_i = 1; step_i <= num_time_steps - 1; ++step_i)
    {
      Real epoch_error = 100.0;
      unsigned int epoch_counter = 0;
      std::vector<Real> converted_prediction;

      // We iterate in each timestep until we get a good neural net control
      while (epoch_counter < _num_control_epochs && epoch_error > 1e-6)
      {
        epoch_counter += 1;

        optimizer.zero_grad();

        // We predict the controller values based on the previous two timesteps
        torch::Tensor control_prediction = _control_nn->forward(input);

        // We add the controller values to the input vectors
        torch::Tensor extended_input = torch::cat({input, control_prediction}, -1).to(at::kDouble);

        // We use the emulator to predict the values of the quantities interest in the
        // next timestep
        torch::Tensor value_prediction = _emulator_nn->forward(extended_input);

        converted_prediction =
            std::vector<Real>({value_prediction.data_ptr<Real>(),
                               value_prediction.data_ptr<Real>() + value_prediction.size(1)});

        // Evaluate the contraints for each response
        std::vector<Real> constraints;
        Point dummy;
        for (unsigned int resp_i = 0; resp_i < num_responses; ++resp_i)
        {
          const Function & constraint_function(getFunctionByName(_response_constraints[resp_i]));
          auto constraint = constraint_function.value(converted_prediction[resp_i], dummy);
          constraints.push_back(constraint);
        }

        // Build the constraint tensor for the loss calculation
        torch::Tensor constraint_tensor =
            torch::from_blob(constraints.data(), {1, num_responses}, options).to(at::kDouble);

        // Compute loss values using a MSE ( mean squared error)
        torch::Tensor loss = torch::mse_loss(value_prediction, constraint_tensor);

        // Propagate error back
        loss.backward();

        // Use new gradients to update the parameters
        optimizer.step();

        epoch_error = loss.item<double>();
      }

      _console << "Controller training step: " << step_i << " (" << epoch_counter
               << ") | Loss: " << COLOR_GREEN << epoch_error << COLOR_DEFAULT << std::endl;

      // Build a new input tensor using the outputs
      for (unsigned int resp_i = 0; resp_i < num_responses; ++resp_i)
      {
        start_vector[resp_i] = start_vector[resp_i + num_responses];
        start_vector[resp_i + num_responses] =
            (converted_prediction[resp_i] - _response_shift_coeffs[resp_i]) /
            _response_normalization_coeffs[resp_i];
      }
      input = torch::from_blob(start_vector.data(), {1, num_cols}, options).to(at::kDouble);
    }
  }

  // Save the controller neural net so our controller can read it
  torch::save(_control_nn, _control_nn->name());

#endif
}
