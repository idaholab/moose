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
#include "LibtorchRandomUtils.h"
#include "MooseError.h"
#include "libmesh/utility.h"

namespace
{

template <typename NamedTensorList>
auto
captureTensorShapes(const NamedTensorList & tensors)
{
  std::vector<std::pair<std::string, std::vector<int64_t>>> shapes;
  shapes.reserve(tensors.size());

  for (const auto & tensor : tensors)
    shapes.emplace_back(
        tensor.key(),
        std::vector<int64_t>(tensor.value().sizes().begin(), tensor.value().sizes().end()));

  return shapes;
}

template <typename NamedTensorList>
void
verifyTensorShapes(const NamedTensorList & tensors,
                   const std::vector<std::pair<std::string, std::vector<int64_t>>> & expected,
                   const char * tensor_kind)
{
  if (tensors.size() != expected.size())
    mooseError("The loaded DRL actor ", tensor_kind, " count does not match the generated schema.");

  for (const auto tensor_i : make_range(tensors.size()))
  {
    const auto actual_shape = std::vector<int64_t>(tensors[tensor_i].value().sizes().begin(),
                                                   tensors[tensor_i].value().sizes().end());

    if (tensors[tensor_i].key() != expected[tensor_i].first ||
        actual_shape != expected[tensor_i].second)
      mooseError("The loaded DRL actor ",
                 tensor_kind,
                 " '",
                 tensors[tensor_i].key(),
                 "' does not match the generated schema.");
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
                                device_type,
                                data_type,
                                false,
                                input_shift_factors,
                                input_scaling_factors,
                                output_scaling_factors),
    _minimum_values(minimum_values),
    _maximum_values(maximum_values),
    _state_independent_std(state_independent_std)
{
  const bool has_minimum_values = !_minimum_values.empty();
  const bool has_maximum_values = !_maximum_values.empty();
  if (has_minimum_values != has_maximum_values)
    mooseError("Bounded action distributions require both minimum_values and maximum_values.");

  if (has_minimum_values)
  {
    if (_minimum_values.size() != _num_outputs || _maximum_values.size() != _num_outputs)
      mooseError("The number of minimum_values and maximum_values entries must match the number "
                 "of action outputs.");

    for (const auto i : make_range(_minimum_values.size()))
      if (!(_maximum_values[i] > _minimum_values[i]))
        mooseError("maximum_values entries must be strictly greater than minimum_values entries.");
  }

  if (build_on_construct)
    constructNeuralNetwork();
}

LibtorchActorNeuralNet::LibtorchActorNeuralNet(const Moose::LibtorchActorNeuralNet & nn,
                                               const bool build_on_construct)
  : LibtorchArtificialNeuralNet(dynamic_cast<const LibtorchArtificialNeuralNet &>(nn), false),
    _minimum_values(nn.minValues()),
    _maximum_values(nn.maxValues()),
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
LibtorchActorNeuralNet::initializeNeuralNetwork(const c10::optional<at::Generator> generator)
{
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
  {
    const auto & activation =
        _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    const Real gain = determineGain(activation);
    Moose::orthogonalInitializeTensor(_weights[i]->weight, gain, generator);
    torch::nn::init::zeros_(_weights[i]->bias);
  }

  _action_distribution->initialize(generator);
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

  if (_minimum_values.empty() && _maximum_values.empty())
    _action_distribution =
        std::make_shared<LibtorchGaussianActionDistribution>("action_distribution",
                                                             inp_neurons,
                                                             _num_outputs,
                                                             _device_type,
                                                             _data_type,
                                                             true,
                                                             _output_scaling_factors,
                                                             _state_independent_std);
  else
    _action_distribution =
        std::make_shared<LibtorchBetaActionDistribution>("action_distribution",
                                                         inp_neurons,
                                                         _num_outputs,
                                                         _minimum_values,
                                                         _maximum_values,
                                                         _device_type,
                                                         _data_type,
                                                         true,
                                                         _output_scaling_factors);

  // Keep the serialized module name stable so existing checkpoints continue to load.
  register_module("action_head", _action_distribution);
}

torch::Tensor
LibtorchActorNeuralNet::entropy()
{
  return _action_distribution->entropy();
}

const LibtorchGaussianActionDistribution *
LibtorchActorNeuralNet::gaussianActionDistributionPtr() const
{
  return dynamic_cast<const LibtorchGaussianActionDistribution *>(_action_distribution.get());
}

LibtorchGaussianActionDistribution *
LibtorchActorNeuralNet::gaussianActionDistributionPtr()
{
  return dynamic_cast<LibtorchGaussianActionDistribution *>(_action_distribution.get());
}

const LibtorchGaussianActionDistribution &
LibtorchActorNeuralNet::gaussianActionDistribution() const
{
  const auto * distribution = gaussianActionDistributionPtr();
  if (!distribution)
    mooseError("Requested a Gaussian action distribution from a bounded actor.");
  return *distribution;
}

LibtorchGaussianActionDistribution &
LibtorchActorNeuralNet::gaussianActionDistribution()
{
  auto * distribution = gaussianActionDistributionPtr();
  if (!distribution)
    mooseError("Requested a Gaussian action distribution from a bounded actor.");
  return *distribution;
}

const LibtorchBetaActionDistribution *
LibtorchActorNeuralNet::betaActionDistributionPtr() const
{
  return dynamic_cast<const LibtorchBetaActionDistribution *>(_action_distribution.get());
}

LibtorchBetaActionDistribution *
LibtorchActorNeuralNet::betaActionDistributionPtr()
{
  return dynamic_cast<LibtorchBetaActionDistribution *>(_action_distribution.get());
}

const LibtorchBetaActionDistribution &
LibtorchActorNeuralNet::betaActionDistribution() const
{
  const auto * distribution = betaActionDistributionPtr();
  if (!distribution)
    mooseError("Requested a Beta action distribution from an unbounded actor.");
  return *distribution;
}

LibtorchBetaActionDistribution &
LibtorchActorNeuralNet::betaActionDistribution()
{
  auto * distribution = betaActionDistributionPtr();
  if (!distribution)
    mooseError("Requested a Beta action distribution from an unbounded actor.");
  return *distribution;
}

void
LibtorchActorNeuralNet::resetDistributionParams(torch::Tensor input)
{
  _action_distribution->reset(input);
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
  }

  return output;
}

torch::Tensor
LibtorchActorNeuralNet::evaluate(torch::Tensor & x,
                                 const bool sampled,
                                 const c10::optional<at::Generator> generator)
{
  torch::Tensor output = forward(x);

  resetDistributionParams(output);

  if (sampled)
    return sample(generator);

  return _action_distribution->deterministicAction();
}

torch::Tensor
LibtorchActorNeuralNet::sample(const c10::optional<at::Generator> generator)
{
  return _action_distribution->sample(generator);
}

torch::Tensor
LibtorchActorNeuralNet::logProbability(const torch::Tensor & action)
{
  return _action_distribution->logProbability(action);
}

void
loadLibtorchActorNeuralNetState(Moose::LibtorchActorNeuralNet & nn, const std::string & filename)
{
  try
  {
    const auto expected_parameters = captureTensorShapes(nn.named_parameters());
    const auto expected_buffers = captureTensorShapes(nn.named_buffers());

    torch::serialize::InputArchive archive;
    archive.load_from(filename);
    nn.load(archive);

    verifyTensorShapes(nn.named_parameters(), expected_parameters, "parameter");
    verifyTensorShapes(nn.named_buffers(), expected_buffers, "buffer");

    nn.synchronizeAffineFactorsFromBuffers();
    nn.actionDistribution().synchronizeScalingFactorsFromBuffer();
  }
  catch (const c10::Error & e)
  {
    mooseError("The requested DRL actor checkpoint could not be loaded as a native libtorch "
               "archive. Make sure the file exists and matches the generated actor schema.\n",
               e.msg());
  }
}

}

#endif
