//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include <torch/torch.h>
#include <torch/script.h>
#include <torch/serialize/archive.h>
#include "LibtorchNeuralNetBase.h"
#include "MooseError.h"
#include "DataIO.h"
#include "MultiMooseEnum.h"
#include "nlohmann/json.h"

namespace Moose
{

/**
 * Simple feed-forward neural net with optional affine input and output scaling.
 */
class LibtorchArtificialNeuralNet : public torch::nn::Module, public LibtorchNeuralNetBase
{
public:
  /**
   * Build a plain feed-forward neural network.
   * @param name Name of the neural network module.
   * @param num_inputs Number of input neurons or parameters.
   * @param num_outputs Number of output neurons.
   * @param num_neurons_per_layer Hidden-layer widths.
   * @param activation_function Hidden-layer activation names.
   * @param device_type Torch device used by the module.
   * @param scalar_type Torch scalar type used by the module.
   * @param build_on_construct Whether to build the torch modules right away.
   * @param input_shift_factors Optional affine input shifts.
   * @param input_scaling_factors Optional affine input scales.
   * @param output_scaling_factors Optional output scaling factors.
   */
  LibtorchArtificialNeuralNet(const std::string name,
                              const unsigned int num_inputs,
                              const unsigned int num_outputs,
                              const std::vector<unsigned int> & num_neurons_per_layer,
                              const std::vector<std::string> & activation_function = {"relu"},
                              const torch::DeviceType device_type = torch::kCPU,
                              const torch::ScalarType scalar_type = torch::kDouble,
                              const bool build_on_construct = true,
                              const std::vector<Real> & input_shift_factors = {},
                              const std::vector<Real> & input_scaling_factors = {},
                              const std::vector<Real> & output_scaling_factors = {});

  /**
   * Copy-construct a feed-forward neural network.
   * @param nn Neural network to copy.
   * @param build_on_construct Whether to rebuild the module structure during the copy.
   */
  LibtorchArtificialNeuralNet(const Moose::LibtorchArtificialNeuralNet & nn,
                              const bool build_on_construct = true);

  /**
   * Add one linear layer to the network.
   * @param layer_name Name of the layer to add.
   * @param parameters Small parameter map that describes the layer shape.
   */
  virtual void addLayer(const std::string & layer_name,
                        const std::unordered_map<std::string, unsigned int> & parameters);

  /**
   * Run a forward pass through the network.
   * @param x Input tensor for the evaluation.
   */
  virtual torch::Tensor forward(const torch::Tensor & x) override;

  /// Return the name of the neural network
  const std::string & name() const { return _name; }
  /// Return the number of neurons on the input layer
  unsigned int numInputs() const { return _num_inputs; }
  /// Return the number of neurons on the output layer
  unsigned int numOutputs() const { return _num_outputs; }
  /// Return the number of hidden layers
  unsigned int numHiddenLayers() const { return _num_neurons_per_layer.size(); }
  /// Return the hidden layer architecture
  const std::vector<unsigned int> & numNeuronsPerLayer() const { return _num_neurons_per_layer; }
  /// Return the multi enum containing the activation functions
  const MultiMooseEnum & activationFunctions() const { return _activation_function; }
  /// Return the device which is used by this neural network
  torch::DeviceType deviceType() const { return _device_type; }
  /// Return the data type which is used by this neural network
  torch::ScalarType dataType() const { return _data_type; }
  /// Return the affine input shift factors used before evaluation
  const std::vector<Real> & inputShiftFactors() const { return _input_shift_factors; }
  /// Return the affine input scaling factors used before evaluation
  const std::vector<Real> & inputScalingFactors() const { return _input_scaling_factors; }
  /// Return the output scaling factors applied after evaluation
  const std::vector<Real> & outputScalingFactors() const { return _output_scaling_factors; }
  /// Construct the neural network
  virtual void constructNeuralNetwork();

  /// Update cached affine metadata vectors from the registered libtorch buffers.
  void synchronizeAffineFactorsFromBuffers();

