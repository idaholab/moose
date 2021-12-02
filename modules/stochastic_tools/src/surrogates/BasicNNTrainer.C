//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <torch/torch.h>

#include "BasicNNTrainer.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", BasicNNTrainer);

InputParameters
BasicNNTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Computes coefficients for polynomial regession model.");

  MooseEnum data_type("real=0 vector_real=1", "real");
  params.addRequiredParam<ReporterName>(
      "response",
      "Reporter value of response results, can be vpp with <vpp_name>/<vector_name> or sampler "
      "column with 'sampler/col_<index>'.");
  params.addParam<MooseEnum>("response_type", data_type, "Response data type.");
  params.addParam<std::vector<ReporterName>>(
      "predictors",
      std::vector<ReporterName>(),
      "Reporter values used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<std::vector<unsigned int>>(
      "predictor_cols",
      std::vector<unsigned int>(),
      "Sampler columns used as the independent random variables, If 'predictors' and "
      "'predictor_cols' are both empty, all sampler columns are used.");
  params.addParam<unsigned int>("no_batches", 1, "Number of batches.");
  params.addParam<unsigned int>("no_epochs", 1, "Number of epochs.");
  params.addParam<unsigned int>("no_hidden_layers", 0, "Number of hidden layers.");
  params.addParam<std::vector<unsigned int>>(
      "no_neurons_per_layer", std::vector<unsigned int>(), "Number of neurons per layer.");
  params.addParam<Real>("learning_rate", 0.001, "Learning rate (relaxation).");

  return params;
}

BasicNNTrainer::BasicNNTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _sampler_row(getSamplerData()),
    _sample_points(declareModelData<std::vector<std::vector<Real>>>("_sample_points")),
    _pvals(getParam<std::vector<ReporterName>>("predictors").size()),
    _pcols(getParam<std::vector<unsigned int>>("predictor_cols")),
    _response(getTrainingData<Real>(getParam<ReporterName>("response"))),
    _no_batches(getParam<unsigned int>("no_batches")),
    _no_epocs(getParam<unsigned int>("no_epochs")),
    _no_hidden_layers(getParam<unsigned int>("no_hidden_layers")),
    _no_neurons_per_layer(getParam<std::vector<unsigned int>>("no_neurons_per_layer")),
    _learning_rate(getParam<Real>("learning_rate"))
// #ifdef ENABLE_TF
//     ,
//     _t_root(tensorflow::Scope::NewRootScope())
// #endif

{
  const auto & pnames = getParam<std::vector<ReporterName>>("predictors");
  for (unsigned int i = 0; i < pnames.size(); ++i)
    _pvals[i] = &getTrainingData<Real>(pnames[i]);

  // If predictors and predictor_cols are empty, use all sampler columns
  if (_pvals.empty() && _pcols.empty())
  {
    _pcols.resize(_sampler.getNumberOfCols());
    std::iota(_pcols.begin(), _pcols.end(), 0);
  }

  // Resize sample points to number of predictors
  _sample_points.resize(_pvals.size() + _pcols.size() + 1);

  if (_no_hidden_layers != _no_neurons_per_layer.size())
    mooseError("The number of layers are not the same!");
}

void
BasicNNTrainer::preTrain()
{
  // Resize to number of sample points
  for (auto & it : _sample_points)
    it.resize(_sampler.getNumberOfLocalRows());
}

void
BasicNNTrainer::train()
{
  unsigned int d = 0;
  // Get predictors from reporter values
  for (const auto & val : _pvals)
    _sample_points[d++][_local_row] = *val;
  // Get predictors from sampler
  for (const auto & col : _pcols)
    _sample_points[d++][_local_row] = _sampler_row[col];

  _sample_points.back()[_local_row] = _response;
}

void
BasicNNTrainer::postTrain()
{
  for (auto & it : _sample_points)
    _communicator.allgather(it);

#ifdef ENABLE_TF

  torch::Tensor tensor = torch::rand({2, 3});
  std::cout << tensor << std::endl;

  // std::vector<tensorflow::Tensor> param_batches, response_batches;
  //
  // tensorflow::Status s;
  //
  // s = prepareBatches(param_batches, response_batches);
  // TF_CHECK_OK(s);
  //
  // s = createGraph();
  // TF_CHECK_OK(s);
  //
  // s = createOptimizationGraph();
  // TF_CHECK_OK(s);
  //
  // s = initializeParameters();
  // TF_CHECK_OK(s);
  //
  // for (unsigned int epoch_id = 0; epoch_id < _no_epocs; ++epoch_id)
  // {
  //   std::cout << "Epoch " << epoch_id + 1 << "/" << _no_epocs << ": ";
  //   double loss_sum = 0.0;
  //   for (unsigned int batch_id = 0; batch_id < _no_batches; ++batch_id)
  //   {
  //     double loss;
  //     s = trainGraph(param_batches[batch_id], response_batches[batch_id], loss);
  //     loss_sum += loss;
  //   }
  //   std::cout << "Average loss: " << loss_sum / _no_batches << std::endl;
  // }

#endif
}

