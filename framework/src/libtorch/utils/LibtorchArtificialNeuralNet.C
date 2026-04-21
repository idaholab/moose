//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchArtificialNeuralNet.h"
#include "MooseError.h"
#include "LibtorchUtils.h"

#include <cmath>

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
isOptionalArtificialNeuralNetBuffer(const std::string & key)
{
  return key == "input_shift" || key == "input_scale" || key == "output_scale";
}

} // namespace

namespace Moose
{

LibtorchArtificialNeuralNet::LibtorchArtificialNeuralNet(
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
    const std::vector<Real> & output_scaling_factors)
  : _name(name),
    _num_inputs(num_inputs),
    _num_outputs(num_outputs),
    _num_neurons_per_layer(num_neurons_per_layer),
    _activation_function(MultiMooseEnum("relu sigmoid elu gelu linear tanh", "relu")),
    _device_type(device_type),
    _data_type(data_type),
    _input_shift_factors(
        normalizeAffineFactors(input_shift_factors, num_inputs, 0.0, "input_shift_factors")),
    _input_scaling_factors(
        normalizeAffineFactors(input_scaling_factors, num_inputs, 1.0, "input_scaling_factors")),
    _output_scaling_factors(
        normalizeAffineFactors(output_scaling_factors, num_outputs, 1.0, "output_scaling_factors")),
    _minimum_values(minimum_values),
    _maximum_values(maximum_values)
{
  _activation_function = activation_function;
  initializeAffineBuffers();

  // Check if the number of activation functions matches the number of hidden layers
  if ((_activation_function.size() != 1) &&
      (_activation_function.size() != _num_neurons_per_layer.size()))
    mooseError("The number of activation functions should be either one or the same as the number "
               "of hidden layers");

  const bool has_minimum_values = !_minimum_values.empty();
  const bool has_maximum_values = !_maximum_values.empty();
  if (has_minimum_values != has_maximum_values)
    mooseError("Bounded neural network outputs require both minimum_values and maximum_values.");

  if (has_minimum_values)
  {
    if (_minimum_values.size() != _num_outputs || _maximum_values.size() != _num_outputs)
      mooseError("The number of minimum_values and maximum_values entries must match the number "
                 "of outputs.");

    for (const auto i : make_range(_minimum_values.size()))
      if (!(_maximum_values[i] > _minimum_values[i]))
        mooseError("maximum_values entries must be strictly greater than minimum_values entries.");

    auto min_value = _minimum_values;
    LibtorchUtils::vectorToTensor(min_value, _min_tensor);
    _min_tensor = _min_tensor.transpose(0, 1).to(_data_type).to(_device_type);
    auto max_value = _maximum_values;
    LibtorchUtils::vectorToTensor(max_value, _max_tensor);
    _max_tensor = _max_tensor.transpose(0, 1).to(_data_type).to(_device_type);
  }

  if (build_on_construct)
    constructNeuralNetwork();
}

LibtorchArtificialNeuralNet::LibtorchArtificialNeuralNet(
    const Moose::LibtorchArtificialNeuralNet & nn, const bool build_on_construct)
  : torch::nn::Module(),
    _name(nn.name()),
    _num_inputs(nn.numInputs()),
    _num_outputs(nn.numOutputs()),
    _num_neurons_per_layer(nn.numNeuronsPerLayer()),
    _activation_function(nn.activationFunctions()),
    _device_type(nn.deviceType()),
    _data_type(nn.dataType()),
    _input_shift_factors(nn.inputShiftFactors()),
    _input_scaling_factors(nn.inputScalingFactors()),
    _output_scaling_factors(nn.outputScalingFactors()),
    _minimum_values(nn.minValues()),
    _maximum_values(nn.maxValues())
{
  initializeAffineBuffers();

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

  if (_minimum_values.size())
  {
    auto min_value = _minimum_values;
    LibtorchUtils::vectorToTensor(min_value, _min_tensor);
    _min_tensor = _min_tensor.transpose(0, 1).to(_data_type).to(_device_type);
    auto max_value = _maximum_values;
    LibtorchUtils::vectorToTensor(max_value, _max_tensor);
    _max_tensor = _max_tensor.transpose(0, 1).to(_data_type).to(_device_type);
  }
}

Real
LibtorchArtificialNeuralNet::determineGain(const std::string & activation)
{
  if (activation == "relu")
    return std::sqrt(2);
  if (activation == "tanh")
    return 5.0 / 3.0;

  return 1.0;
}

void
LibtorchArtificialNeuralNet::initializeNeuralNetwork()
{
  for (unsigned int i = 0; i < numHiddenLayers(); ++i)
  {
    const auto & activation =
        _activation_function.size() > 1 ? _activation_function[i] : _activation_function[0];
    const Real gain = determineGain(activation);
    torch::nn::init::orthogonal_(_weights[i]->weight, gain);
    torch::nn::init::zeros_(_weights[i]->bias);
  }

  torch::nn::init::orthogonal_(_weights.back()->weight);
  torch::nn::init::zeros_(_weights.back()->bias);
}

std::vector<Real>
LibtorchArtificialNeuralNet::normalizeAffineFactors(const std::vector<Real> & factors,
                                                    const unsigned int expected_size,
                                                    const Real default_value,
                                                    const std::string & factor_name,
                                                    const bool forbid_zero)
{
  const auto normalized =
      factors.empty() ? std::vector<Real>(expected_size, default_value) : factors;

  if (normalized.size() != expected_size)
    mooseError("The number of ", factor_name, " entries must match ", expected_size, ".");

  if (forbid_zero)
    for (const auto factor : normalized)
      if (std::abs(factor) == 0.0)
        mooseError("The ", factor_name, " entries must be non-zero.");

  return normalized;
}

void
LibtorchArtificialNeuralNet::initializeAffineBuffers()
{
  auto input_shift = _input_shift_factors;
  LibtorchUtils::vectorToTensor(input_shift, _input_shift_tensor);
  _input_shift_tensor = register_buffer(
      "input_shift", _input_shift_tensor.transpose(0, 1).to(_data_type).to(_device_type));

  auto input_scale = _input_scaling_factors;
  LibtorchUtils::vectorToTensor(input_scale, _input_scale_tensor);
  _input_scale_tensor = register_buffer(
      "input_scale", _input_scale_tensor.transpose(0, 1).to(_data_type).to(_device_type));

  auto output_scale = _output_scaling_factors;
  LibtorchUtils::vectorToTensor(output_scale, _output_scale_tensor);
  _output_scale_tensor = register_buffer(
      "output_scale", _output_scale_tensor.transpose(0, 1).to(_data_type).to(_device_type));
}

void
LibtorchArtificialNeuralNet::synchronizeAffineFactorsFromBuffers()
{
  auto input_shift = _input_shift_tensor.detach().reshape({-1}).to(torch::kCPU).to(torch::kDouble);
  LibtorchUtils::tensorToVector(input_shift, _input_shift_factors);

  auto input_scale = _input_scale_tensor.detach().reshape({-1}).to(torch::kCPU).to(torch::kDouble);
  LibtorchUtils::tensorToVector(input_scale, _input_scaling_factors);

  auto output_scale =
      _output_scale_tensor.detach().reshape({-1}).to(torch::kCPU).to(torch::kDouble);
  LibtorchUtils::tensorToVector(output_scale, _output_scaling_factors);
}

void
LibtorchArtificialNeuralNet::constructNeuralNetwork()
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
  // Adding output layer
  std::unordered_map<std::string, unsigned int> parameters = {{"inp_neurons", inp_neurons},
                                                              {"out_neurons", _num_outputs}};
  addLayer("output_layer_", parameters);
  _weights.back()->to(_device_type, _data_type);
}