  /**
   * Map an activation name to the orthogonal-initialization gain we want to use.
   * @param activation Activation name to look up.
   */
  Real determineGain(const std::string & activation);

  /**
   * Initialize the trainable weights and biases.
   * @param generator Optional torch random-number generator used for reproducible initialization.
   */
  virtual void initializeNeuralNetwork(c10::optional<at::Generator> generator = c10::nullopt);

  /// Store the network architecture in a json file (for debugging, visualization)
  void store(nlohmann::json & json) const;

protected:
  /**
   * Set affine metadata by either accepting the user values or filling defaults.
   * @param factors User-provided affine factors.
   * @param expected_size Expected number of entries.
   * @param default_value Default value used when the vector is empty.
   * @param factor_name Name used in error messages.
   */
  static std::vector<Real> setAffineFactors(const std::vector<Real> & factors,
                                            unsigned int expected_size,
                                            Real default_value,
                                            const std::string & factor_name);

  /// Initialize the registered affine metadata buffers used by serialization.
  void initializeAffineBuffers();

  /**
   * Apply affine preprocessing to the raw input tensor.
   * @param x Raw input tensor.
   */
  virtual torch::Tensor preprocessInput(const torch::Tensor & x) const;

  /**
   * Apply the configured output scaling to a network output tensor.
   * @param y Raw network output tensor.
   */
  virtual torch::Tensor scaleOutput(const torch::Tensor & y) const;

  /// Name of the neural network
  const std::string _name;
  /// Submodules that hold linear operations and the corresponding
  /// weights and biases (y = W * x + b)
  std::vector<torch::nn::Linear> _weights;
  // Number of neurons on the input layer
  const unsigned int _num_inputs;
  /// Number of neurons on the output layer
  const unsigned int _num_outputs;
  /// Hidden layer architecture
  const std::vector<unsigned int> _num_neurons_per_layer;
  /// Activation functions (either one for all hidden layers or one for every layer
  /// separately)
  MultiMooseEnum _activation_function;
  /// The device type used for this neural network
  const torch::DeviceType _device_type;
  /// The data type used in this neural network
  const torch::ScalarType _data_type;
  /// Affine preprocessing applied to the flattened input
  std::vector<Real> _input_shift_factors;
  /// Multiplicative affine preprocessing applied after shifting the input
  std::vector<Real> _input_scaling_factors;
  /// Multiplicative scaling applied after the network output is formed
  std::vector<Real> _output_scaling_factors;
  /// Registered libtorch buffer holding the affine input shifts
  torch::Tensor _input_shift_tensor;
  /// Registered libtorch buffer holding the affine input scaling factors
  torch::Tensor _input_scale_tensor;
  /// Registered libtorch buffer holding the output scaling factors
  torch::Tensor _output_scale_tensor;
};

void to_json(nlohmann::json & json, const Moose::LibtorchArtificialNeuralNet * const & network);

void loadLibtorchArtificialNeuralNetState(Moose::LibtorchArtificialNeuralNet & nn,
                                          const std::string & filename);
}

template <>
void dataStore<Moose::LibtorchArtificialNeuralNet>(
    std::ostream & stream,
    std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn,
    void * context);

template <>
void dataLoad<Moose::LibtorchArtificialNeuralNet>(
    std::istream & stream,
    std::shared_ptr<Moose::LibtorchArtificialNeuralNet> & nn,
    void * context);

// This is needed because the reporter which is used to ouput the neural net parameters to JSON
// requires a dataStore/dataLoad. However, these functions will be empty due to the fact that
// we are only interested in the JSON output and we don't want to output everything
template <>
void dataStore<Moose::LibtorchArtificialNeuralNet const>(
    std::ostream & stream, Moose::LibtorchArtificialNeuralNet const *& nn, void * context);

template <>
void dataLoad<Moose::LibtorchArtificialNeuralNet const>(
    std::istream & stream, Moose::LibtorchArtificialNeuralNet const *& nn, void * context);

#endif
