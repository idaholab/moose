//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "GenericActiveLearningSampler.h"
#include "ActiveLearningGaussianProcess.h"
#include "GaussianProcess.h"
#include "SurrogateModel.h"
#include "SurrogateModelInterface.h"
#include "GaussianProcessSurrogate.h"
#include "ParallelAcquisitionFunctionBase.h"
#include "ParallelAcquisitionInterface.h"

// forward declarations
template <typename SamplerType>
class GenericActiveLearnerTempl;

typedef GenericActiveLearnerTempl<GenericActiveLearningSampler> GenericActiveLearner;

/**
 * A generic reporter to support parallel active learning: re-trains GP and picks the next best
 * batch
 */
template <typename SamplerType>
class GenericActiveLearnerTempl : public GeneralReporter,
                                  public ParallelAcquisitionInterface,
                                  public SurrogateModelInterface

{
public:
  static InputParameters validParams();
  GenericActiveLearnerTempl(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /**
   * Sets up the training data for the GP model
   * @param data_out The data vector containing the outputs to train the GP
   * @param data_in The data matrix containing the inputs to train the GP
   */
  virtual void setupGPData(const std::vector<Real> & data_out, const DenseMatrix<Real> & data_in);

  /**
   * Computes the outputs of the trained GP model
   * @param eval_outputs The outputs predicted by the GP model
   */
  virtual void computeGPOutput(std::vector<Real> & eval_outputs);

  /**
   * Computes the convergence value during active learning
   */
  virtual Real computeConvergenceValue();

  /**
   * Evaluate the GP on all the test samples sent by the Sampler
   */
  virtual void evaluateGPTest();

  /**
    Setup the generic variable for acquisition computation (depends on the objective:
    optimization, UQ, etc.)
  */
  virtual void setupGeneric();

  /**
   * Include additional inputs before evaluating the acquisition function.
   * Has trivial function in base, but can be modified in derived if necessary depending
   * upon the objective of active learning (i.e., forward UQ, inverse UQ, optimization, etc.)
   */
  virtual void includeAdditionalInputs();

  /**
   * Output the acquisition function values and ordering of the indices
   * @param acq_new The computed values of the acquisition function
   * @param indices The indices ordered according to the acqusition values to be sent to Sampler
   */
  virtual void getAcquisition(std::vector<Real> & acq_new, std::vector<unsigned int> & indices);

  /// The base sampler
  SamplerType & _al_sampler;

  /// The input dimension for GP, equal to Sampler columns
  unsigned int _n_dim;

  /// Storage for the number of parallel proposals
  dof_id_type _props;

  /// Storage for all the proposed samples to test the GP model
  const std::vector<std::vector<Real>> & _inputs_test;

  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Modified value of model output by this reporter class
  std::vector<Real> & _output_comm;

  /// The selected sample indices to evaluate the subApp
  std::vector<unsigned int> & _sorted_indices;

  /// The active learning GP trainer that permits re-training
  const ActiveLearningGaussianProcess & _al_gp;

  /// The GP evaluator object that permits re-evaluations
  const SurrogateModel & _gp_eval;

  /// Storage for the parallel acquisition object to be utilized
  ParallelAcquisitionFunctionBase & _acquisition_obj;

  /// The acquistion function values in the current iteration
  std::vector<Real> & _acquisition_value;

  /// For monitoring convergence of active learning
  Real & _convergence_value;

  /// Storage for all the modified proposed samples to test the GP model
  std::vector<std::vector<Real>> _inputs_test_modified;

  /// Transmit the required inputs to the json file
  std::vector<std::vector<Real>> & _inputs_required;

  /// Penalize acquisition to prevent clustering when operating in parallel
  const bool & _penalize_acquisition;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Storage for the GP re-training inputs
  std::vector<std::vector<Real>> _gp_inputs;

  /// Storage for the GP re-training outputs
  std::vector<Real> _gp_outputs;

  /// Outputs of GP model for the test samples
  std::vector<Real> _gp_outputs_test;

  /// Outputs of GP model standard deviation for the test samples
  std::vector<Real> _gp_std_test;

  /// Storage for the length scales after the GP training
  std::vector<Real> _length_scales;

  /// A generic parameter to be passed to the acquisition function
  std::vector<Real> _generic;

  /// The GP outputs from the current iteration before re-training (to evaluate convergence)
  std::vector<Real> _eval_outputs_current;
};

template <typename SamplerType>
InputParameters
GenericActiveLearnerTempl<SamplerType>::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += ParallelAcquisitionInterface::validParams();
  params.addClassDescription("A generic reporter to support parallel active learning: re-trains GP "
                             "and picks the next best batch.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "outputs_required",
      "outputs_required",
      "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluator for the trained GP.");
  params.addParam<ReporterValueName>(
      "sorted_indices",
      "sorted_indices",
      "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addParam<ReporterValueName>(
      "acquisition_function",
      "acquisition_function",
      "The values of the acquistion function in the current iteration.");
  params.addParam<ReporterValueName>(
      "convergence_value", "convergence_value", "Value to measure convergence of active learning.");
  params.addParam<ReporterValueName>(
      "inputs", "inputs", "Modified value of the model inputs from this reporter class.");
  params.addRequiredParam<UserObjectName>("acquisition", "Name of the acquisition function.");
  params.addParam<bool>(
      "penalize_acquisition",
      true,
      "Set true to prevent clustering of the best batch inputs when operating in parallel.");
  return params;
}