torch::Tensor
LibtorchArtificialNeuralNet::preprocessInput(const torch::Tensor & x) const
{
  torch::Tensor input(x);
  if (_data_type != input.scalar_type())
    input = input.to(_data_type);
  if (_device_type != input.device().type())
    input = input.to(_device_type);

  return (input - _input_shift_tensor) * _input_scale_tensor;
}

torch::Tensor
LibtorchArtificialNeuralNet::scaleOutput(const torch::Tensor & y) const
{
  return y * _output_scale_tensor;
}

torch::Tensor
LibtorchArtificialNeuralNet::forward(const torch::Tensor & x)
{
  torch::Tensor output = preprocessInput(x);

  for (unsigned int i = 0; i < _weights.size() - 1; ++i)
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

  if (_minimum_values.size())
  {
    output = torch::sigmoid(_weights[_weights.size() - 1]->forward(output));
    const auto scale = _max_tensor - _min_tensor;
    output = torch::mul(output, scale);
    output = output + _min_tensor;
  }
  else
    output = _weights[_weights.size() - 1]->forward(output);

  return scaleOutput(output);
}

void
LibtorchArtificialNeuralNet::addLayer(
    const std::string & layer_name,
    const std::unordered_map<std::string, unsigned int> & parameters)
{
  auto it = parameters.find("inp_neurons");
  if (it == parameters.end())
    ::mooseError("Number of input neurons not found during the construction of "
                 "LibtorchArtificialNeuralNet!");
  unsigned int inp_neurons = it->second;

  it = parameters.find("out_neurons");
  if (it == parameters.end())
    ::mooseError("Number of output neurons not found during the construction of "
                 "LibtorchArtificialNeuralNet!");
  unsigned int out_neurons = it->second;

  _weights.push_back(register_module(layer_name, torch::nn::Linear(inp_neurons, out_neurons)));
}

