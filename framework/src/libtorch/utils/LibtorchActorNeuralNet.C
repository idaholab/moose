//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchActorNeuralNet.h"
#include "MooseError.h"

namespace
{

bool
readArchiveTensor(torch::serialize::InputArchive & archive,
                  const std::string & key,
                  torch::Tensor & tensor)
{
  try
  {
    archive.read(key, tensor);
    return true;
  }
  catch (const c10::Error &)
  {
    return false;
  }
}

void
copyTensor(torch::Tensor & destination, const torch::Tensor & source)
{
  destination.data().copy_(source.to(destination.options()));
}

bool
readActorStateTensor(torch::serialize::InputArchive & archive,
                     const std::string & key,
                     torch::Tensor & tensor)
{
  if (readArchiveTensor(archive, key, tensor))
    return true;

  if (key.rfind("action_head.", 0) == 0)
    return readArchiveTensor(archive, key.substr(std::string("action_head.").size()), tensor);

  return false;
}

bool
isOptionalActorBuffer(const std::string & key)
{
  return key == "input_shift" || key == "input_scale" || key == "output_scale" ||
         key == "action_head.action_scale";
}

bool
isOptionalActorParameter(const std::string & key)
{
  return key == "action_head.mean.bias" || key == "action_head.std.bias";
}

template <typename NamedTensorList>
bool
findNamedTensor(const NamedTensorList & tensors, const std::string & key, torch::Tensor & tensor)
{
  for (const auto & entry : tensors)
    if (entry.name == key)
    {
      tensor = entry.value;
      return true;
    }

  return false;
}

template <typename NamedTensorList>
bool
readScriptedActorStateTensor(const NamedTensorList & tensors,
                             const std::string & key,
                             torch::Tensor & tensor)
{
  if (findNamedTensor(tensors, key, tensor))
    return true;

  if (key.rfind("action_head.", 0) == 0)
    return findNamedTensor(tensors, key.substr(std::string("action_head.").size()), tensor);

  return false;
}

bool
loadActorStateFromArchive(Moose::LibtorchActorNeuralNet & nn,
                          const std::string & filename,
                          std::string & error)
{
  try
  {
    torch::serialize::InputArchive archive;
    archive.load_from(filename);

    for (auto & parameter : nn.named_parameters())
    {
      torch::Tensor stored_tensor;
      if (!readActorStateTensor(archive, parameter.key(), stored_tensor))
      {
        if (isOptionalActorParameter(parameter.key()))
        {
          parameter.value().data().zero_();
          continue;
        }

        error = "Missing serialized parameter: " + parameter.key();
        return false;
      }

      copyTensor(parameter.value(), stored_tensor);
    }

    for (auto & buffer : nn.named_buffers())
    {
      torch::Tensor stored_tensor;
      if (!readActorStateTensor(archive, buffer.key(), stored_tensor))
      {
        if (isOptionalActorBuffer(buffer.key()))
          continue;

        error = "Missing serialized buffer: " + buffer.key();
        return false;
      }

      copyTensor(buffer.value(), stored_tensor);
    }

    nn.synchronizeAffineFactorsFromBuffers();
    nn.actionDistributionHead().synchronizeScalingFactorsFromBuffer();
    return true;
  }
  catch (const c10::Error & e)
  {
    error = e.msg();
    return false;
  }
}

bool
loadActorStateFromTorchScript(Moose::LibtorchActorNeuralNet & nn,
                              const std::string & filename,
                              std::string & error)
{
  try
  {
    const auto scripted = torch::jit::load(filename);
    const auto scripted_parameters = scripted.named_parameters();
    const auto scripted_buffers = scripted.named_buffers();

    for (auto & parameter : nn.named_parameters())
    {
      torch::Tensor stored_tensor;
      if (!readScriptedActorStateTensor(scripted_parameters, parameter.key(), stored_tensor))
      {
        if (isOptionalActorParameter(parameter.key()))
        {
          parameter.value().data().zero_();
          continue;
        }

        error = "Missing scripted parameter: " + parameter.key();
        return false;
      }

      copyTensor(parameter.value(), stored_tensor);
    }

    for (auto & buffer : nn.named_buffers())
    {
      torch::Tensor stored_tensor;
      if (!readScriptedActorStateTensor(scripted_buffers, buffer.key(), stored_tensor))
      {
        if (isOptionalActorBuffer(buffer.key()))
          continue;

        error = "Missing scripted buffer: " + buffer.key();
        return false;
      }

      copyTensor(buffer.value(), stored_tensor);
    }

    nn.synchronizeAffineFactorsFromBuffers();
    nn.actionDistributionHead().synchronizeScalingFactorsFromBuffer();
    return true;
  }
  catch (const c10::Error & e)
  {
    error = e.msg();
    return false;
  }
}

} // namespace