// #ifdef ENABLE_TF
// tensorflow::Output
// BasicNNTrainer::addLayer(std::string idx,
//                          tensorflow::Scope scope,
//                          int in_neurons,
//                          int out_neurons,
//                          tensorflow::Input input,
//                          bool activate)
// {
//   tensorflow::TensorShape shape = {in_neurons, out_neurons};
//
//   _nn_params["W" + idx] =
//       tensorflow::ops::Variable(scope.WithOpName("W"), shape, tensorflow::DT_DOUBLE);
//   _nn_shapes["W" + idx] = shape;
//   _nn_assigns["W" + idx + "_assign"] =
//       tensorflow::ops::Assign(scope.WithOpName("W_assign"),
//                               _nn_params["W" + idx],
//                               XavierInit(scope, in_neurons, out_neurons));
//
//   shape = {out_neurons};
//
//   _nn_params["B" + idx] =
//       tensorflow::ops::Variable(scope.WithOpName("B"), shape, tensorflow::DT_DOUBLE);
//   _nn_shapes["B" + idx] = shape;
//   _nn_assigns["B" + idx + "_assign"] =
//       tensorflow::ops::Assign(scope.WithOpName("B_assign"),
//                               _nn_params["B" + idx],
//                               tensorflow::Input::Initializer(0.0, shape));
//
//   auto evaluated = tensorflow::ops::Add(
//       scope.WithOpName("Add_B"),
//       tensorflow::ops::MatMul(scope.WithOpName("Mult_W"), input, _nn_params["W" + idx]),
//       _nn_params["B" + idx]);
//   if (activate)
//     return tensorflow::ops::Relu(scope.WithOpName("Relu"), evaluated);
//   else
//     return evaluated;
// }
//
// tensorflow::Input
// BasicNNTrainer::XavierInit(tensorflow::Scope scope, int in_neurons, int out_neurons)
// {
//   double dev(sqrt(6.0 / (in_neurons + out_neurons)));
//
//   tensorflow::Tensor shape_tensor(tensorflow::DT_INT64, {2});
//   auto shape = shape_tensor.vec<tensorflow::int64>();
//   shape(0) = in_neurons;
//   shape(1) = out_neurons;
//
//   auto rand = tensorflow::ops::RandomUniform(scope, shape_tensor, tensorflow::DT_DOUBLE);
//   return tensorflow::ops::Multiply(scope, tensorflow::ops::Sub(scope, rand, 0.5), dev * 2.0);
// }
//
// tensorflow::Status
// BasicNNTrainer::prepareBatches(std::vector<tensorflow::Tensor> & param_batches,
//                                std::vector<tensorflow::Tensor> & response_batches)
// {
//   int sample_per_batch = _sample_points[0].size() / _no_batches;
//   if (sample_per_batch * _no_batches <= _sample_points[0].size())
//     sample_per_batch += 1;
//
//   for (dof_id_type batch_id = 0; batch_id < _no_batches; ++batch_id)
//   {
//     int xdim = static_cast<int>(_sample_points.size()) - 1;
//     int ydim;
//     if ((batch_id+1) * sample_per_batch <= _sample_points[0].size())
//       ydim = sample_per_batch;
//     else
//       ydim = _sample_points[0].size() - batch_id * sample_per_batch;
//
//     std::vector<tensorflow::Input> params, responses;
//     for (dof_id_type sid = 0; sid < ydim; ++sid)
//     {
//       tensorflow::Tensor temp_params(tensorflow::DT_DOUBLE, tensorflow::TensorShape({xdim}));
//       for (dof_id_type pid = 0; pid < xdim; ++pid)
//       {
//         temp_params.vec<double>()(pid) = _sample_points[pid][sid];
//       }
//       params.push_back(tensorflow::Input(temp_params));
//
//       tensorflow::Tensor temp_response(tensorflow::DT_DOUBLE, tensorflow::TensorShape({1}));
//       temp_response.scalar<double>()(0) = _sample_points.back()[sid];
//       responses.push_back(temp_response);
//     }
//
//     tensorflow::InputList batch_inputs(params);
//     tensorflow::InputList batch_responses(responses);
//
//     tensorflow::Scope root = tensorflow::Scope::NewRootScope();
//     auto stacked_inputs = tensorflow::ops::Stack(root, batch_inputs);
//     auto stacked_responses = tensorflow::ops::Stack(root, batch_responses);
//
//     TF_CHECK_OK(root.status());
//
//     tensorflow::ClientSession session(root);
//     std::vector<tensorflow::Tensor> out_tensors;
//
//     TF_CHECK_OK(session.Run({}, {stacked_inputs, stacked_responses}, &out_tensors));
//     param_batches.push_back(out_tensors[0]);
//     response_batches.push_back(out_tensors[1]);
//   }
//   return tensorflow::Status::OK();
// }
//
// tensorflow::Status
// BasicNNTrainer::createGraph()
// {
//   _input_param_batch =
//       tensorflow::ops::Placeholder(_t_root.WithOpName(_input_name), tensorflow::DT_DOUBLE);
//
//   tensorflow::Input next_input = _input_param_batch;
//   int in_neurons = static_cast<int>(_sample_points.size()) - 1;
//
//   std::vector<tensorflow::Input> inputs;
//   inputs.push_back(next_input);
//
//   for (unsigned int layer_id = 0; layer_id < _no_hidden_layers; ++layer_id)
//   {
//     tensorflow::Scope sub_scope = _t_root.NewSubScope("Layer" + std::to_string(layer_id));
//     inputs.push_back(tensorflow::Input(addLayer(std::to_string(layer_id),
//                                                 sub_scope,
//                                                 in_neurons,
//                                                 _no_neurons_per_layer[layer_id],
//                                                 inputs[layer_id],
//                                                 true)));
//
//     in_neurons = _no_neurons_per_layer[layer_id];
//   }
//
//   tensorflow::Scope sub_scope = _t_root.NewSubScope("Output");
//   _output = addLayer("Output", sub_scope, in_neurons, 1, inputs.back(), false);
//   return _t_root.status();
// }
//
// tensorflow::Status
// BasicNNTrainer::createOptimizationGraph()
// {
//   _input_response_batch =
//       tensorflow::ops::Placeholder(_t_root.WithOpName(_input_name + "R"), tensorflow::DT_DOUBLE);
//
//   tensorflow::Scope loss_scope = _t_root.NewSubScope("Loss_Scope");
//
//   _output_loss = tensorflow::ops::Mean(
//       loss_scope.WithOpName("Loss"),
//       tensorflow::ops::SquaredDifference(loss_scope, _output, _input_response_batch),
//       {0});
//
//   TF_CHECK_OK(loss_scope.status());
//
//   for (std::pair<std::string, tensorflow::Output> i : _nn_params)
//     _raw_params.push_back(i.second);
//
//   std::vector<tensorflow::Output> grad_outputs;
//   TF_CHECK_OK(
//       tensorflow::AddSymbolicGradients(_t_root, {_output_loss}, _raw_params, &grad_outputs));
//
//   unsigned int counter = 0;
//   for (std::pair<std::string, tensorflow::Output> i : _nn_params)
//   {
//     std::string sind = std::to_string(counter);
//     auto param_var_1 =
//         tensorflow::ops::Variable(_t_root, _nn_shapes[i.first], tensorflow::DT_DOUBLE);
//     auto param_var_2 =
//         tensorflow::ops::Variable(_t_root, _nn_shapes[i.first], tensorflow::DT_DOUBLE);
//
//     _nn_assigns["param_var_1_" + sind] = tensorflow::ops::Assign(
//         _t_root, param_var_1, tensorflow::Input::Initializer(0.0, _nn_shapes[i.first]));
//     _nn_assigns["param_var_2_" + sind] = tensorflow::ops::Assign(
//         _t_root, param_var_2, tensorflow::Input::Initializer(0.0, _nn_shapes[i.first]));
//
//     auto adam = tensorflow::ops::ApplyAdam(_t_root,
//                                            i.second,
//                                            param_var_1,
//                                            param_var_2,
//                                            0.0,
//                                            0.0,
//                                            _learning_rate,
//                                            0.9,
//                                            0.999,
//                                            0.00000001,
//                                            {grad_outputs[counter]});
//     _v_out_grads.push_back(adam.operation);
//     counter++;
//   }
//
//   return _t_root.status();
// }
//
// tensorflow::Status
// BasicNNTrainer::initializeParameters()
// {
//   if (!_t_root.ok())
//     return _t_root.status();
//
//   std::vector<tensorflow::Output> initialize_ops;
//
//   for (std::pair<std::string, tensorflow::Output> i : _nn_assigns)
//   {
//     initialize_ops.push_back(i.second);
//   }
//
//   _t_session = std::unique_ptr<tensorflow::ClientSession>(new
//   tensorflow::ClientSession(_t_root));
//
//   TF_CHECK_OK(_t_session->Run(initialize_ops, NULL));
//
//   return tensorflow::Status::OK();
// }
//
// tensorflow::Status
// BasicNNTrainer::trainGraph(tensorflow::Tensor & param_batch,
//                            tensorflow::Tensor & response_batch,
//                            double & loss)
// {
//   if (!_t_root.ok())
//     return _t_root.status();
//
//   std::vector<tensorflow::Tensor> out_tensors;
//
//   TF_CHECK_OK(
//       _t_session->Run({{_input_param_batch, param_batch}, {_input_response_batch,
//       response_batch}},
//                       {_output_loss, _output},
//                       _v_out_grads,
//                       &out_tensors));
//
//   loss = out_tensors[0].scalar<double>()(0);
//
//   return tensorflow::Status::OK();
// }
//
// #endif