void
LibtorchArtificialNeuralNet::store(nlohmann::json & json) const
{
  const auto & named_params = this->named_parameters();
  for (const auto & param_i : make_range(named_params.size()))
  {
    // We cast the parameters into a 1D vector
    json[named_params[param_i].key()] = std::vector<Real>(
        named_params[param_i].value().data_ptr<Real>(),
        named_params[param_i].value().data_ptr<Real>() + named_params[param_i].value().numel());
  }

  json["input_shift_factors"] = _input_shift_factors;
  json["input_scaling_factors"] = _input_scaling_factors;
  json["output_scaling_factors"] = _output_scaling_factors;
}

void
to_json(nlohmann::json & json, const Moose::LibtorchArtificialNeuralNet * const & network)
{
  if (network)
    network->store(json);
}

void
loadLibtorchArtificialNeuralNetState(Moose::LibtorchArtificialNeuralNet & nn,
                                     const std::string & filename)
{
  torch::serialize::InputArchive archive;
  archive.load_from(filename);

  for (auto & parameter : nn.named_parameters())
  {
    torch::Tensor stored_tensor;
    if (!readArchiveTensor(archive, parameter.key(), stored_tensor))
      mooseError("The requested pytorch parameter file could not be loaded. This can either be "
                 "the result of the file not existing or a misalignment in the generated "
                 "container and the data in the file. Make sure the dimensions of the generated "
                 "neural net are the same as the dimensions of the parameters in the input file!\n"
                 "Missing serialized parameter: ",
                 parameter.key());

    copyTensor(parameter.value(), stored_tensor);
  }

  for (auto & buffer : nn.named_buffers())
  {
    torch::Tensor stored_tensor;
    if (!readArchiveTensor(archive, buffer.key(), stored_tensor))
    {
      if (isOptionalArtificialNeuralNetBuffer(buffer.key()))
        continue;

      mooseError("The requested pytorch parameter file could not be loaded. This can either be "
                 "the result of the file not existing or a misalignment in the generated "
                 "container and the data in the file. Make sure the dimensions of the generated "
                 "neural net are the same as the dimensions of the parameters in the input file!\n"
                 "Missing serialized buffer: ",
                 buffer.key());
    }

    copyTensor(buffer.value(), stored_tensor);
  }

  nn.synchronizeAffineFactorsFromBuffers();
}

}

