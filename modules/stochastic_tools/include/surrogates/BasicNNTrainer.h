//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"
#include "SurrogateTrainer.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/cc/client/client_session.h"

class BasicNNTrainer : public SurrogateTrainer
{
public:
  static InputParameters validParams();

  BasicNNTrainer(const InputParameters & parameters);

  virtual void preTrain() override;

  virtual void train() override;

  virtual void postTrain() override;

protected:
#ifdef ENABLE_TF
  tensorflow::Output addLayer(std::string idx,
                              tensorflow::Scope scope,
                              int in_neurons,
                              int out_neurons,
                              tensorflow::Input input,
                              bool activate);

  tensorflow::Input XavierInit(tensorflow::Scope scope, int in_neurons, int out_neurons);

  tensorflow::Status prepareBatches(std::vector<tensorflow::Tensor> & param_batches,
                                    std::vector<tensorflow::Tensor> & response_batches);

  tensorflow::Status createGraph();

  tensorflow::Status createOptimizationGraph();

  tensorflow::Status initializeParameters();

  tensorflow::Status trainGraph(tensorflow::Tensor & param_batch,
                                tensorflow::Tensor & response_batch,
                                double & loss);
#endif

private:
  unsigned int _no_batches;
  unsigned int _no_epocs;

  unsigned int _no_hidden_layers;
  std::vector<unsigned int> _no_neurons_per_layer;

  Real _learning_rate;

#ifdef ENABLE_TF

  tensorflow::Scope _t_root;

  std::unique_ptr<tensorflow::ClientSession> _t_session;

  tensorflow::Output _input_param_batch;
  tensorflow::Output _input_response_batch;
  std::string _input_name = "input";

  tensorflow::Output _output;
  tensorflow::Output _output_loss;

  std::map<std::string, tensorflow::Output> _nn_params;
  std::vector<tensorflow::Output> _raw_params;
  std::map<std::string, tensorflow::TensorShape> _nn_shapes;
  std::map<std::string, tensorflow::Output> _nn_assigns;

  std::vector<tensorflow::Output> _v_weights_biases;
  std::vector<tensorflow::Operation> _v_out_grads;

#endif

  /// Data from the current sampler row
  const std::vector<Real> & _sampler_row;

  /// Map containing sample points and the results
  std::vector<std::vector<Real>> & _sample_points;

  /// Predictor values from reporters
  std::vector<const Real *> _pvals;

  /// Columns from sampler for predictors
  std::vector<unsigned int> _pcols;

  /// Response value
  const Real & _response;
};