namespace Moose
{

LibtorchActorNeuralNet::LibtorchActorNeuralNet(
    const std::string name,
    const unsigned int num_inputs,
    const unsigned int num_outputs,
    const std::vector<unsigned int> & num_neurons_per_layer,
    const std::vector<std::string> & activation_function,
    const std::vector<Real> & minimum_values,
    const std::vector<Real> & maximum_values,
    const torch::DeviceType device_type,
    const torch::ScalarType data_type,
    const bool build_on_construct,
    const std::vector<Real> & input_shift_factors,
    const std::vector<Real> & input_scaling_factors,
    const std::vector<Real> & output_scaling_factors,
    const bool state_independent_std)
  : LibtorchArtificialNeuralNet(name,
                                num_inputs,
                                num_outputs,
                                num_neurons_per_layer,
                                activation_function,
                                minimum_values,
                                maximum_values,
                                device_type,
                                data_type,
                                false,
                                input_shift_factors,
                                input_scaling_factors,
                                output_scaling_factors),
    _state_independent_std(state_independent_std)
{
  if (build_on_construct)
    constructNeuralNetwork();
}

LibtorchActorNeuralNet::LibtorchActorNeuralNet(const Moose::LibtorchActorNeuralNet & nn,
                                               const bool build_on_construct)
  : LibtorchArtificialNeuralNet(dynamic_cast<const LibtorchArtificialNeuralNet &>(nn), false),
    _state_independent_std(nn.stateIndependentStd())
{
  // We construct the NN architecture
  if (build_on_construct)
  {
    constructNeuralNetwork();
    // We fill it up with the current parameter values
    const auto & from_params = nn.named_parameters();
    auto to_params = this->named_parameters();
    for (unsigned int param_i : make_range(from_params.size()))
      to_params[param_i].value().data() = from_params[param_i].value().data().clone();

    const auto & from_buffers = nn.named_buffers();
    auto to_buffers = this->named_buffers();
    for (unsigned int buffer_i : make_range(from_buffers.size()))
      to_buffers[buffer_i].value().data() = from_buffers[buffer_i].value().data().clone();
  }
}

void
LibtorchActorNeuralNet::initializeNeuralNetwork()
{
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
  {
    const auto & activation =
        _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    const Real gain = determineGain(activation);

    auto sizes = _weights[i]->weight.sizes();
    auto max_dim_size = *std::max_element(sizes.begin(), sizes.end());
    torch::nn::init::orthogonal_(_weights[i]->weight, gain / max_dim_size);
    torch::nn::init::zeros_(_weights[i]->bias);
  }

  _action_head->initialize();
}

void
LibtorchActorNeuralNet::constructNeuralNetwork()
{
  // Adding hidden layers
  unsigned int inp_neurons = _num_inputs;
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
  {
    std::unordered_map<std::string, unsigned int> parameters = {
        {"inp_neurons", inp_neurons}, {"out_neurons", _num_neurons_per_layer[i]}};
    addLayer("hidden_layer_" + std::to_string(i + 1), parameters);

    // Necessary to retain double precision (and error-free runs)
    _weights[i]->to(_device_type, _data_type);
    inp_neurons = _num_neurons_per_layer[i];
  }

  _action_head = std::make_shared<LibtorchActionDistributionHead>("action_head",
                                                                  inp_neurons,
                                                                  _num_outputs,
                                                                  _minimum_values,
                                                                  _maximum_values,
                                                                  _device_type,
                                                                  _data_type,
                                                                  true,
                                                                  _output_scaling_factors,
                                                                  _state_independent_std);
  register_module("action_head", _action_head);
}

torch::Tensor
LibtorchActorNeuralNet::entropy()
{
  return _action_head->entropy();
}

void
LibtorchActorNeuralNet::resetDistributionParams(torch::Tensor input)
{
  _action_head->reset(input);
  _alpha_tensor = _action_head->alphaTensor();
  _beta_tensor = _action_head->betaTensor();
  _std_tensor = _action_head->stdTensor();
}

torch::Tensor
LibtorchActorNeuralNet::forward(const torch::Tensor & x)
{
  torch::Tensor output = preprocessInput(x);

  for (unsigned int i = 0; i < _weights.size(); ++i)
  {
    std::string activation =
        _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    if (activation == "relu")
      output = torch::relu(_weights[i]->forward(output));
    else if (activation == "sigmoid")
      output = torch::sigmoid(_weights[i]->forward(output));
    else if (activation == "tanh")
      output = torch::tanh(_weights[i]->forward(output));
    else if (activation == "elu")
      output = torch::elu(_weights[i]->forward(output));
    else if (activation == "gelu")
      output = torch::gelu(_weights[i]->forward(output));
    else if (activation == "linear")
      output = _weights[i]->forward(output);

    // std::cout << "midresult" << i << output << std::endl;
  }

  return output;
}

torch::Tensor
LibtorchActorNeuralNet::evaluate(torch::Tensor & x, bool sampled)
{
  torch::Tensor output = forward(x);

  // std::cout << "midresult" << output << std::endl;
  resetDistributionParams(output);

  if (sampled)
    return sample();

  return _action_head->deterministicAction();
}

torch::Tensor
LibtorchActorNeuralNet::sample()
{
  return _action_head->sample();
}

torch::Tensor
LibtorchActorNeuralNet::logProbability(const torch::Tensor & action)
{
  return _action_head->logProbability(action);
}

void
loadLibtorchActorNeuralNetState(Moose::LibtorchActorNeuralNet & nn, const std::string & filename)
{
  std::string archive_error;
  if (loadActorStateFromArchive(nn, filename, archive_error))
    return;

  std::string torchscript_error;
  if (loadActorStateFromTorchScript(nn, filename, torchscript_error))
    return;

  mooseError("The requested pytorch parameter file could not be loaded. This can either be "
             "the result of the file not existing or a misalignment in the generated "
             "container and the data in the file. Make sure the dimensions of the generated "
             "neural net are the same as the dimensions of the parameters in the input file!\n"
             "InputArchive load failed with: ",
             archive_error,
             "\nTorchScript load failed with: ",
             torchscript_error);
}

bool
isLegacyLibtorchActorArchive(const std::string & filename)
{
  try
  {
    const auto scripted = torch::jit::load(filename);
    const auto parameters = scripted.named_parameters();

    torch::Tensor ignored;
    return findNamedTensor(parameters, "output_layer_.weight", ignored) &&
           !findNamedTensor(parameters, "action_head.mean.weight", ignored);
  }
  catch (const c10::Error &)
  {
    return false;
  }
}

void
loadLegacyLibtorchActorNeuralNetState(Moose::LibtorchActorNeuralNet & nn,
                                      const std::string & filename,
                                      const std::vector<Real> & action_standard_deviations)
{
  if (nn.actionDistributionHead().isBounded())
    mooseError("Legacy deterministic DRL checkpoints are only supported for unbounded actors.");

  const auto legacy_std = action_standard_deviations.empty()
                              ? std::vector<Real>(nn.numOutputs(), 1e-12)
                              : action_standard_deviations;

  if (legacy_std.size() != nn.numOutputs())
    mooseError("The number of action_standard_deviations entries must match the number of action "
               "outputs when loading a legacy deterministic DRL checkpoint.");

  for (const auto std_value : legacy_std)
    if (!(std_value > 0.0))
      mooseError("Legacy action_standard_deviations entries must be strictly positive.");

  const auto scripted = torch::jit::load(filename);
  const auto legacy_parameters = scripted.named_parameters();

  for (auto & parameter : nn.named_parameters())
  {
    const auto & key = parameter.key();
    torch::Tensor stored_tensor;

    if (key == "action_head.mean.weight")
    {
      if (!findNamedTensor(legacy_parameters, "output_layer_.weight", stored_tensor))
        mooseError("Legacy deterministic DRL checkpoint is missing output_layer_.weight.");
      copyTensor(parameter.value(), stored_tensor);
      continue;
    }

    if (key == "action_head.mean.bias")
    {
      if (!findNamedTensor(legacy_parameters, "output_layer_.bias", stored_tensor))
        mooseError("Legacy deterministic DRL checkpoint is missing output_layer_.bias.");
      copyTensor(parameter.value(), stored_tensor);
      continue;
    }

    if (key == "action_head.std.weight")
    {
      parameter.value().data().zero_();
      continue;
    }

    if (key == "action_head.std.bias")
    {
      auto log_std = torch::log(torch::tensor(legacy_std, parameter.value().options()));
      copyTensor(parameter.value(), log_std);
      continue;
    }

    if (!findNamedTensor(legacy_parameters, key, stored_tensor))
      mooseError("Legacy deterministic DRL checkpoint is missing serialized parameter: ", key);

    copyTensor(parameter.value(), stored_tensor);
  }

  nn.synchronizeAffineFactorsFromBuffers();
  nn.actionDistributionHead().synchronizeScalingFactorsFromBuffer();
}

}

#endif