template <>
void
dataStore<Moose::LibtorchArtificialNeuralNet>(
    std::ostream & stream, std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn, void * context)
{
  std::string n(nn->name());
  dataStore(stream, n, context);

  unsigned int ni(nn->numInputs());
  dataStore(stream, ni, context);

  unsigned int no(nn->numOutputs());
  dataStore(stream, no, context);

  unsigned int nhl(nn->numHiddenLayers());
  dataStore(stream, nhl, context);

  std::vector<unsigned int> nnpl(nn->numNeuronsPerLayer());
  dataStore(stream, nnpl, context);

  unsigned int afs(nn->activationFunctions().size());
  dataStore(stream, afs, context);

  std::vector<std::string> items(afs);
  for (unsigned int i = 0; i < afs; ++i)
    items[i] = nn->activationFunctions()[i];

  // unsigned int nminv(nn->minValues().size());
  // dataStore(stream, nminv, context);
  std::vector<Real> minv(nn->minValues());
  dataStore(stream, minv, context);

  // unsigned int nmaxv(nn->minValues().size());
  // dataStore(stream, nmaxv, context);
  std::vector<Real> maxv(nn->maxValues());
  dataStore(stream, maxv, context);

  dataStore(stream, items, context);

  auto device_type = static_cast<std::underlying_type<torch::DeviceType>::type>(nn->deviceType());
  dataStore(stream, device_type, context);

  auto data_type = static_cast<std::underlying_type<torch::ScalarType>::type>(nn->dataType());
  dataStore(stream, data_type, context);

  torch::save(nn, nn->name());
}

template <>
void
dataLoad<Moose::LibtorchArtificialNeuralNet>(
    std::istream & stream, std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn, void * context)
{
  std::string name;
  dataLoad(stream, name, context);

  unsigned int num_inputs;
  dataLoad(stream, num_inputs, context);

  unsigned int num_outputs;
  dataLoad(stream, num_outputs, context);

  unsigned int num_hidden_layers;
  dataLoad(stream, num_hidden_layers, context);

  std::vector<unsigned int> num_neurons_per_layer;
  num_neurons_per_layer.resize(num_hidden_layers);
  dataLoad(stream, num_neurons_per_layer, context);

  unsigned int num_activation_items;
  dataLoad(stream, num_activation_items, context);

  std::vector<std::string> activation_functions;
  activation_functions.resize(num_activation_items);
  dataLoad(stream, activation_functions, context);

  std::vector<Real> min_values;
  min_values.resize(num_outputs);
  dataLoad(stream, min_values, context);

  std::vector<Real> max_values;
  max_values.resize(num_outputs);
  dataLoad(stream, max_values, context);

  std::underlying_type<torch::DeviceType>::type device_type;
  dataLoad(stream, device_type, context);
  const torch::DeviceType divt(static_cast<torch::DeviceType>(device_type));

  std::underlying_type<torch::ScalarType>::type data_type;
  dataLoad(stream, data_type, context);
  const torch::ScalarType datt(static_cast<torch::ScalarType>(data_type));

  nn = std::make_shared<Moose::LibtorchArtificialNeuralNet>(name,
                                                            num_inputs,
                                                            num_outputs,
                                                            num_neurons_per_layer,
                                                            activation_functions,
                                                            min_values,
                                                            max_values,
                                                            divt,
                                                            datt);

  Moose::loadLibtorchArtificialNeuralNetState(*nn, name);
}

template <>
void
dataStore<Moose::LibtorchArtificialNeuralNet const>(
    std::ostream & /*stream*/,
    Moose::LibtorchArtificialNeuralNet const *& /*nn*/,
    void * /*context*/)
{
}

template <>
void
dataLoad<Moose::LibtorchArtificialNeuralNet const>(
    std::istream & /*stream*/,
    Moose::LibtorchArtificialNeuralNet const *& /*nn*/,
    void * /*context*/)
{
}

#endif