template <typename SamplerType>
GenericActiveLearnerTempl<SamplerType>::GenericActiveLearnerTempl(
    const InputParameters & parameters)
  : GeneralReporter(parameters),
    ParallelAcquisitionInterface(parameters),
    SurrogateModelInterface(this),
    _al_sampler(getSampler<SamplerType>("sampler")),
    _n_dim(_al_sampler.getNumberOfCols()),
    _props(_al_sampler.getNumParallelProposals()),
    _inputs_test(_al_sampler.getSampleTries()),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_comm(declareValue<std::vector<Real>>("outputs_required")),
    _sorted_indices(declareValue<std::vector<unsigned int>>("sorted_indices")),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcessSurrogate>("gp_evaluator")),
    _acquisition_obj(getParallelAcquisitionFunctionByName(getParam<UserObjectName>("acquisition"))),
    _acquisition_value(declareValue<std::vector<Real>>("acquisition_function")),
    _convergence_value(declareValue<Real>("convergence_value")),
    _inputs_required(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _penalize_acquisition(getParam<bool>("penalize_acquisition")),
    _check_step(std::numeric_limits<int>::max())
{
  // Setting up the variable sizes to facilitate active learning.
  _gp_outputs_test.resize(_inputs_test.size());
  _gp_std_test.resize(_inputs_test.size());
  _acquisition_value.resize(_props);
  _length_scales.resize(_n_dim);
  _eval_outputs_current.resize(_props);
  _generic.resize(1);
  _inputs_required.resize(_props, std::vector<Real>(_n_dim, 0.0));
  _sorted_indices.resize(_props, 1u);
}

template <typename SamplerType>
void
GenericActiveLearnerTempl<SamplerType>::setupGPData(const std::vector<Real> & data_out,
                                                    const DenseMatrix<Real> & data_in)
{
  for (unsigned int i = 0; i < data_out.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      _inputs_required[i][j] = data_in(i, j);
    _gp_inputs.push_back(_inputs_required[i]);
    _gp_outputs.push_back(data_out[i]);
  }
}

template <typename SamplerType>
void
GenericActiveLearnerTempl<SamplerType>::computeGPOutput(std::vector<Real> & eval_outputs)
{
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
    eval_outputs[i] = _gp_eval.evaluate(_gp_inputs[i]);
}

template <typename SamplerType>
void
GenericActiveLearnerTempl<SamplerType>::setupGeneric()
{
  _generic = _gp_outputs;
}

template <typename SamplerType>
void
GenericActiveLearnerTempl<SamplerType>::includeAdditionalInputs()
{
  _inputs_test_modified = _inputs_test;
}

template <typename SamplerType>
void
GenericActiveLearnerTempl<SamplerType>::getAcquisition(std::vector<Real> & acq_new,
                                                       std::vector<unsigned int> & indices)
{
  std::vector<Real> acq;
  acq.resize(_inputs_test.size());
  includeAdditionalInputs();
  _acquisition_obj.computeAcquisition(
      acq, _gp_outputs_test, _gp_std_test, _inputs_test_modified, _gp_inputs, _generic);
  acq_new = acq;
  if (_penalize_acquisition)
    _acquisition_obj.penalizeAcquisition(
        acq_new, indices, acq, _length_scales, _inputs_test_modified);
}

template <typename SamplerType>
Real
GenericActiveLearnerTempl<SamplerType>::computeConvergenceValue()
{
  Real convergence_value = 0.0;
  for (unsigned int ii = 0; ii < _output_comm.size(); ++ii)
    convergence_value += Utility::pow<2>(_output_comm[ii] - _eval_outputs_current[ii]);
  convergence_value = std::sqrt(convergence_value) / _output_comm.size();
  return convergence_value;
}

template <typename SamplerType>
void
GenericActiveLearnerTempl<SamplerType>::evaluateGPTest()
{
  for (unsigned int i = 0; i < _gp_outputs_test.size(); ++i)
    _gp_outputs_test[i] = _gp_eval.evaluate(_inputs_test[i], _gp_std_test[i]);
}

template <typename SamplerType>
void
GenericActiveLearnerTempl<SamplerType>::execute()
{
  if (_al_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }

  DenseMatrix<Real> data_in(_al_sampler.getNumberOfRows(), _al_sampler.getNumberOfCols());
  for (dof_id_type ss = _al_sampler.getLocalRowBegin(); ss < _al_sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _al_sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _al_sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  _communicator.sum(data_in.get_values());
  _output_comm = _output_value;
  _communicator.allgather(_output_comm);

  if (_t_step > 1)
  {
    // Setup the GP training data
    setupGPData(_output_comm, data_in);

    // Compute the convergence value before re-training the GP
    if (_t_step > 2)
    {
      computeGPOutput(_eval_outputs_current);
      _convergence_value = computeConvergenceValue();
    }

    // Retrain the GP and get the length scales
    _al_gp.reTrain(_gp_inputs, _gp_outputs);
    _length_scales = _al_gp.getLengthScales();

    // Evaluate the GP on all the test samples sent by the Sampler
    evaluateGPTest();

    // Setup the generic variable for acquisition computation (depends on the objective:
    // optimization, UQ, etc.)
    setupGeneric();

    // Get the acquisition function values and ordering of indices as per the acquisition
    std::vector<Real> acq_new;
    std::vector<unsigned int> indices;
    indices.resize(_inputs_test.size());
    getAcquisition(acq_new, indices);

    // Output the acquisition function values and the best ordering of the indices
    std::copy_n(indices.begin(), _props, _sorted_indices.begin());
    std::copy_n(acq_new.begin(), _props, _acquisition_value.begin());
  }
  else
    std::iota(_sorted_indices.begin(), _sorted_indices.end(), 0);

  // Track the current step
  _check_step = _t_step;
}
